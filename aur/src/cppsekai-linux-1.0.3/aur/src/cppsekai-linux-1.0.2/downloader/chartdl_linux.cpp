// CppSekai chart downloader - Linux CLI version.
// 上游 chartdl.cpp 的跨平台部分 + libcurl HTTP。GUI 版后续用 GTK3 加。
//
// 用法：
//   cppsekai-chartdl --list [filter]        # 打印歌曲列表
//   cppsekai-chartdl --download <ids>       # ids: 374 或 374,75,127
//       [--diffs all|easy,normal,hard,expert,master]
//       [--vocals all|0,1,2]
//       [--out <dir>]                       # 默认 charts/ 于 exe 同目录
//   cppsekai-chartdl --download-all [--out <dir>]   # 下载全部

#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
using nlohmann::json;

// ===== 数据源 =====
constexpr int kCnIdMin = 10000;
constexpr const char* kCnAssetBase = "https://storage.sekai.best/sekai-cn-assets";
const char* kDiffNames[5] = {"easy", "normal", "hard", "expert", "master"};

// ===== 数据结构 =====
struct VocalVersion {
    int id = 0;
    std::string type, caption, asset, singers;
};

struct Song {
    int id = 0;
    std::string title, kana, lyricist, composer, arranger, jacket;
    double fillerSec = 0.0;
    int levels[5] = {0,0,0,0,0};
    bool levelsKnown = false;
    std::vector<VocalVersion> vocals;
    std::vector<std::string> aliases;
    std::string searchKey;
};

std::vector<Song> gSongs;
std::string gDataError;

// ===== 辅助 =====
std::string id4(int id) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%04d", id);
    return buf;
}

std::string foldCase(const std::string& s) {
    std::string out = s;
    for (auto& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

bool fileExists(const fs::path& p) {
    std::error_code ec;
    return fs::is_regular_file(p, ec);
}

std::string jacketStem(const std::string& name, int id) {
    return name.empty() ? id4(id) + "_01" : name;
}

// ===== URL 生成 =====
std::string chartUrl(const Song& s, const char* diff) {
    if (s.id >= kCnIdMin)
        return std::string(kCnAssetBase) + "/music/music_score/" + id4(s.id) + "_01/" + diff + ".txt";
    return "https://assets.unipjsk.com/startapp/music/music_score/" + id4(s.id) + "_01/" + diff;
}

std::string audioUrl(const VocalVersion& v, int id) {
    const std::string asset = v.asset.empty() ? id4(id) + "_01" : v.asset;
    if (id >= kCnIdMin)
        return std::string(kCnAssetBase) + "/music/long/" + asset + "/" + asset + ".mp3";
    return "https://assets.unipjsk.com/ondemand/music/long/" + asset + "/" + asset + ".mp3";
}

std::string jacketUrl(const Song& s) {
    const std::string stem = jacketStem(s.jacket, s.id);
    if (s.id >= kCnIdMin)
        return std::string(kCnAssetBase) + "/music/jacket/" + stem + "/" + stem + ".png";
    return "https://assets.unipjsk.com/startapp/music/jacket/" + stem + "/" + stem + ".png";
}

// ===== 文件名 =====
fs::path chartPath(const fs::path& dir, int id, const char* diff) {
    return dir / (id4(id) + "_" + diff + ".sus");
}
fs::path audioPath(const fs::path& dir, const VocalVersion& v, int id) {
    return dir / (v.asset.empty() ? id4(id) + ".mp3" : v.asset + ".mp3");
}
fs::path jacketPath(const fs::path& dir, const Song& s) {
    return dir / (id4(s.id) + ".png");
}
fs::path sidecarPath(const fs::path& dir, int id) {
    return dir / (id4(id) + ".json");
}

// ===== libcurl HTTP =====
static size_t writeCb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* cb = static_cast<std::function<bool(const char*, size_t)>*>(userdata);
    return (*cb)(ptr, size * nmemb) ? size * nmemb : 0;
}

bool httpGet(const std::string& url,
    const std::function<bool(const char*, size_t, long long, long long)>& onChunk,
    std::string& error)
{
    CURL* curl = curl_easy_init();
    if (!curl) { error = "curl init failed"; return false; }

    long long total = 0;
    long long received = 0;
    std::function<bool(const char*, size_t)> chunkWrap =
        [&](const char* data, size_t len) -> bool {
            received += static_cast<long long>(len);
            return onChunk(data, len, received, total);
        };

    char errbuf[CURL_ERROR_SIZE] = {0};
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunkWrap);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 600L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CppSekai/1.0 (Linux)");
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    CURLcode rc = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (rc != CURLE_OK) {
        error = errbuf[0] ? errbuf : curl_easy_strerror(rc);
        return false;
    }
    if (httpCode != 200) {
        error = "HTTP " + std::to_string(httpCode);
        return false;
    }
    return true;
}

