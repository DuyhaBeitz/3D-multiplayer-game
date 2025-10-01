cmake_minimum_required(VERSION 3.20)

project(Multiplayer LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)

find_package(RaylibRetainedGUI)
find_package(EasyNet)
find_package(nlohmann_json REQUIRED)
find_package(raylib 5.0 REQUIRED)

add_executable(server src/server.cpp)
target_link_libraries(server PRIVATE
    EasyNet
    raylib
    nlohmann_json::nlohmann_json
)

add_executable(client src/client.cpp)
target_link_libraries(client PRIVATE
    EasyNet
    raylib
    RaylibRetainedGUI
    nlohmann_json::nlohmann_json
)