#include "GameServer.hpp"
#include <thread>

std::unique_ptr<GameServer> game_server;
bool running = true;

int main(){
    std::cout << "Server running" << std::endl;
    EasyNetInit();
    game_server = std::make_unique<GameServer>();

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

    return 0;
}