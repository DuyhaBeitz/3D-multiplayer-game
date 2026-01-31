cmake_minimum_required(VERSION 3.20)
project(Multiplayer LANGUAGES CXX VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build all dependencies as static libraries" FORCE)
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps")
set(FETCHCONTENT_QUIET OFF)
set(CMAKE_BUILD_TYPE Release)

option(HEADLESS_BUILD "Build only server" OFF)

if(HEADLESS_BUILD)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/headless.cmake)
else()
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/regular.cmake)
endif()
