# CppSekai Linux 移植版

CppSekai 的 Linux 移植版本。

原版是 [BSOD-MEMZ/CppSekai](https://github.com/BSOD-MEMZ/CppSekai)——一个为 Windows 触屏设备（如教室希沃白板）设计的 Project SEKAI 风格 SUS 谱面播放器。

本仓库在上游基础上完成 Linux 平台适配，代码保留 AGPL-3.0 许可。


---

## 与原版的差异

| 功能 | 原版 Windows | Linux 移植 |
|---|---|---|
| 窗口/输入 | Win32 + SDL2 | SDL2 |
| 渲染 | OpenGL 3.3 core | OpenGL 3.3 core（不变）|
| 音频 | miniaudio | miniaudio（ALSA/PulseAudio）|
| 字体 | 注册表查 Windows 字体 | fontconfig 查系统 CJK 字体 |
| 音乐商店 | Win32 GUI + WinHTTP | GTK3 GUI + libcurl |
| 媒体集成 | SMTC（Windows 媒体浮层）| 已移除 |
| Aero 特效 | Win7 Aero | 已移除 |
| 本地多人 | 命名共享内存 | 暂未实现 |
| 文件关联 | 注册表 | 

---

## 安装

### Arch Linux（AUR）

AUR 目前暂停新包提交，可先手动构建：


---

## 操作

### 游戏

| 输入 | 操作 |
|---|---|
| 鼠标左键 / 键盘 D F J K | 左侧 4 轨 |
| 鼠标右键 / 键盘其它 | 右侧 4 轨 |
| 中间 4 轨 | 键盘 S L 等（可自定义）|
| 鼠标左右同按 | 双押 |
| ESC | 暂停 |
| 手柄 A / B / X / Y | 对应轨道 |
| 手柄 Start | 暂停 |

完整 12 键映射在「设置 → 演奏 → 按键映射」里。

### 谱面下载器

CLI：