// ===== 数据加载 =====
bool readJson(const std::string& fileName, json& out) {
    std::ifstream in(fileName);
    if (!in) return false;
    try { in >> out; return true; } catch (...) { return false; }
}

void loadData(const fs::path& dataDir) {
    gSongs.clear();
    gDataError.clear();

    auto path = [&](const char* name) { return (dataDir / name).string(); };

    json musics;
    if (!readJson(path("musics.json"), musics) || !musics.is_array()) {
        gDataError = "musics.json not found in " + dataDir.string();
        return;
    }
    std::set<int> seen;
    for (const auto& row : musics) {
        if (!row.is_object()) continue;
        Song s;
        s.id = row.value("id", 0);
        s.title = row.value("title", std::string{});
        s.kana = row.value("pronunciation", std::string{});
        s.lyricist = row.value("lyricist", std::string{});
        s.composer = row.value("composer", std::string{});
        s.arranger = row.value("arranger", std::string{});
        s.jacket = row.value("assetbundleName", std::string{});
        s.fillerSec = row.value("fillerSec", 0.0);
        if (s.id > 0 && !s.title.empty() && seen.insert(s.id).second) {
            gSongs.push_back(std::move(s));
        }
    }

    json vocals;
    if (readJson(path("music-vocals.json"), vocals) && vocals.is_object()) {
        std::map<int, size_t> byId;
        for (size_t i = 0; i < gSongs.size(); ++i) byId[gSongs[i].id] = i;
        for (auto it = vocals.begin(); it != vocals.end(); ++it) {
            auto found = byId.find(std::atoi(it.key().c_str()));
            if (found == byId.end() || !it.value().is_array()) continue;
            for (const auto& row : it.value()) {
                VocalVersion v;
                v.id = row.value("id", 0);
                v.type = row.value("type", std::string{});
                v.caption = row.value("caption", std::string{});
                v.asset = row.value("asset", std::string{});
                if (row.contains("singers") && row["singers"].is_array()) {
                    for (const auto& sg : row["singers"]) {
                        if (!sg.is_string()) continue;
                        if (!v.singers.empty()) v.singers += "、";
                        v.singers += sg.get<std::string>();
                    }
                }
                if (!v.asset.empty()) gSongs[found->second].vocals.push_back(std::move(v));
            }
        }
    }

    json levels;
    if (readJson(path("music-levels.json"), levels) && levels.is_object()) {
        std::map<int, size_t> byId;
        for (size_t i = 0; i < gSongs.size(); ++i) byId[gSongs[i].id] = i;
        for (auto it = levels.begin(); it != levels.end(); ++it) {
            auto found = byId.find(std::atoi(it.key().c_str()));
            if (found == byId.end() || !it.value().is_array()) continue;
            const auto& arr = it.value();
            bool any = false;
            for (size_t d = 0; d < arr.size() && d < 5; ++d) {
                if (arr[d].is_number()) {
                    gSongs[found->second].levels[d] = arr[d].get<int>();
                    if (arr[d].get<int>() > 0) any = true;
                }
            }
            gSongs[found->second].levelsKnown = any;
        }
    }

    // 社区别名（music-aliases.json）：{"1":["tell","chrome",...], ...}
    json aliases;
    if (readJson(path("music-aliases.json"), aliases) && aliases.is_object()) {
        std::map<int, size_t> byId;
        for (size_t i = 0; i < gSongs.size(); ++i) byId[gSongs[i].id] = i;
        for (auto it = aliases.begin(); it != aliases.end(); ++it) {
            auto found = byId.find(std::atoi(it.key().c_str()));
            if (found == byId.end() || !it.value().is_array()) continue;
            for (const auto& row : it.value()) {
                if (!row.is_string()) continue;
                std::string a = row.get<std::string>();
                if (!a.empty()) gSongs[found->second].aliases.push_back(a);
            }
        }
    }

    // 预计算 searchKey：title + kana + aliases 合并小写
    for (auto& s : gSongs) {
        std::string key = s.title + " " + s.kana;
        for (const auto& a : s.aliases) key += " " + a;
        s.searchKey = foldCase(key);
    }
}

