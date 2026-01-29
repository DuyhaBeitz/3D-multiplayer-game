#pragma once

#include <RaylibRetainedGUI/RaylibRetainedGUI.hpp>

enum Menus {
    MENU_PAUSE,
    MENU_SETTINGS
};

class MenusScreen {
private:
    Menus m_current_menu = MENU_PAUSE;

    // PAUSE
    std::shared_ptr<UIScreen> m_pause_screen = nullptr;
    std::shared_ptr<UIBar> m_pause_bar = nullptr;

    // SETTINGS
    std::shared_ptr<UIScreen> m_settings_screen = nullptr;
    std::shared_ptr<UIBar> m_settings_bar = nullptr;

    std::function<void()> m_on_resume = 0;
    
    void SetupPauseScreen();
    void SetupSettingsScreen();

public:
    MenusScreen() {
        SetupPauseScreen();
        SetupSettingsScreen();
    }

    void Update();
    void Draw();

    void BindOnResume(std::function<void()> func) {m_on_resume = func;}
};