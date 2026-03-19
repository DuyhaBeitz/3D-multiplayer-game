# patch_raylib.cmake
# This script is executed by CMake's -P mode.

# The path to the file we need to patch is passed as a variable.
set(target_file "${SOURCE_DIR}/external/raylib/src/rmodels.c")
set(patch_file "${PATCH_FILE}")

message(STATUS "Checking if ${target_file} needs patching...")

# Read the current content of the file
file(READ "${target_file}" file_content)

# Check if the line we want to add is already present
if(NOT file_content MATCHES "#include <assert.h>")
    message(STATUS "Patching ${target_file}...")
    # Apply the patch using git apply. We can use the full path to the patch file.
    # Using 'git apply' is still okay here as it's a standalone command that doesn't need a shell.
    execute_process(
        COMMAND git apply -p1 "${patch_file}"
        WORKING_DIRECTORY "${SOURCE_DIR}/external/raylib"
        RESULT_VARIABLE patch_result
    )
    if(NOT patch_result EQUAL 0)
        message(FATAL_ERROR "Failed to apply patch: ${patch_file}")
    endif()
else()
    message(STATUS "File already patched. Skipping.")
endif()