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

#define ADD_SETTING(name, ptr, UI_CLASS) \
    auto name##_text = std::make_shared<UIText>(#name); \
    auto name##_button = std::make_shared<UI_CLASS>(ptr); \
    auto name##_split = std::make_shared<UISplit>( \
        name##_text, \
        name##_button, \
        0.5f, \
        rect \
    ); \
    m_settings_bar->AddChild(name##_split); \

#define ADD_SETTING_WTIH_PRE_PARAMS(name, ptr, UI_CLASS, ParamsPre) \
    auto name##_text = std::make_shared<UIText>(#name); \
    auto name##_button = std::make_shared<UI_CLASS>(ParamsPre, ptr); \
    auto name##_split = std::make_shared<UISplit>( \
        name##_text, \
        name##_button, \
        0.5f, \
        rect \
    ); \
    m_settings_bar->AddChild(name##_split); \

#define ADD_SETTING_BOOL(name, bool_ptr) ADD_SETTING(name, bool_ptr, UIBoolButton)
#define ADD_SETTING_SLIDER(name, float_ptr) ADD_SETTING_WTIH_PRE_PARAMS(name, float_ptr, UISlider, Orientation::Horizontal)
#define ADD_SETTING_FLOAT(name, float_ptr) ADD_SETTING(name, float_ptr, UIFloatButton)

#define ADD_SETTING_ENUM(name, int_ptr, names_vector) ADD_SETTING(name, int_ptr, UIEnum) \
    for (int k = 0; k < names_vector.size(); k++) { \
        name##_button->AddItem(names_vector[k]);    \
    }\


void MenusScreen::SetupSettingsScreen() {
    m_settings_screen = std::make_shared<UIScreen>();
    m_settings_bar = std::make_shared<UIScrollable>(Rectangle{0.6, 0.0, 0.4, 1.0});    

    int elems = 10;
    Rectangle rect = SizeRect(1, 1.0f/elems);

    auto title = std::make_shared<UIText>("Settings", rect);    
    
    ADD_SETTING_SLIDER(Resolution_scale, Settings::Get().GetResolutionScalePtr())
    ADD_SETTING_BOOL(Dof_enabled, Settings::Get().GetDofEnabledPtr())
    ADD_SETTING_SLIDER(Dof_focus_point, Settings::Get().GetDofFocusPointPtr())
    ADD_SETTING_FLOAT(Dof_focus_scale, Settings::Get().GetDofFocusScalePtr())
    ADD_SETTING_FLOAT(Dof_max_blur, Settings::Get().GetDofMaxBlurPtr())

    std::vector<std::string> tonemap_modes = {
        "LINEAR",
        "REINHARD",   
        "FILMIC",
        "ACES",
        "AGX",
    };
    ADD_SETTING_ENUM(Tonemap_mode, Settings::Get().GetTonemapModePtr(), tonemap_modes)

    auto back_button = std::make_shared<UIFuncButton>("Back", rect);
    back_button->BindOnReleased(
        [this](){m_current_menu = MENU_PAUSE;}
    );
    
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
