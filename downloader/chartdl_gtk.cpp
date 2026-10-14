// CppSekai chart downloader - GTK3 GUI (Linux)。
#define CHARTDL_NO_MAIN
#include "chartdl_linux.cpp"
#include <gtk/gtk.h>

struct GtkUi {
    GtkWidget* window;
    GtkWidget* tree;
    GtkListStore* store;
    GtkWidget* searchEntry;
    GtkWidget* outDirEntry;
    GtkWidget* progress;
    GtkWidget* statusLabel;
    GtkWidget* startBtn;
    GtkWidget* cancelBtn;
    GtkCheckButton* diffChecks[5];
    GtkCheckButton* jacketCheck;
    GtkCheckButton* sidecarCheck;
    GtkComboBoxText* vocalCombo;
};

static GtkUi gUi;

enum {
    COL_CHECK = 0, COL_ID, COL_TITLE, COL_KANA,
    COL_EASY, COL_NORMAL, COL_HARD, COL_EXPERT, COL_MASTER,
    COL_VERSIONS, COL_STATE, COL_COUNT
};

static void refreshList();

static void buildTree() {
    gUi.store = gtk_list_store_new(COL_COUNT,
        G_TYPE_BOOLEAN, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_INT, G_TYPE_STRING);
    gUi.tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(gUi.store));

    GtkCellRenderer* toggle = gtk_cell_renderer_toggle_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(gUi.tree),
        gtk_tree_view_column_new_with_attributes("", toggle, "active", COL_CHECK, NULL));

    const char* titles[] = {"ID","标题","读音","EASY","NORMAL","HARD","EXPERT","MASTER","版本","状态"};
    int cols[] = {COL_ID,COL_TITLE,COL_KANA,COL_EASY,COL_NORMAL,COL_HARD,COL_EXPERT,COL_MASTER,COL_VERSIONS,COL_STATE};
    for (int i = 0; i < 10; ++i) {
        GtkCellRenderer* r = gtk_cell_renderer_text_new();
        GtkTreeViewColumn* c = gtk_tree_view_column_new_with_attributes(titles[i], r, "text", cols[i], NULL);
        gtk_tree_view_column_set_resizable(c, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(gUi.tree), c);
    }
}

static void onRowActivated(GtkTreeView*, GtkTreePath* path, GtkTreeViewColumn*, gpointer) {
    GtkTreeIter it;
    if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(gUi.store), &it, path)) return;
    gboolean active;
    gtk_tree_model_get(GTK_TREE_MODEL(gUi.store), &it, COL_CHECK, &active, -1);
    gtk_list_store_set(gUi.store, &it, COL_CHECK, !active, -1);
}

static void onSearchChanged(GtkEditable*, gpointer) {
    refreshList();
}

static void refreshList() {
    const char* filter = gtk_entry_get_text(GTK_ENTRY(gUi.searchEntry));
    std::string q = filter ? foldCase(filter) : "";
    gtk_list_store_clear(gUi.store);
    for (size_t i = 0; i < gSongs.size(); ++i) {
        const Song& s = gSongs[i];
        if (!q.empty()) {
            // 支持多关键词（空格分隔 AND）
            bool hit = true;
            size_t pos = 0;
            while (pos < q.size()) {
                while (pos < q.size() && q[pos] == ' ') pos++;
                if (pos >= q.size()) break;
                size_t end = q.find(' ', pos);
                std::string term = q.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
                if (s.searchKey.find(term) == std::string::npos) { hit = false; break; }
                if (end == std::string::npos) break;
                pos = end + 1;
            }
            if (!hit) continue;
        }
        GtkTreeIter it;
        gtk_list_store_append(gUi.store, &it);
        char lv[5][8];
        for (int d = 0; d < 5; ++d) {
            if (s.levels[d] > 0) std::snprintf(lv[d], sizeof(lv[d]), "%d", s.levels[d]);
            else std::strcpy(lv[d], "-");
        }
        gtk_list_store_set(gUi.store, &it,
            COL_CHECK, FALSE, COL_ID, s.id,
            COL_TITLE, s.title.c_str(), COL_KANA, s.kana.c_str(),
            COL_EASY, lv[0], COL_NORMAL, lv[1], COL_HARD, lv[2], COL_EXPERT, lv[3], COL_MASTER, lv[4],
            COL_VERSIONS, (int)s.vocals.size(), COL_STATE, "", -1);
    }
}

