include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/shared.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/r3d.cmake)

add_compile_definitions(WITH_RENDER)

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