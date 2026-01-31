#include "FixWinConflicts.hpp"

#include "GameServer.hpp"
#include <thread>

//#define WITH_RENDER 1

std::unique_ptr<GameServer> game_server;
bool running = true;

int main(){
    EasyNetInit();
    game_server = std::make_unique<GameServer>();

    #ifdef WITH_RENDER
    InitWindow(1000, 1000, "Server");
    SetWindowState(FLAG_WINDOW_TOPMOST);
    SetTargetFPS(iters_per_sec);
    Rendering::Init();

    while (!WindowShouldClose()) {
        game_server->Update();
        BeginDrawing();
        game_server->DrawGame();
        EndDrawing();
    }
    R3D_Close();
    CloseWindow();
    #else
    auto next_tick = std::chrono::steady_clock::now();
    while (running) {
        auto now = std::chrono::steady_clock::now();

        while (now >= next_tick) {
            game_server->Update();

            next_tick += std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>(dt)
            );
        }
        std::this_thread::sleep_until(next_tick);
    }  
    #endif    

    return 0;
}