// 进度轮询回调
static gboolean pollProgress(gpointer) {
    int total = (int)gJobs.size();
    int done = 0;
    long long bytes = 0, totalBytes = 0;
    {
        std::lock_guard<std::mutex> lock(gJobMutex);
        for (const auto& j : gJobs) {
            if (j.state != JobState::Waiting && j.state != JobState::Active) done++;
            bytes += j.bytes;
            totalBytes += j.total;
        }
    }
    if (total > 0) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(gUi.progress), (double)done / total);
        char buf[128];
        if (totalBytes > 0)
            std::snprintf(buf, sizeof(buf), "%d / %d (%.1f / %.1f MB)", done, total, bytes / 1048576.0, totalBytes / 1048576.0);
        else
            std::snprintf(buf, sizeof(buf), "%d / %d", done, total);
        gtk_label_set_text(GTK_LABEL(gUi.statusLabel), buf);
    }
    if (gActiveWorkers.load() == 0 && done == total && total > 0) {
        gtk_widget_set_sensitive(gUi.startBtn, TRUE);
        gtk_widget_set_sensitive(gUi.cancelBtn, FALSE);
        return FALSE;
    }
    return TRUE;
}

static void onStartClicked(GtkButton*, gpointer) {
    Request req;
    for (int d = 0; d < 5; ++d)
        req.diffs[d] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gUi.diffChecks[d]));
    req.jacket = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gUi.jacketCheck));
    req.sidecar = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gUi.sidecarCheck));
    int vocalMode = gtk_combo_box_get_active(GTK_COMBO_BOX(gUi.vocalCombo));

    std::vector<int> selectedIds;
    GtkTreeModel* model = GTK_TREE_MODEL(gUi.store);
    GtkTreeIter it;
    gboolean valid = gtk_tree_model_get_iter_first(model, &it);
    while (valid) {
        gboolean check; int id;
        gtk_tree_model_get(model, &it, COL_CHECK, &check, COL_ID, &id, -1);
        if (check) selectedIds.push_back(id);
        valid = gtk_tree_model_iter_next(model, &it);
    }
    if (selectedIds.empty()) {
        gtk_label_set_text(GTK_LABEL(gUi.statusLabel), "没选中歌曲");
        return;
    }

    const char* outDir = gtk_entry_get_text(GTK_ENTRY(gUi.outDirEntry));
    fs::path out = (outDir && *outDir) ? outDir : "charts";
    fs::create_directories(out);

    gJobs.clear();
    gCancel.store(false);
    for (int id : selectedIds) {
        Song* s = findSong(id);
        if (!s) continue;
        Request r = req;
        if (vocalMode == 2) {
            for (size_t i = 0; i < s->vocals.size(); ++i) r.vocalIndexes.push_back(i);
        } else if (vocalMode == 1 && !s->vocals.empty()) {
            r.vocalIndexes.push_back(0);
        }
        queueSong(*s, r, out, false);
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    startWorker();
    gtk_widget_set_sensitive(gUi.startBtn, FALSE);
    gtk_widget_set_sensitive(gUi.cancelBtn, TRUE);
    g_timeout_add(200, pollProgress, NULL);
}

static void onCancelClicked(GtkButton*, gpointer) {
    gCancel.store(true);
}

static void onSelectAll(GtkButton*, gpointer) {
    GtkTreeIter it;
    if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(gUi.store), &it)) return;
    do { gtk_list_store_set(gUi.store, &it, COL_CHECK, TRUE, -1); }
    while (gtk_tree_model_iter_next(GTK_TREE_MODEL(gUi.store), &it));
}

static void onSelectNone(GtkButton*, gpointer) {
    GtkTreeIter it;
    if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(gUi.store), &it)) return;
    do { gtk_list_store_set(gUi.store, &it, COL_CHECK, FALSE, -1); }
    while (gtk_tree_model_iter_next(GTK_TREE_MODEL(gUi.store), &it));
}

