
#include "FixWinConflicts.hpp"
#include <RaylibRetainedGUI/RaylibRetainedGUI.hpp>

#include "Settings.hpp"
#include "MenusUI.hpp"
#include "GameClient.hpp"

#include "configparser/configparser.hpp"

std::shared_ptr<UIScreen> connect_screen = nullptr;
std::shared_ptr<UIBar> connect_bar = nullptr;
std::shared_ptr<UIFuncButton> connect_button = nullptr;

std::unique_ptr<MenusScreen> menus_screen = nullptr;

std::unique_ptr<GameClient> game_client = nullptr;
std::shared_ptr<EasyNetClient> net_client = nullptr;

std::string server_ip = "127.0.0.1"; // buffer for ui ip input

std::vector<std::string> servers{};

void Init();
void UpdateGame(float& accumulator);

enum GameScreens {
    PLAYING = 0,
    MENUS,
};

GameScreens current_screen = PLAYING;

int main() {
    Init();

    float accumulator = 0.0f;

    while (WindowGlobal::Get().IsRunning()) {
        net_client->Update();

        if (!game_client->IsConnected()) {
            connect_screen->Update(nullptr);
            BeginDrawing();
            ClearBackground(LIGHTGRAY);
            connect_screen->Draw();
            EndDrawing();
        }
        else {
            switch (current_screen) {
                case PLAYING:
                    UpdateGame(accumulator);
                    BeginDrawing();
                    game_client->DrawGame();
                    EndDrawing();
                    
                    if (IsKeyReleased(KEY_ESCAPE)) {
                        current_screen = MENUS;
                        EnableCursor();
                    }
                    break;
                case MENUS:
                    game_client->Update({}); // empty input
                    
                    menus_screen->Update();
                    BeginDrawing();
                    game_client->DrawGame();
                    menus_screen->Draw();
                    EndDrawing();
            }
        }
        WindowGlobal::Get().Update();
        Settings::Get().Update();
    }
    net_client->RequestDisconnectFromServer();
    net_client->Update();
    Resources::Get().Unload();
    R3D_Close();
    CloseWindow();
    return 0;
}

void Init() {
    EasyNetInit();
    
    Settings::Init();

    SetTraceLogLevel(raylib_log_level);
    InitWindow(1920/2, 1080, "Client");
    SetWindowState(FLAG_WINDOW_TOPMOST);
    SetTargetFPS(iters_per_sec);
    SetExitKey(KEY_NULL);
    
    Rendering::Init();

    game_client = std::make_unique<GameClient>();
    net_client = game_client->GetNetClient();

    /********** Load config **********/
    // if the file doesn't exist, the vector just will be empty
    std::string config_path = "client_config.ini";
    ConfigParser parser = ConfigParser(config_path);
    servers = parser.aConfigVec<std::string>("Client", "server_ips");

    /********** UI **********/

    UIElement::SetDefaultStyle(std::make_shared<UIStyle>(Resources::Get().FontFromKey(R_FONT_DEFAULT)));

    connect_screen = std::make_shared<UIScreen>();
    connect_bar = std::make_shared<UIBar>(CenteredRect(0.9, 0.5));
    int elems = 3 + servers.size();
    Rectangle rect = SizeRect(1, 1.0f/elems);

    auto ip_text = std::make_shared<UIText>("IP  ");    
    auto server_ip_button = std::make_shared<UIStringButton>(&server_ip);
    auto split_ip = std::make_shared<UISplit>(ip_text, server_ip_button, 0.3f, rect);

    auto port_text = std::make_shared<UIText>("PORT");
    auto server_port_button = std::make_shared<UIIntButton>(&server_port);
    auto split_port = std::make_shared<UISplit>(port_text, server_port_button, 0.3f, rect);

    connect_button = std::make_shared<UIFuncButton>("Connect", rect);

    connect_bar->AddChild(split_ip);
    connect_bar->AddChild(split_port);
    connect_bar->AddChild(connect_button);
    
    // Will be removed
    for (int i = 0; i < servers.size(); i++) {
        auto button = std::make_shared<UIFuncButton>(TextFormat("Server%d", i), rect);
        
        button->BindOnReleased([i](){
                net_client->ConnectToServer(servers[i], 7777);
            }
        );
        connect_bar->AddChild(button);
    }

    connect_screen->AddChild(connect_bar);

    connect_button->BindOnReleased([](){
            net_client->ConnectToServer(server_ip, server_port);
        }
    );

    menus_screen = std::make_unique<MenusScreen>();
    menus_screen->BindOnResume([](){ current_screen = PLAYING; DisableCursor(); });
}

void UpdateGame(float& accumulator) {
    accumulator += GetFrameTime();
    GameInput input;
    input.Detect();

    int i = 0;
    while (accumulator >= dt) {
        accumulator -= dt;
        i++;
    }
    input.Divide(i);

    game_client->Update(input);
    input.ClearNonContinuous();

    for (int j = 1; j < i; j++) {
        game_client->Update(input);
    }
}