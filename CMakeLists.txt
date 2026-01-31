cmake_minimum_required(VERSION 3.20)

project(Multiplayer LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build all dependencies as static libraries" FORCE)
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps")
set(FETCHCONTENT_QUIET OFF)
#set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
set(CMAKE_BUILD_TYPE Release)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        12.1.0
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    EasyNet
    GIT_REPOSITORY https://github.com/DuyhaBeitz/EasyNet.git
    GIT_TAG main
    UPDATE_COMMAND ""
)
FetchContent_MakeAvailable(EasyNet)

include(r3d.cmake)

FetchContent_Declare(
    RaylibRetainedGUI
    GIT_REPOSITORY https://github.com/DuyhaBeitz/RaylibRetainedGUI.git
    GIT_TAG master
    UPDATE_COMMAND ""
    GIT_TAG 0.0.1
)
FetchContent_MakeAvailable(RaylibRetainedGUI)

add_executable(server
    src/World.cpp
    src/Game.cpp
    src/server.cpp
    src/Physics.cpp
    src/Resources.cpp
    src/Rendering.cpp
)
target_link_libraries(server PUBLIC
    EasyNet
    fmt::fmt
    raylib
    r3d
)

add_executable(client 
    src/World.cpp
    src/Game.cpp
    src/client.cpp
    src/Physics.cpp
    src/Resources.cpp
    src/Rendering.cpp
    src/MenusUI.cpp
    src/Settings.cpp
)
add_executable(standalone 
    src/World.cpp
    src/Game.cpp
    src/standalone.cpp
    src/Physics.cpp
    src/Resources.cpp
    src/Rendering.cpp
    src/MenusUI.cpp
    src/Settings.cpp
)
add_executable(test
    src/World.cpp
    src/Game.cpp
    src/test.cpp
    src/Physics.cpp
    src/Resources.cpp
    src/Rendering.cpp
)

# r3d doesn't currently expose its public headers via target properties,
# so add its include directory explicitly for the application target.
if(NOT r3d_SOURCE_DIR)
    FetchContent_GetProperties(r3d)
endif()

if(NOT raylib_SOURCE_DIR)
    FetchContent_GetProperties(raylib)
endif()

target_include_directories(server
    PRIVATE
        src
        ${r3d_SOURCE_DIR}/include
        ${raylib_SOURCE_DIR}/include
)

target_include_directories(client
    PRIVATE
        src
        ${r3d_SOURCE_DIR}/include
        ${raylib_SOURCE_DIR}/include
)

target_include_directories(standalone
    PRIVATE
        src
        ${r3d_SOURCE_DIR}/include
        ${raylib_SOURCE_DIR}/include
)

target_include_directories(test
    PRIVATE
        src
        ${r3d_SOURCE_DIR}/include
        ${raylib_SOURCE_DIR}/include
)

target_link_libraries(client PRIVATE
    EasyNet
    fmt::fmt
    raylib
    RaylibRetainedGUI
    r3d
)


target_link_libraries(standalone PRIVATE
    raylib
    RaylibRetainedGUI
    r3d
)

target_link_libraries(test PRIVATE
    raylib
    RaylibRetainedGUI
    r3d
)

if(WIN32)
    target_link_libraries(server PRIVATE ws2_32)
    target_link_libraries(client PRIVATE ws2_32)
    target_link_libraries(standalone PRIVATE ws2_32)
    target_link_libraries(test PRIVATE ws2_32)
endif()