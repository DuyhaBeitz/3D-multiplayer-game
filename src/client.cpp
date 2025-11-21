
#include "FixWinConflicts.hpp"
#include <RaylibRetainedGUI/RaylibRetainedGUI.hpp>

#include "GameClient.hpp"

std::shared_ptr<UIScreen> screen;
std::shared_ptr<UIBar> bar;
std::shared_ptr<UIFuncButton> connect_button;

std::unique_ptr<GameClient> game_client;
std::shared_ptr<EasyNetClient> net_client;

std::string server_ip = "127.0.0.1";

void Init();

int main() {
    Init();

    while (!WindowShouldClose()) {
        net_client->Update();

        if (!game_client->IsConnected()) {
            screen->Update(nullptr);
            BeginDrawing();
            ClearBackground(LIGHTGRAY);
            screen->Draw();
            EndDrawing();
        }
        else {
            game_client->Update();
            BeginDrawing();
            game_client->DrawGame();
            EndDrawing();
        }
    }
    net_client->RequestDisconnectFromServer();
    net_client->Update();
    CloseWindow();
    return 0;
}

void Init() {
    EasyNetInit();
    InitWindow(1920/2, 1080, "Client");
    SetWindowState(FLAG_WINDOW_TOPMOST);
    SetTargetFPS(iters_per_sec);

    Resources::Init();

    game_client = std::make_unique<GameClient>();
    net_client = game_client->GetNetClient();

    /********** UI **********/

    UIElement::SetDefaultStyle(std::make_shared<UIStyle>(Resources::Get().FontFromKey(R_FONT_DEFAULT)));

    screen = std::make_shared<UIScreen>();
    bar = std::make_shared<UIBar>(CenteredRect(0.9, 0.5));
    Rectangle rect = SizeRect(1, 0.33);

    auto ip_text = std::make_shared<UIText>("IP  ");    
    auto server_ip_button = std::make_shared<UIStringButton>(&server_ip, rect);
    auto split_ip = std::make_shared<UISplit>(ip_text, server_ip_button, 0.3, rect);

    auto port_text = std::make_shared<UIText>("PORT");
    auto server_port_button = std::make_shared<UIIntButton>(&server_port, rect);
    auto split_port = std::make_shared<UISplit>(port_text, server_port_button, 0.3, rect);

    connect_button = std::make_shared<UIFuncButton>("Connect", rect);

    bar->AddChild(split_ip);
    bar->AddChild(split_port);
    bar->AddChild(connect_button);
    
    screen->AddChild(bar);

    /********** LOGIC **********/
    connect_button->BindOnReleased([](){
            net_client->RequestConnectToServer(server_ip, server_port);
        }
    );

    constexpr int server_count = 3;
    const char* servers[server_count] = {
        "127.0.0.1",
        "45.159.79.84",
        "185.245.34.7"
    };
    int i = 0;
    while (!net_client->RequestConnectToServer(servers[i], 7777) && i < server_count) {
        i++;
    }
}