// ===== 下载任务 =====
enum class JobState { Waiting, Active, Done, Failed, Skipped };

struct Job {
    std::string url;
    fs::path path;
    JobState state = JobState::Waiting;
    long long bytes = 0;
    long long total = 0;
    std::string note;
};

struct Request {
    bool diffs[5] = {true, true, true, true, true};
    std::vector<size_t> vocalIndexes;
    bool jacket = true;
    bool sidecar = true;
};

std::mutex gJobMutex;
std::vector<Job> gJobs;
std::atomic<bool> gCancel{false};
std::atomic<int> gActiveWorkers{0};
std::mutex gLogMutex;

void logLine(const std::string& text) {
    std::lock_guard<std::mutex> lock(gLogMutex);
    std::printf("%s\n", text.c_str());
    std::fflush(stdout);
}

void writeSidecar(const fs::path& dir, const Song& song, const std::vector<VocalVersion>& vocals) {
    json doc;
    doc["title"] = song.title;
    doc["artist"] = song.composer.empty() ? song.lyricist : song.composer;
    if (!song.lyricist.empty()) doc["lyricist"] = song.lyricist;
    if (!song.composer.empty()) doc["composer"] = song.composer;
    if (!song.arranger.empty()) doc["arranger"] = song.arranger;
    if (!vocals.empty() && !vocals.front().singers.empty()) doc["vocal"] = vocals.front().singers;
    if (song.fillerSec > 0.0) doc["fillerSec"] = song.fillerSec;
    std::ofstream out(sidecarPath(dir, song.id));
    if (out) out << doc.dump(2) << std::endl;
}

void queueSong(const Song& song, const Request& req, const fs::path& dir, bool force) {
    std::error_code ec;
    fs::create_directories(dir, ec);
    auto add = [&](const std::string& url, const fs::path& path) {
        std::lock_guard<std::mutex> lock(gJobMutex);
        for (const Job& j : gJobs) if (j.path == path) return;
        Job j;
        j.url = url;
        j.path = path;
        if (!force && fileExists(path)) {
            j.state = JobState::Skipped;
            j.note = "already there";
        }
        gJobs.push_back(std::move(j));
    };

    for (int d = 0; d < 5; ++d) {
        if (req.diffs[d]) add(chartUrl(song, kDiffNames[d]), chartPath(dir, song.id, kDiffNames[d]));
    }
    for (size_t idx : req.vocalIndexes) {
        if (idx < song.vocals.size()) add(audioUrl(song.vocals[idx], song.id), audioPath(dir, song.vocals[idx], song.id));
    }
    if (req.jacket) add(jacketUrl(song), jacketPath(dir, song));
    if (req.sidecar) {
        std::vector<VocalVersion> picked;
        for (size_t idx : req.vocalIndexes) if (idx < song.vocals.size()) picked.push_back(song.vocals[idx]);
        writeSidecar(dir, song, picked);
    }
}

