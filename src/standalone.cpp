#include "GameStandalone.hpp"

std::unique_ptr<GameStandalone> game;

void Init();

int main() {
    Init();

    while (!WindowShouldClose()) {
        GameInput input;
        input.Detect();
        game->Update(input);    
        
        BeginDrawing();
        game->DrawGame();
        EndDrawing();
        R3D_UpdateResolution(GetScreenWidth(), GetScreenHeight());
    }
    CloseWindow();
    return 0;
}

void Init() {
    SetTraceLogLevel(raylib_log_level);

    InitWindow(1000, 1000, "Standalone");
    SetWindowState(FLAG_WINDOW_TOPMOST);
    SetTargetFPS(iters_per_sec);

    game = std::make_unique<GameStandalone>(); 
}