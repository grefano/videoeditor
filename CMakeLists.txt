

cmake_minimum_required(VERSION 4.3.0)
project(myeditor CXX)

set(DIR_IMGUI vendor/imgui/docking/)
add_executable(editor)

target_sources(editor PRIVATE src/main.cpp libs/glad.o vendor/stb.cpp
src/application/render.cpp
src/infrastructure/ffmpeg.cpp src/infrastructure/opengl.cpp src/infrastructure/mediapool.cpp src/infrastructure/timelinewalker.cpp
src/domain/timeline.cpp src/domain/shadercomponent.cpp 
${DIR_IMGUI}imgui.o ${DIR_IMGUI}imgui_demo.o ${DIR_IMGUI}imgui_draw.o ${DIR_IMGUI}imgui_impl_glfw.o ${DIR_IMGUI}imgui_impl_opengl3.o ${DIR_IMGUI}imgui_tables.o ${DIR_IMGUI}imgui_widgets.o)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)
find_package(glfw3 REQUIRED)

target_include_directories(editor PRIVATE src src/debug src/domain src/application src/infrastructure ${MSYS2}/include vendor/ ${DIR_IMGUI})

target_link_libraries(editor PRIVATE ${FFMPEG_LIBRARIES} glfw3)