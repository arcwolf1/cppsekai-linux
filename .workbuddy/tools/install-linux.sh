#!/usr/bin/env bash
# CppSekai 通用 Linux 包的安装脚本。
#
#   ./install.sh              # 装到 /usr（需要 root，脚本会自动 sudo）
#   ./install.sh --user       # 装到 ~/.local（不用 root）
#   ./install.sh /opt/cppsekai  # 自定义前缀
#
# 不装也能用：直接 ./cppsekai 就跑，游戏会拿二进制所在目录当资源目录
#（搜索顺序见下面"资源目录"）—— 想随身带走整个目录时这样最省事。
set -e

cd "$(dirname "$0")"

if [ "${1:-}" = "--user" ]; then
    PREFIX="${HOME}/.local"
else
    PREFIX="${1:-/usr}"
fi

# 写到系统目录但不是 root —— 套一层 sudo
run() {
    if [ -w "$PREFIX" ] || [ "$EUID" -ne 0 ] && [ -w "$(dirname "$PREFIX")" ]; then
        "$@"
    else
        sudo "$@"
    fi
}
# 上面那段判断在 PREFIX 还不存在时会兜不住，这里直接按 EUID 决定
if [ "$EUID" -ne 0 ] && [[ "$PREFIX" == /usr* || "$PREFIX" == /opt* ]]; then
    SUDO=sudo
else
    SUDO=
fi

echo "[install] 前缀: $PREFIX"

$SUDO install -d "$PREFIX/bin"
$SUDO install -m755 cppsekai "$PREFIX/bin/cppsekai"
$SUDO install -m755 cppsekai-chartdl "$PREFIX/bin/cppsekai-chartdl"
$SUDO install -m755 cppsekai-chartdl-gtk "$PREFIX/bin/cppsekai-chartdl-gtk"

# 资源目录：游戏按 环境变量 CPSEKAI_DATA_DIR → /usr/share/cppsekai →
# /usr/local/share/cppsekai → ~/.local/share/cppsekai → 二进制同目录 的顺序找
# musics.json，所以这里必须跟着前缀走。
$SUDO install -d "$PREFIX/share/cppsekai"
$SUDO cp -r assets "$PREFIX/share/cppsekai/"
$SUDO cp -f musics.json music-levels.json music-vocals.json music-aliases.json "$PREFIX/share/cppsekai/"

# 图标 + 桌面文件：只在 FHS 布局下装（自定义前缀没有 share/applications 的约定）
if [ -d "$PREFIX/share" ]; then
    $SUDO install -d "$PREFIX/share/icons/hicolor/256x256/apps"
    $SUDO install -m644 icon.png "$PREFIX/share/icons/hicolor/256x256/apps/cppsekai.png"
    $SUDO install -d "$PREFIX/share/applications"
    $SUDO install -m644 cppsekai.desktop "$PREFIX/share/applications/cppsekai.desktop"
    $SUDO install -d "$PREFIX/share/licenses/cppsekai"
    $SUDO install -m644 LICENSE "$PREFIX/share/licenses/cppsekai/LICENSE"
    if command -v update-desktop-database >/dev/null 2>&1; then
        $SUDO update-desktop-database "$PREFIX/share/applications" || true
    fi
fi

echo "[install] 完成："
echo "  主程序       $PREFIX/bin/cppsekai"
echo "  谱面下载器   $PREFIX/bin/cppsekai-chartdl-gtk（GUI） / cppsekai-chartdl（CLI）"
echo "  资源目录     $PREFIX/share/cppsekai"
echo
echo "依赖：sdl2 gtk3 curl fontconfig（Debian/Ubuntu: libsdl2-0 libgtk-3-0 libcurl4 libfontconfig1）"
