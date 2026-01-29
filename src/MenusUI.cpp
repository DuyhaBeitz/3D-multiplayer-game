#include "MenusUI.hpp"
#include "Settings.hpp"

void MenusScreen::SetupPauseScreen() {
    m_pause_screen = std::make_shared<UIScreen>();
    m_pause_bar = std::make_shared<UIBar>(CenteredRect(0.9, 0.5));
    int elems = 6;
    Rectangle rect = SizeRect(1, 1.0f/elems);

    auto title = std::make_shared<UIText>("Pause", rect);    
    auto resume_button = std::make_shared<UIFuncButton>("Resume", rect);
    auto settings_button = std::make_shared<UIFuncButton>("settings", rect);

    resume_button->BindOnReleased(
        [this](){if (m_on_resume) m_on_resume(); }
    );

    settings_button->BindOnReleased(
        [this](){m_current_menu = MENU_SETTINGS;}
    );
    
    //m_pause_bar->AddChild(title);
    m_pause_bar->AddChild(resume_button);
    m_pause_bar->AddChild(settings_button);

    m_pause_screen->AddChild(m_pause_bar);
}

void MenusScreen::SetupSettingsScreen() {
    m_settings_screen = std::make_shared<UIScreen>();
    m_settings_bar = std::make_shared<UIBar>(CenteredRect(0.9, 0.5));
    int elems = 6;
    Rectangle rect = SizeRect(1, 1.0f/elems);

    auto title = std::make_shared<UIText>("Settings", rect);    
    
    auto resolution_scale_text = std::make_shared<UIText>("Resolution scale");
    auto resolution_scale_slider = std::make_shared<UISlider>(
        Orientation::Horizontal,
        Settings::Get().GetResolutionScalePtr()
    );
    auto resolution_scale_split = std::make_shared<UISplit>(
        resolution_scale_text,
        resolution_scale_slider,
        0.5f,
        rect
    );

    auto back_button = std::make_shared<UIFuncButton>("Back", rect);

    back_button->BindOnReleased(
        [this](){m_current_menu = MENU_PAUSE;}
    );
    
    //m_settings_bar->AddChild(title);
    m_settings_bar->AddChild(resolution_scale_split);
    m_settings_bar->AddChild(back_button);

    m_settings_screen->AddChild(m_settings_bar);
}

void MenusScreen::Update() {
    switch (m_current_menu) {
    case MENU_PAUSE:
        m_pause_screen->Update(nullptr);
        break;
    case MENU_SETTINGS:
        m_settings_screen->Update(nullptr);
        break;
    default:
        break;
    }
}

void MenusScreen::Draw() {
    switch (m_current_menu) {
    case MENU_PAUSE:
        m_pause_screen->Draw();
        break;
    case MENU_SETTINGS:
        m_settings_screen->Draw();
        break;
    default:
        break;
    }
}
