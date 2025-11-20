#include <RaylibRetainedGUI.hpp>

#include "GameStandalone.hpp"

std::unique_ptr<GameStandalone> game;

void Init();

int main() {
    Init();

    while (!WindowShouldClose()) {
        game->Update();
        BeginDrawing();
        game->DrawGame();
        EndDrawing();
    }

    return 0;
}

void Init() {
    InitWindow(1000, 1000, "Standalone");
    SetWindowState(FLAG_WINDOW_TOPMOST);
    SetTargetFPS(iters_per_sec);

    game = std::make_unique<GameStandalone>(); 
}