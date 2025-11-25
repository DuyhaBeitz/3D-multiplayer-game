#include "GameStandalone.hpp"

std::unique_ptr<GameStandalone> game;

void Init();

int main() {
    Init();

    float accumulator = 0.0f;
    while (!WindowShouldClose()) {
        accumulator += GetFrameTime();
        GameInput input;
        input.Detect();
        while (accumulator >= dt) {
            game->Update(input);
            accumulator -= dt;
            input.ClearNonContinuous();
        }
        
        BeginDrawing();
        game->DrawGame();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}

void Init() {
    InitWindow(1000, 1000, "Standalone");
    SetWindowState(FLAG_WINDOW_TOPMOST);
    SetTargetFPS(iters_per_sec);

    game = std::make_unique<GameStandalone>(); 
}