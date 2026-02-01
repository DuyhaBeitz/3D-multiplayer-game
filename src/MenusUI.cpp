#include "MenusUI.hpp"
#include "Settings.hpp"
#include "WindowGlobal.hpp"

void MenusScreen::SetupPauseScreen() {
    m_pause_screen = std::make_shared<UIScreen>();
    m_pause_bar = std::make_shared<UIBar>(CenteredRect(0.9, 0.5));
    int elems = 6;
    Rectangle rect = SizeRect(1, 1.0f/elems);

    auto title = std::make_shared<UIText>("Pause", rect);    
    auto resume_button = std::make_shared<UIFuncButton>("Resume", rect);
    auto settings_button = std::make_shared<UIFuncButton>("Settings", rect);

    resume_button->BindOnReleased(
        [this](){if (m_on_resume) m_on_resume(); }
    );

    settings_button->BindOnReleased(
        [this](){m_current_menu = MENU_SETTINGS;}
    );
    
    auto close_button = std::make_shared<UIFuncButton>("Close game", rect);
    close_button->BindOnReleased(
        [](){ WindowGlobal::Get().SetRunning(false); }
    );

    //m_pause_bar->AddChild(title);
    m_pause_bar->AddChild(resume_button);
    m_pause_bar->AddChild(settings_button);
    m_pause_bar->AddChild(close_button);

    m_pause_screen->AddChild(m_pause_bar);
}

void MenusScreen::SetupSettingsScreen() {
    m_settings_screen = std::make_shared<UIScreen>();
    m_settings_bar = std::make_shared<UIScrollable>(Rectangle{0.6, 0.0, 0.4, 1.0});    

    int elems = 10;
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

    auto dof_enabled_text = std::make_shared<UIText>("Dof enabled");
    auto dof_enabled_button = std::make_shared<UIBoolButton>(Settings::Get().GetDofEnabledPtr());
    auto dof_enabled_split = std::make_shared<UISplit>(
        dof_enabled_text,
        dof_enabled_button,
        0.5f,
        rect
    );

    auto dof_focus_point_text = std::make_shared<UIText>("Dof focus point");
    auto dof_focus_point_button = std::make_shared<UIFloatButton>(Settings::Get().GetDofFocusPointPtr());
    auto dof_focus_point_split = std::make_shared<UISplit>(
        dof_focus_point_text,
        dof_focus_point_button,
        0.5f,
        rect
    );

    auto dof_focus_scale_text = std::make_shared<UIText>("Dof focus scale");
    auto dof_focus_scale_button = std::make_shared<UIFloatButton>(Settings::Get().GetDofFocusScalePtr());
    auto dof_focus_scale_split = std::make_shared<UISplit>(
        dof_focus_scale_text,
        dof_focus_scale_button,
        0.5f,
        rect
    );

    auto dof_max_blur_text = std::make_shared<UIText>("Dof max blur");
    auto dof_max_blur_button = std::make_shared<UIFloatButton>(Settings::Get().GetDofMaxBlurPtr());
    auto dof_max_blur_split = std::make_shared<UISplit>(
        dof_max_blur_text,
        dof_max_blur_button,
        0.5f,
        rect
    );

    auto back_button = std::make_shared<UIFuncButton>("Back", rect);

    back_button->BindOnReleased(
        [this](){m_current_menu = MENU_PAUSE;}
    );
    
    //m_settings_bar->AddChild(title);
    m_settings_bar->AddChild(resolution_scale_split);
    m_settings_bar->AddChild(dof_enabled_split);
    m_settings_bar->AddChild(dof_focus_point_split);
    m_settings_bar->AddChild(dof_focus_scale_split);
    m_settings_bar->AddChild(dof_max_blur_split);
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
