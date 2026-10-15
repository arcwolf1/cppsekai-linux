// CppSekai - Linux stub for multi-instance co-op.
//
// 上游用 Windows 命名共享内存（CreateFileMapping）实现同机多实例合作。
// Linux 上可以走 POSIX shm（shm_open + mmap），但第一版先 stub 掉：
// 单人模式完全可用，多人游玩功能待后续实现。
// 所有 PartyLink 接口返回"无房间"，调用方退化为单人。

#include "Party.hpp"
#include <chrono>

namespace platform {

bool traceEnabled() { return false; }

std::uint64_t PartyLink::nowCounter() {
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());
}
double PartyLink::counterFrequency() { return 1.0e9; }
double PartyLink::counterToSeconds(std::uint64_t c) {
    return static_cast<double>(c) / counterFrequency();
}

bool PartyLink::init() { mActive = false; mSlot = -1; return false; }
void PartyLink::shutdown() {}
bool PartyLink::roomExists() { return false; }
void PartyLink::update() {}
bool PartyLink::isHost() const { return false; }
bool PartyLink::hostAlive() const { return false; }
int  PartyLink::playerCount() const { return 0; }
void PartyLink::publishHostClock(double) {}
bool PartyLink::readHostClock(double&, std::uint64_t&) const { return false; }
void PartyLink::setName(const std::string&) {}
void PartyLink::setSeat(int) {}
void PartyLink::setReady(bool) {}
void PartyLink::setDifficulty(int) {}
void PartyLink::setPlayingScore(double, int, float) {}
void PartyLink::clearPlayingScore() {}
void PartyLink::lockSong(int, const std::string&, const std::string&, int, int) {}
void PartyLink::beginLoading() {}
void PartyLink::armStart(std::uint64_t) {}
void PartyLink::setPhase(int) {}
void PartyLink::setHostPaused(bool) {}
void PartyLink::publishTrackEnd(double) {}
double PartyLink::readTrackEnd() const { return 0.0; }
void PartyLink::releaseSong() {}
PartyState PartyLink::read() const { PartyState s; s.valid = false; return s; }
std::vector<PartyPlayer> PartyLink::players() const { return {}; }
bool PartyLink::allReady() const { return true; }
bool PartyLink::allLoaded() const { return true; }

} // namespace platform