void worker() {
    while (true) {
        size_t index = SIZE_MAX;
        {
            std::lock_guard<std::mutex> lock(gJobMutex);
            for (size_t i = 0; i < gJobs.size(); ++i) {
                if (gJobs[i].state == JobState::Waiting) {
                    index = i;
                    gJobs[i].state = JobState::Active;
                    break;
                }
            }
        }
        if (index == SIZE_MAX) break;
        if (gCancel.load()) {
            std::lock_guard<std::mutex> lock(gJobMutex);
            gJobs[index].state = JobState::Failed;
            gJobs[index].note = "cancelled";
            continue;
        }

        std::string url;
        fs::path path;
        {
            std::lock_guard<std::mutex> lock(gJobMutex);
            url = gJobs[index].url;
            path = gJobs[index].path;
        }
        std::error_code ec;
        fs::create_directories(path.parent_path(), ec);
        const fs::path partPath = path.string() + ".part";
        std::ofstream out(partPath, std::ios::binary);
        if (!out) {
            std::lock_guard<std::mutex> lock(gJobMutex);
            gJobs[index].state = JobState::Failed;
            gJobs[index].note = "cannot write " + partPath.string();
            continue;
        }

        std::string error;
        bool ok = httpGet(url,
            [&](const char* data, size_t len, long long received, long long total) {
                out.write(data, static_cast<std::streamsize>(len));
                std::lock_guard<std::mutex> lock(gJobMutex);
                gJobs[index].bytes = received;
                gJobs[index].total = total;
                return !gCancel.load();
            },
            error);
        out.close();

        {
            std::lock_guard<std::mutex> lock(gJobMutex);
            if (ok && !gCancel.load()) {
                gJobs[index].state = JobState::Done;
            } else {
                gJobs[index].state = JobState::Failed;
                gJobs[index].note = error.empty() ? "download failed" : error;
            }
        }
        if (ok && !gCancel.load()) {
            std::error_code ec2;
            fs::remove(path, ec2);
            fs::rename(partPath, path, ec2);
            logLine("[ok]   " + path.filename().string());
        } else {
            std::error_code ec2;
            fs::remove(partPath, ec2);
            logLine("[fail] " + path.filename().string() + "  " + error);
        }
    }
    if (gActiveWorkers.fetch_sub(1) <= 1) {
        logLine("[done] queue finished");
    }
}

void startWorker() {
    const int n = 4;
    gActiveWorkers.store(n);
    for (int i = 0; i < n; ++i) {
        std::thread(worker).detach();
    }
}

Song* findSong(int id) {
    for (auto& s : gSongs) if (s.id == id) return &s;
    return nullptr;
}

// ===== 命令行 =====
static void printUsage() {
    std::printf(
        "CppSekai chart downloader (Linux)\n"
        "\n"
        "  cppsekai-chartdl --list [filter]        print song table\n"
        "  cppsekai-chartdl --download <ids>       ids: 374 or 374,75,127\n"
        "                      [--diffs all|easy,normal,hard,expert,master]\n"
        "                      [--vocals all|0,1,2]    indexes into vocal list\n"
        "                      [--out <dir>]           default: ./charts\n"
        "  cppsekai-chartdl --download-all         download everything\n"
        "                      [--out <dir>]\n"
        "  cppsekai-chartdl --data <dir>           where musics.json lives\n"
        "                                          default: exe dir, /usr/share/cppsekai\n");
}

