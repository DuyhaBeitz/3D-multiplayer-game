#include "WindowGlobal.hpp"

#include <raylib.h>

WindowGlobal::WindowGlobal() {

}

void WindowGlobal::Update() {
    if (IsKeyPressed(KEY_F11)) {
        ToggleWindow();
    }
}

void WindowGlobal::ToggleWindow() {
    bool cursor_was_hidden = IsCursorHidden();

    EnableCursor();
    ToggleBorderlessWindowed();
    BeginDrawing();
    EndDrawing();
    if (cursor_was_hidden) DisableCursor();
    BeginDrawing();
    EndDrawing();  
}

void WindowGlobal::ToggleCursor(){
    if (IsCursorHidden()){
        EnableCursor();
    }
    else {
        DisableCursor();
    }
}

bool WindowGlobal::IsRunning() {
    return m_running && !WindowShouldClose();
}