int main(int argc, char** argv) {
    gtk_init(&argc, &argv);

    // 数据目录
    fs::path dataDir;
    if (argc > 1) {
        dataDir = argv[1];
    } else {
        std::error_code ec;
        fs::path exeDir = fs::canonical("/proc/self/exe", ec).parent_path();
        fs::path candidates[] = {exeDir, "/usr/share/cppsekai", exeDir / ".." / "share" / "cppsekai"};
        for (auto& p : candidates) {
            if (fileExists(p / "musics.json")) { dataDir = p; break; }
        }
        if (dataDir.empty()) dataDir = exeDir;
    }
    loadData(dataDir);
    if (!gDataError.empty()) {
        GtkWidget* dlg = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK, "%s", gDataError.c_str());
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        return 1;
    }

    // 窗口
    gUi.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gUi.window), "CppSekai 谱面下载器");
    gtk_window_set_default_size(GTK_WINDOW(gUi.window), 1100, 700);
    g_signal_connect(gUi.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);
    gtk_container_add(GTK_CONTAINER(gUi.window), vbox);

    // 顶部工具栏
    GtkWidget* top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_pack_start(GTK_BOX(vbox), top, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top), gtk_label_new("输出:"), FALSE, FALSE, 0);
    gUi.outDirEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(gUi.outDirEntry), "charts");
    gtk_box_pack_start(GTK_BOX(top), gUi.outDirEntry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(top), gtk_label_new("搜索:"), FALSE, FALSE, 0);
    gUi.searchEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(top), gUi.searchEntry, TRUE, TRUE, 0);
    g_signal_connect(gUi.searchEntry, "changed", G_CALLBACK(onSearchChanged), NULL);

    GtkWidget* btnAll = gtk_button_new_with_label("全选");
    GtkWidget* btnNone = gtk_button_new_with_label("清空");
    gtk_box_pack_start(GTK_BOX(top), btnAll, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top), btnNone, FALSE, FALSE, 0);
    g_signal_connect(btnAll, "clicked", G_CALLBACK(onSelectAll), NULL);
    g_signal_connect(btnNone, "clicked", G_CALLBACK(onSelectNone), NULL);

    // 歌曲列表
    buildTree();
    g_signal_connect(gUi.tree, "row-activated", G_CALLBACK(onRowActivated), NULL);
    GtkWidget* scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), gUi.tree);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    // 下载选项
    GtkWidget* opts = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_pack_start(GTK_BOX(vbox), opts, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(opts), gtk_label_new("难度:"), FALSE, FALSE, 0);
    const char* dnames[] = {"EASY", "NORMAL", "HARD", "EXPERT", "MASTER"};
    for (int d = 0; d < 5; ++d) {
        gUi.diffChecks[d] = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(dnames[d]));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gUi.diffChecks[d]), TRUE);
        gtk_box_pack_start(GTK_BOX(opts), GTK_WIDGET(gUi.diffChecks[d]), FALSE, FALSE, 0);
    }
    gUi.jacketCheck = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("封面"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gUi.jacketCheck), TRUE);
    gtk_box_pack_start(GTK_BOX(opts), GTK_WIDGET(gUi.jacketCheck), FALSE, FALSE, 0);
    gUi.sidecarCheck = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("元数据"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gUi.sidecarCheck), TRUE);
    gtk_box_pack_start(GTK_BOX(opts), GTK_WIDGET(gUi.sidecarCheck), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(opts), gtk_label_new("  人声:"), FALSE, FALSE, 0);
    gUi.vocalCombo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(gUi.vocalCombo, "不下");
    gtk_combo_box_text_append_text(gUi.vocalCombo, "首个");
    gtk_combo_box_text_append_text(gUi.vocalCombo, "全部");
    gtk_combo_box_set_active(GTK_COMBO_BOX(gUi.vocalCombo), 1);
    gtk_box_pack_start(GTK_BOX(opts), GTK_WIDGET(gUi.vocalCombo), FALSE, FALSE, 0);

    // 底部
    GtkWidget* bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_pack_start(GTK_BOX(vbox), bottom, FALSE, FALSE, 0);
    gUi.progress = gtk_progress_bar_new();
    gtk_widget_set_hexpand(gUi.progress, TRUE);
    gtk_box_pack_start(GTK_BOX(bottom), gUi.progress, TRUE, TRUE, 0);
    gUi.statusLabel = gtk_label_new("就绪");
    gtk_box_pack_start(GTK_BOX(bottom), gUi.statusLabel, FALSE, FALSE, 0);
    gUi.startBtn = gtk_button_new_with_label("开始下载");
    gtk_box_pack_start(GTK_BOX(bottom), gUi.startBtn, FALSE, FALSE, 0);
    g_signal_connect(gUi.startBtn, "clicked", G_CALLBACK(onStartClicked), NULL);
    gUi.cancelBtn = gtk_button_new_with_label("取消");
    gtk_widget_set_sensitive(gUi.cancelBtn, FALSE);
    gtk_box_pack_start(GTK_BOX(bottom), gUi.cancelBtn, FALSE, FALSE, 0);
    g_signal_connect(gUi.cancelBtn, "clicked", G_CALLBACK(onCancelClicked), NULL);

    refreshList();
    gtk_widget_show_all(gUi.window);
    gtk_main();
    return 0;
}
