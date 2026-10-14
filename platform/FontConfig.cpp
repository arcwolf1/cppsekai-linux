// CppSekai - fontconfig 查询系统 CJK 字体路径。
// Linux 上用 fontconfig 而不是硬编码路径：不同发行版装的字体不一样。

#include "FontConfig.hpp"

#ifdef __linux__
#include <fontconfig/fontconfig.h>
#include <cstdio>
#include <cstring>

namespace platform {

std::string findCjkFont() {
    // 优先顺序：Noto Sans CJK SC → MiSans → 文泉驿 → 系统默认 sans-serif
    const char* prefer[] = {
        "Noto Sans CJK SC",
        "Noto Sans SC",
        "MiSans",
        "WenQuanYi Zen Hei",
        "WenQuanYi Micro Hei",
        "sans-serif",
    };

    if (!FcInit()) {
        return "";
    }

    for (const char* name : prefer) {
        FcPattern* pat = FcNameParse(reinterpret_cast<const FcChar8*>(name));
        if (pat == nullptr) continue;

        // 加上语言约束：必须支持简体中文
        FcPatternAddString(pat, FC_LANG, reinterpret_cast<const FcChar8*>("zh-cn"));
        FcConfigSubstitute(nullptr, pat, FcMatchPattern);
        FcDefaultSubstitute(pat);

        FcResult result = FcResultNoMatch;
        FcPattern* match = FcFontMatch(nullptr, pat, &result);
        FcPatternDestroy(pat);
        if (match == nullptr) continue;

        FcChar8* file = nullptr;
        if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch && file != nullptr) {
            std::string path(reinterpret_cast<char*>(file));
            FcPatternDestroy(match);
            std::printf("[font] fontconfig: %s -> %s\n", name, path.c_str());
            return path;
        }
        FcPatternDestroy(match);
    }

    return "";
}

} // namespace platform

#endif
