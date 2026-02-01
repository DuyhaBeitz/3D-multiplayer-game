#include "Settings.hpp"
#include "MenusUI.hpp"
#include "GameStandalone.hpp"

std::unique_ptr<GameStandalone> game;
std::unique_ptr<MenusScreen> menus_screen = nullptr;

enum GameScreens {
    PLAYING = 0,
    MENUS,
};

GameScreens current_screen = PLAYING;

void Init();

int main() {
    Init();

    while (WindowGlobal::Get().IsRunning()) {

        switch (current_screen) {
            case PLAYING:
                {
                    GameInput input;
                    input.Detect();
                    game->Update(input);    
                    BeginDrawing();
                    game->DrawGame();
                    EndDrawing();

                    if (IsKeyReleased(KEY_ESCAPE)) {
                        current_screen = MENUS;
                        EnableCursor();
                    }
                }

                break;
            case MENUS:
                game->Update({}); // empty input
                
                menus_screen->Update();
                BeginDrawing();
                game->DrawGame();
                menus_screen->Draw();
                EndDrawing();
        }
        WindowGlobal::Get().Update();
        Settings::Get().Update();
    }
    CloseWindow();
    return 0;
}

void Init() {
    SetTraceLogLevel(raylib_log_level);

    InitWindow(1000, 1000, "Standalone");
    SetWindowState(FLAG_WINDOW_TOPMOST);
    SetTargetFPS(iters_per_sec);
    SetExitKey(KEY_NULL);

    Rendering::Init();

    game = std::make_unique<GameStandalone>(); 


    UIElement::SetDefaultStyle(std::make_shared<UIStyle>(Resources::Get().FontFromKey(R_FONT_DEFAULT)));

    menus_screen = std::make_unique<MenusScreen>();
    menus_screen->BindOnResume([](){ current_screen = PLAYING; DisableCursor(); });
}