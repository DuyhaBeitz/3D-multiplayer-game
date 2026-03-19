
# Allow users to override git tags to pin versions when desired
set(RAYLIB_GIT_TAG 5.5 CACHE STRING "Git tag/branch for raylib")
set(ASSIMP_GIT_TAG v6.0.4 CACHE STRING "Git tag/branch for assimp")
set(R3D_GIT_TAG v0.9 CACHE STRING "Git tag/branch for r3d")

# Configure raylib similar to r3d's vendored setup to avoid symbol clashes
set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
set(SUPPORT_FILEFORMAT_OBJ OFF CACHE BOOL "" FORCE)
set(SUPPORT_FILEFORMAT_MTL OFF CACHE BOOL "" FORCE)
set(SUPPORT_FILEFORMAT_IQM OFF CACHE BOOL "" FORCE)
set(SUPPORT_FILEFORMAT_GLTF OFF CACHE BOOL "" FORCE)
set(SUPPORT_FILEFORMAT_VOX OFF CACHE BOOL "" FORCE)
set(SUPPORT_FILEFORMAT_M3D OFF CACHE BOOL "" FORCE)
set(SUPPORT_MESH_GENERATION OFF CACHE BOOL "" FORCE)

set(SUPPORT_FILEFORMAT_JPG ON CACHE BOOL "" FORCE)

# assimp knobs (leaner build; keep install/tests off)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)

set(R3D_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(R3D_RAYLIB_VENDORED ON CACHE BOOL "" FORCE)
set(R3D_ASSIMP_VENDORED ON CACHE BOOL "" FORCE)

# # Fetch raylib
# message(STATUS "Downloading and configuring raylib...")
# FetchContent_Declare(
#     raylib
#     GIT_REPOSITORY https://github.com/raysan5/raylib.git
#     GIT_TAG ${RAYLIB_GIT_TAG}
#     GIT_SHALLOW TRUE
# )
# FetchContent_MakeAvailable(raylib)

# # Fetch assimp
# message(STATUS "Downloading and configuring assimp...")
# FetchContent_Declare(
#     assimp
#     GIT_REPOSITORY https://github.com/assimp/assimp.git
#     GIT_TAG ${ASSIMP_GIT_TAG}
#     GIT_SHALLOW TRUE
# )
# FetchContent_MakeAvailable(assimp)

# Configure r3d to use the just-built raylib/assimp instead of its vendored copies

# r3d requires Python for shader/asset embedding
if(NOT DEFINED PYTHON_EXECUTABLE)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}" CACHE FILEPATH "" FORCE)
endif()

# Fetch r3d
message(STATUS "Downloading and configuring r3d...")
FetchContent_Declare(
    r3d
    GIT_REPOSITORY https://github.com/Bigfoot71/r3d.git
    GIT_TAG ${R3D_GIT_TAG}
    GIT_SHALLOW TRUE
    PATCH_COMMAND
        ${CMAKE_COMMAND}
            -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/r3d-src
            -DPATCH_FILE=${CMAKE_CURRENT_SOURCE_DIR}/raylib_assert.patch
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/r3d_patch.cmake
)
FetchContent_MakeAvailable(r3d)

target_include_directories(r3d PUBLIC
    ${r3d_BINARY_DIR}/generated/include
)