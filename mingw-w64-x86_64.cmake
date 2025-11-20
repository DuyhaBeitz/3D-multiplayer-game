set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Install target path for raylib inside MinGW root
set(CMAKE_INSTALL_PREFIX /usr/x86_64-w64-mingw32)

# Host tools should be found normally
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Target libs/headers only inside MinGW root
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)