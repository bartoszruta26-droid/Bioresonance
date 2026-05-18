cmake_minimum_required(VERSION 3.10)
project(BioresonanceGUI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opcje buildu
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Znajdź SDL2
find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
    find_package(SDL2 REQUIRED)
endif()

# ImGui sources
set(IMGUI_SOURCES
    imgui_lib/imgui.cpp
    imgui_lib/imgui_demo.cpp
    imgui_lib/imgui_draw.cpp
    imgui_lib/imgui_tables.cpp
    imgui_lib/imgui_widgets.cpp
    imgui_lib/backends/imgui_impl_sdl2.cpp
    imgui_lib/backends/imgui_impl_sdlrenderer2.cpp
)

# Pliki źródłowe
set(SOURCES
    src/main.cpp
    src/gui_app.cpp
    src/network_client.cpp
    src/probe_manager.cpp
    src/frequency_loader.cpp
    ${IMGUI_SOURCES}
)

set(HEADERS
    include/gui_app.h
    include/network_client.h
    include/probe_manager.h
    include/frequency_loader.h
    include/types.h
)

# Główna aplikacja
add_executable(bioresonance_gui 
    ${SOURCES} 
    ${HEADERS}
)

target_include_directories(bioresonance_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_lib
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_lib/backends
)

# Linkowanie SDL2
if(TARGET SDL2::SDL2)
    target_link_libraries(bioresonance_gui PRIVATE SDL2::SDL2)
else()
    target_link_libraries(bioresonance_gui PRIVATE SDL2)
endif()

# Platform-specific settings
if(WIN32)
    target_compile_definitions(bioresonance_gui PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(bioresonance_gui PRIVATE ws2_32 imm32 winmm)
elseif(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(METAL_FRAMEWORK Metal)
    find_library(METALKIT_FRAMEWORK MetalKit)
    target_link_libraries(bioresonance_gui PRIVATE ${COCOA_FRAMEWORK} ${METAL_FRAMEWORK} ${METALKIT_FRAMEWORK})
elseif(UNIX)
    find_package(X11)
    if(X11_FOUND)
        target_include_directories(bioresonance_gui PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(bioresonance_gui PRIVATE ${X11_LIBRARIES})
    endif()
    target_link_libraries(bioresonance_gui PRIVATE dl)
endif()

# Instalacja
install(TARGETS bioresonance_gui DESTINATION bin)

# Copy frequencies.md to build directory
configure_file(${CMAKE_SOURCE_DIR}/../frequencies.md ${CMAKE_BINARY_DIR}/frequencies.md COPYONLY)
