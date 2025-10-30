#include "GameServer.hpp"
#include <thread>

#define VIS 1

std::unique_ptr<GameServer> game_server;
bool running = true;

int main(){
    EasyNetInit();
    game_server = std::make_unique<GameServer>();

    #ifdef VIS
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
    #else
    InitWindow(1000, 1000, "Headless server");
    SetWindowState(FLAG_WINDOW_HIDDEN);
    SetTargetFPS(iters_per_sec); // not actually used
    Rendering::Init();
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


// #include <r3d.h>
// #include <raymath.h>

// int main(void)
// {
//     InitWindow(800, 600, "R3D Example");
//     R3D_Init(800, 600, 0);

//     // Create scene objects
//     R3D_Mesh mesh = R3D_GenMeshSphere(1.0f, 16, 32, true);
//     R3D_Material material = R3D_GetDefaultMaterial();
    
//     // Setup lighting
//     R3D_Light light = R3D_CreateLight(R3D_LIGHT_DIR);
//     R3D_SetLightDirection(light, (Vector3){ -1, -1, -1 });
//     R3D_SetLightActive(light, true);
    
//     // Camera setup
//     Camera3D camera = {
//         .position = { -3, 3, 3 },
//         .target = { 0, 0, 0 },
//         .up = { 0, 1, 0 },
//         .fovy = 60.0f,
//         .projection = CAMERA_PERSPECTIVE
//     };

//     // Main loop
//     while (!WindowShouldClose()) {
//         BeginDrawing();
//         R3D_Begin(camera);
//         //R3D_DrawMesh(&mesh, &material, MatrixIdentity());
//         DrawCubeV(Vector3{0, 0, 0}, Vector3{1, 1, 1}, WHITE);
//         R3D_End();
//         EndDrawing();
//     }

//     R3D_UnloadMesh(&mesh);
//     R3D_Close();
//     CloseWindow();
//     return 0;
// }