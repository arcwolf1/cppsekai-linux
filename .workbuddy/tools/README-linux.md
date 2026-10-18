# CppSekai for Linux — 通用包

Project SEKAI 风格的 SUS 谱面播放器（Windows 原版的 Linux 移植）。
**非官方、非营利、仅供本机个人游玩与学习**，与 SEGA / Colorful Palette / 《初音未来：缤纷舞台》官方没有任何关系。

## 两种用法

**1) 解压即跑（不安装）**

```bash
tar xzf cppsekai-1.0.4-linux-x86_64.tar.gz
cd cppsekai-1.0.4-linux-x86_64
./cppsekai
```

游戏把二进制所在目录当资源目录，所以整个目录拷到 U 盘也能跑，存档（userdata.json / profiles/）也写在这层。

**2) 装进系统**

```bash
./install.sh            # 装到 /usr（自动 sudo）
./install.sh --user     # 装到 ~/.local，不用 root
./install.sh /opt/cppsekai
```

装完后资源目录是 `<前缀>/share/cppsekai`，桌面图标和菜单项一并装上。

## 依赖

`sdl2`、`gtk3`、`curl`、`fontconfig`

- Arch / CachyOS：`sudo pacman -S sdl2 gtk3 curl fontconfig`
- Debian / Ubuntu：`sudo apt install libsdl2-0 libgtk-3-0 libcurl4 libfontconfig1`
- Fedora：`sudo dnf install SDL2 gtk3 libcurl fontconfig`

Arch 系用户直接用 AUR 更省事：`yay -S cppsekai`（或 `paru -S cppsekai`）。

## 谱面

`cppsekai-chartdl-gtk` 是谱面下载器（GUI），`cppsekai-chartdl` 是它的命令行版。
谱面默认放在 `charts/`，音频放 `assets/mmusics/`（下载器会自己处理）。

## 许可

AGPL-3.0-only，见 `LICENSE`。曲名、角色、美术、音频与谱面的权利全部归原权利人。
