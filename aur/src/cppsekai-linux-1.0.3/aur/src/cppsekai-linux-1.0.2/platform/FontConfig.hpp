#pragma once

#include <string>

namespace platform {
// 用 fontconfig 查一个能渲染简体中文的字体文件路径。
// 返回空字符串表示没找到。
std::string findCjkFont();
} // namespace platform
