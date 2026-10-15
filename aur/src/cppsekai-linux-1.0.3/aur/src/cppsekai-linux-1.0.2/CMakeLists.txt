cmake_minimum_required(VERSION 3.16)
project(cppsekai LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---- 依赖
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(LIBCURL REQUIRED libcurl)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
find_package(OpenGL REQUIRED)
find_package(Fontconfig REQUIRED)

# ---- 编译选项
add_compile_definitions(_XM_NO_INTRINSICS_)
add_compile_options(-Wall -Wextra -Wno-unused-parameter)

# ---- 源文件
set(UPSTREAM_SOURCES
    core/native/src/mmw_preview.cpp
    core/native/mmw_port/Math.cpp
    core/native/mmw_port/MinMax.cpp
    core/native/mmw_port/Note.cpp
    core/native/mmw_port/Score.cpp
    core/native/mmw_port/Tempo.cpp
    core/native/mmw_port/Utilities.cpp
    core/native/mmw_port/EffectView.cpp
    core/native/mmw_port/Particle.cpp
    core/native/mmw_port/ResourceManager.cpp
    core/native/mmw_port/Rendering/Camera.cpp)

set(GAME_SOURCES
    main.cpp
    platform/Audio.cpp
    platform/CoreApi.cpp
    platform/FontOutline.cpp
    platform/FontConfig.cpp
    platform/NativeMenu.cpp
    platform/Renderer.cpp
    platform/ShellIntegration.cpp
    platform/SystemMedia.cpp
    platform/Party_linux.cpp
    game/Hud.cpp game/Intro.cpp game/Judgement.cpp game/PartyScreen.cpp
    game/Result.cpp game/SongSelect.cpp game/StageBackground.cpp
    game/TapEffect.cpp game/Ui.cpp)

set(IMGUI_SOURCES
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/imgui_impl_sdl2.cpp
    third_party/imgui/imgui_impl_opengl3.cpp)

# ---- 上游代码编成静态库（关警告）
add_library(cppsekai_upstream STATIC ${UPSTREAM_SOURCES})
target_include_directories(cppsekai_upstream PUBLIC
    core/native core/native/src core/native/generated
    third_party third_party/nlohmann core/native/vendor
    third_party/DirectXMath/Inc)
target_compile_options(cppsekai_upstream PRIVATE -w)

# ---- imgui
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC third_party/imgui ${SDL2_INCLUDE_DIRS})
target_link_libraries(imgui PUBLIC ${SDL2_LIBRARIES})
target_link_directories(imgui PUBLIC ${SDL2_LIBRARY_DIRS})
target_compile_options(imgui PRIVATE -w)

# ---- 主程序
add_executable(cppsekai ${GAME_SOURCES})
target_include_directories(cppsekai PRIVATE
    .
    core/native core/native/src core/native/generated
    third_party third_party/nlohmann third_party/DirectXMath/Inc)
target_link_libraries(cppsekai
    cppsekai_upstream imgui
    ${SDL2_LIBRARIES}
    OpenGL::GL
    Fontconfig::Fontconfig
    dl pthread)
target_include_directories(cppsekai PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_directories(cppsekai PRIVATE ${SDL2_LIBRARY_DIRS})

# ===== 谱面下载器 CLI =====
add_executable(cppsekai-chartdl
    downloader/chartdl_linux.cpp)
target_include_directories(cppsekai-chartdl PRIVATE third_party)
target_include_directories(cppsekai-chartdl PRIVATE ${LIBCURL_INCLUDE_DIRS})
target_link_libraries(cppsekai-chartdl ${LIBCURL_LIBRARIES})
target_link_directories(cppsekai-chartdl PRIVATE ${LIBCURL_LIBRARY_DIRS})

# ===== 谱面下载器 GTK3 GUI =====
add_executable(cppsekai-chartdl-gtk
    downloader/chartdl_gtk.cpp)
target_include_directories(cppsekai-chartdl-gtk PRIVATE third_party ${LIBCURL_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS})
target_link_libraries(cppsekai-chartdl-gtk ${LIBCURL_LIBRARIES} ${GTK3_LIBRARIES})
target_link_directories(cppsekai-chartdl-gtk PRIVATE ${LIBCURL_LIBRARY_DIRS} ${GTK3_LIBRARY_DIRS})
target_compile_options(cppsekai-chartdl-gtk PRIVATE ${GTK3_CFLAGS_OTHER})

# ===== install 规则 =====
include(GNUInstallDirs)

# 二进制
install(TARGETS cppsekai cppsekai-chartdl cppsekai-chartdl-gtk
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# 数据文件（musics.json 等）
install(FILES
    musics.json
    music-levels.json
    music-vocals.json
    music-aliases.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cppsekai)

# assets 目录（整个树）
install(DIRECTORY assets
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cppsekai
    PATTERN "*.md" EXCLUDE)

# 图标：从源 icon.png 生成多尺寸
install(FILES icon.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
    RENAME cppsekai.png)

# 桌面文件
install(FILES cppsekai.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
