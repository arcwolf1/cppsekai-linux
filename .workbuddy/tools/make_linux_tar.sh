#!/usr/bin/env bash
# 打「通用 Linux 包」：解压即可跑，也可以 ./install.sh 装进系统。
#
# 布局是**扁平**的（二进制和 assets/ 在同一层），因为游戏找资源时最后会退回
# SDL_GetBasePath()，也就是二进制所在目录 —— 放到 bin/ 里就找不到了。
# install.sh 装进系统时再把它们拆成 FHS（bin/ + share/cppsekai/）。
#
# 用法：
#   bash .workbuddy/tools/make_linux_tar.sh <staging 目录> <版本> [输出目录]
set -euo pipefail

STAGE=${1:?用法: make_linux_tar.sh <staging 目录> <版本> [输出目录]}
VER=${2:?缺版本}
OUT=${3:-.}
NAME="cppsekai-$VER-linux-x86_64"

[ -d "$STAGE/usr" ] || { echo "[tar] $STAGE/usr 不存在 —— 先 cmake --install 到这个目录"; exit 1; }

# 别用 mktemp -d：这台机器的 /tmp 是 10M 的 tmpfs，assets（~17M）拷不进去。
ROOT="$OUT/.staging-$NAME"
rm -rf "$ROOT"
DIR="$ROOT/$NAME"
mkdir -p "$DIR"

# 二进制放顶层
install -m755 "$STAGE"/usr/bin/cppsekai "$DIR"/cppsekai
install -m755 "$STAGE"/usr/bin/cppsekai-chartdl "$DIR"/cppsekai-chartdl
install -m755 "$STAGE"/usr/bin/cppsekai-chartdl-gtk "$DIR"/cppsekai-chartdl-gtk

# 数据（assets + 四张曲目表）也放顶层
cp -r "$STAGE"/usr/share/cppsekai/assets "$DIR"/assets
cp "$STAGE"/usr/share/cppsekai/*.json "$DIR"/

# 装系统时才用得到的东西
cp "$STAGE"/usr/share/applications/cppsekai.desktop "$DIR"/cppsekai.desktop
cp icon.png "$DIR"/icon.png
cp LICENSE "$DIR"/LICENSE
cp .workbuddy/tools/install-linux.sh "$DIR"/install.sh
chmod +x "$DIR"/install.sh
cp .workbuddy/tools/README-linux.md "$DIR"/README.md

mkdir -p "$OUT"
TARGET="$OUT/$NAME.tar.gz"
rm -f "$TARGET"
tar -czf "$TARGET" -C "$ROOT" "$NAME"
rm -rf "$ROOT"

echo "[tar] $TARGET"
ls -lh "$TARGET"
