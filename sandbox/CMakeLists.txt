

cmake_minimum_required(VERSION 4.3.0)
project(sandbox CXX)

set(DIR_IMGUI ../vendor/imgui/docking/)
add_executable(sandbox)

target_sources(sandbox PRIVATE renderfile.cpp ../libs/glad.o ../vendor/stb.cpp
${DIR_IMGUI}imgui.o ${DIR_IMGUI}imgui_demo.o ${DIR_IMGUI}imgui_draw.o ${DIR_IMGUI}imgui_impl_glfw.o ${DIR_IMGUI}imgui_impl_opengl3.o ${DIR_IMGUI}imgui_tables.o ${DIR_IMGUI}imgui_widgets.o 
../src/infrastructure/opengl.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale )
find_package(glfw3 REQUIRED)

target_include_directories(sandbox PRIVATE ./ ../src/debug ../src/infrastructure ${MSYS2}/include ../vendor/ ${DIR_IMGUI})

target_link_libraries(sandbox PRIVATE ${FFMPEG_LIBRARIES} glfw3)