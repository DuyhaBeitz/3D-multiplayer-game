
// #include "Game.hpp"
// #include "Serialization.hpp"

// void PrintTest(bool test, std::string name) {
//     std::cout << name << std::endl;
//     if (test) {
//         std::cout << "\t" << "PASSED +" << std::endl;
//     }
//     else {
//         std::cout << "\t" << "FAILED -" << std::endl;
//     }
// }

// int main() {
//     InitWindow(10, 10, "Test");

//     BodyData body_data;
//     CollisionShape sphere(SphereData{13.0});
//     body_data.shapes.push_back(sphere);
//     ActorData actor_data(body_data);

//     WorldData world_data;
    
//     ActorKey key = world_data.AddActor(actor_data);
//     world_data.GetActor(key).body.position = Vector3{10, 20, 30};
//     world_data.Update(10);

//     GameState game1;
//     game1.world_data = world_data;

//     Game game_manager = {};
//     GameState game2 = game1;
//     auto s1 = game_manager.Serialize(game1);
//     auto s2 = game_manager.Serialize(game1);

//     bool eq = (s1.tick == s2.tick) && (s1.size = s2.size);
//     if (eq) {
//         for (int i = 0; i < s1.size; i++) {
//             if (s1.bytes[i] != s2.bytes[i]) {
//                 eq = false;
//                 break;
//             }
//         }
//     }

//     PrintTest(eq, "Serialization");
// }

#include <r3d/r3d.h>
#include <raymath.h>

#ifndef RESOURCES_PATH
#	define RESOURCES_PATH "./assets/"
#endif
#include <iostream>
#include <vector>

int main(void)
{
    // Initialize window
    InitWindow(1920, 1080, "[r3d] - Animation example");
    SetTargetFPS(60);

    // Initialize R3D with FXAA
    R3D_Init(GetScreenWidth(), GetScreenHeight());
    R3D_SetAntiAliasing(R3D_ANTI_ALIASING_FXAA);

    // Setup environment sky
    R3D_Cubemap cubemap = R3D_LoadCubemap(RESOURCES_PATH "panorama/indoor.hdr", R3D_CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_ENVIRONMENT_SET(background.skyBlur, 0.3f);
    R3D_ENVIRONMENT_SET(background.energy, 0.6f);
    R3D_ENVIRONMENT_SET(background.sky, cubemap);

    // Setup environment ambient
    R3D_AmbientMap ambientMap = R3D_GenAmbientMap(cubemap, R3D_AMBIENT_ILLUMINATION);
    R3D_ENVIRONMENT_SET(ambient.map, ambientMap);
    R3D_ENVIRONMENT_SET(ambient.energy, 0.25f);

    // Setup tonemapping
    R3D_ENVIRONMENT_SET(tonemap.mode, R3D_TONEMAP_FILMIC);
    R3D_ENVIRONMENT_SET(tonemap.exposure, 0.75f);

    // Generate a ground plane and load the animated model
    R3D_Mesh plane = R3D_GenMeshPlane(10, 10, 1, 1);

    // Setup lights with shadows
    R3D_Light light = R3D_CreateLight(R3D_LIGHT_DIR);
    R3D_SetLightDirection(light, (Vector3){-1.0f, -1.0f, -1.0f});
    R3D_SetLightActive(light, true);
    R3D_SetLightRange(light, 10.0f);
    R3D_EnableShadow(light);

    // Setup camera
    Camera3D camera = {
        .position = {0, 1.5f, 6.0f},
        .target = {0, 0.75f, 0.0f},
        .up = {0, 1, 0},
        .fovy = 90
    };



    R3D_Model model = R3D_LoadModel(RESOURCES_PATH "models/Solus_the_knight.gltf");
    R3D_Model ball = R3D_LoadModel(RESOURCES_PATH "football_ball.glb");

    // Load animations
    R3D_AnimationLib modelAnims = R3D_LoadAnimationLib(RESOURCES_PATH "models/Solus_the_knight.gltf");
    std::cout << modelAnims.count << std::endl;
    int N = 21;
    std::vector<R3D_AnimationPlayer> animPlayers = {};
    for (int i = 0; i < N; i++) {
        animPlayers.push_back(R3D_LoadAnimationPlayer(model.skeleton, modelAnims));
        for (int j = 0; j < modelAnims.count; j++) {
            // Setup animation playing
            R3D_SetAnimationWeight(&animPlayers[i], j, (j == i) ? 1.f : 0.f);
            R3D_SetAnimationLoop(&animPlayers[i], j, true);
        }
    }



    // Main loop
    while (!WindowShouldClose())
    {
        float delta = GetFrameTime();
        //UpdateCamera(&camera, CAMERA_ORBITAL);
        
        BeginDrawing();
            ClearBackground(RAYWHITE);
            R3D_Begin(camera);

                R3D_DrawMesh(plane, R3D_MATERIAL_BASE, Vector3Zero(), 1.0f);
                for (int i = 0; i < N; i++) {
                    R3D_PlayAnimation(&animPlayers[i], i);    

                    R3D_UpdateAnimationPlayer(&animPlayers[i], delta);
                    R3D_DrawAnimatedModel(model, animPlayers[i], Vector3{(i-10)*1.0f, 0, 0}, 1.25f);
                    R3D_DrawModel(ball, {0, 1, 0}, 3.0f);
                }
                
            R3D_End();
        EndDrawing();
    }

    R3D_Close();

    CloseWindow();

    return 0;
}