#ifndef CHARTDL_NO_MAIN
int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);

    bool doList = false;
    bool doDownload = false;
    bool doDownloadAll = false;
    std::string listFilter;
    std::string idsArg;
    std::string diffsArg = "all";
    std::string vocalsArg = "0";
    std::string outArg = "charts";
    std::string dataArg;

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "--list") {
            doList = true;
            if (i + 1 < args.size() && args[i + 1][0] != '-') listFilter = args[++i];
        } else if (a == "--download") {
            doDownload = true;
            if (i + 1 < args.size()) idsArg = args[++i];
        } else if (a == "--download-all") {
            doDownloadAll = true;
        } else if (a == "--diffs") {
            if (i + 1 < args.size()) diffsArg = args[++i];
        } else if (a == "--vocals") {
            if (i + 1 < args.size()) vocalsArg = args[++i];
        } else if (a == "--out") {
            if (i + 1 < args.size()) outArg = args[++i];
        } else if (a == "--data") {
            if (i + 1 < args.size()) dataArg = args[++i];
        } else if (a == "-h" || a == "--help") {
            printUsage();
            return 0;
        } else {
            std::fprintf(stderr, "Unknown arg: %s\n", a.c_str());
            printUsage();
            return 1;
        }
    }

    if (!doList && !doDownload && !doDownloadAll) {
        printUsage();
        return 1;
    }

    // 数据目录
    fs::path dataDir;
    if (!dataArg.empty()) {
        dataDir = dataArg;
    } else {
        std::error_code ec;
        fs::path exeDir = fs::canonical("/proc/self/exe", ec).parent_path();
        fs::path candidates[] = {
            exeDir,
            "/usr/share/cppsekai",
            exeDir / ".." / "share" / "cppsekai",
        };
        for (auto& p : candidates) {
            if (fileExists(p / "musics.json")) { dataDir = p; break; }
        }
        if (dataDir.empty()) dataDir = exeDir;
    }

    std::printf("[data] %s\n", dataDir.string().c_str());
    loadData(dataDir);
    if (!gDataError.empty()) {
        std::fprintf(stderr, "[error] %s\n", gDataError.c_str());
        return 1;
    }
    std::printf("[data] %zu songs loaded\n", gSongs.size());

    if (doList) {
        for (const auto& s : gSongs) {
            if (!listFilter.empty() && s.title.find(listFilter) == std::string::npos
                && foldCase(s.title).find(foldCase(listFilter)) == std::string::npos) {
                continue;
            }
            std::printf("%4d  %s\n", s.id, s.title.c_str());
        }
        return 0;
    }

    Request req;
    if (diffsArg == "all") {
        for (int d = 0; d < 5; ++d) req.diffs[d] = true;
    } else {
        for (int d = 0; d < 5; ++d) req.diffs[d] = false;
        std::stringstream ss(diffsArg);
        std::string tok;
        while (std::getline(ss, tok, ',')) {
            for (int d = 0; d < 5; ++d) if (tok == kDiffNames[d]) req.diffs[d] = true;
        }
    }

    std::vector<int> targetIds;
    if (doDownloadAll) {
        for (const auto& s : gSongs) targetIds.push_back(s.id);
    } else {
        std::stringstream ss(idsArg);
        std::string tok;
        while (std::getline(ss, tok, ',')) {
            int id = std::atoi(tok.c_str());
            if (id > 0) targetIds.push_back(id);
        }
    }
    if (targetIds.empty()) {
        std::fprintf(stderr, "[error] no songs selected\n");
        return 1;
    }

    fs::path outDir = outArg;
    fs::create_directories(outDir);

    gJobs.clear();
    for (int id : targetIds) {
        Song* s = findSong(id);
        if (!s) {
            std::fprintf(stderr, "[warn] id %d not found\n", id);
            continue;
        }
        Request r = req;
        if (vocalsArg == "all") {
            for (size_t i = 0; i < s->vocals.size(); ++i) r.vocalIndexes.push_back(i);
        } else {
            std::stringstream vs(vocalsArg);
            std::string tok;
            while (std::getline(vs, tok, ',')) {
                size_t idx = static_cast<size_t>(std::atoi(tok.c_str()));
                if (idx < s->vocals.size()) r.vocalIndexes.push_back(idx);
            }
        }
        queueSong(*s, r, outDir, false);
    }

    {
        std::lock_guard<std::mutex> lock(gJobMutex);
        int total = static_cast<int>(gJobs.size());
        int skipped = 0;
        for (const auto& j : gJobs) if (j.state == JobState::Skipped) skipped++;
        std::printf("[queue] %d job(s), %d already present\n", total, skipped);
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    startWorker();

    while (gActiveWorkers.load() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    curl_global_cleanup();
    return 0;
}
#endif  // CHARTDL_NO_MAIN
