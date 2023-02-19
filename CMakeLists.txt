cmake_minimum_required(VERSION 3.15)

project(MCK_GUI VERSION 0.0.1)

add_subdirectory(concurrentqueue)
add_subdirectory(cpp-httplib)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKWK REQUIRED gtk+-3.0 webkit2gtk-4.0)

add_library(MckGui)

target_compile_definitions(MckGui PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

target_include_directories(MckGui
    PRIVATE
    ./include
    ./webview
    ./concurrentqueue
    ./cpp-httplib
    ${GTKWK_INCLUDE_DIRS}
)

target_compile_options(MckGui PRIVATE
    ${GTK_CFLAGS_OTHER}
)

target_sources(MckGui
    PRIVATE
    src/GuiWindow.cpp)

target_link_libraries(MckGui
    PRIVATE
    concurrentqueue
    httplib
    ${GTKWK_LIBRARIES}
)