简短版，复制到 GitHub 网页编辑：

```markdown
# CppSekai Linux

CppSekai 的 Linux 移植版。原版 [BSOD-MEMZ/CppSekai](https://github.com/BSOD-MEMZ/CppSekai) 是 Windows 触屏用的 Project SEKAI 风格 SUS 谱面播放器。

## 与原版的差异

| 功能 | 原版 | Linux |
|---|---|---|
| 窗口/输入 | Win32 + SDL2 | SDL2 |
| 渲染 | OpenGL 3.3 | 不变 |
| 音频 | miniaudio | miniaudio (ALSA/Pulse) |
| 字体 | Windows 注册表 | fontconfig |
| 音乐商店 | Win32 + WinHTTP | GTK3 + libcurl |
| SMTC / Aero | 有 | 已移除 |
| 本地多人 | 命名共享内存 | 暂未实现 |

## 功能

- SUS 谱面解析，12 轨全音符
- 判定 / 结算对齐原作
- 键盘 12 键 / 鼠标左右键（双押）/ 手柄
- 别名搜索（~1.2 万条）
- GTK3 谱面下载器

## 安装

### Arch

```bash
git clone https://github.com/arcwolf1/cppSekai-linux
cd cppSekai-linux
makepkg -si
```

### 其他发行版

依赖：`sdl2` `gtk3` `libcurl` `fontconfig` OpenGL `cmake` `pkgconf`

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)
sudo cmake --install build
```

## 运行

```bash
cppsekai              # 游戏
cppsekai-chartdl-gtk  # 谱面下载器 GUI
cppsekai-chartdl --list tell   # CLI 搜索
```

资源搜索顺序：`$CPSEKAI_DATA_DIR` → `/usr/share/cppsekai/` → `~/.local/share/cppsekai/` → 可执行文件同目录

## 谱面

下载器输出到 `charts/`。游戏扫描 `<数据目录>/charts/` 和 `../charts/`。

每个谱面需要：
- `<id4>_<难度>.sus`
- `<vocal_asset>.mp3`
- `<id4>.png`（可选）
- `<id4>.json`（可选）

## 许可

AGPL-3.0，继承自上游。
```

直接粘贴到 GitHub 网页的 README.md 编辑器。
