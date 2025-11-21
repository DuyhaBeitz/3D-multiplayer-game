// Define these BEFORE including raylib
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #define NOGDI
    #define NOUSER
    #include <windows.h>
#endif

// Then include raylib AFTER the Windows definitions
#include <raylib.h>
// Then include enet
#include <EasyNet/EasyNetShared.hpp>