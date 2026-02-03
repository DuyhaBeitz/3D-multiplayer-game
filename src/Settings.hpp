#pragma once

class Settings {
private:
    Settings();

    ~Settings() {
    }

    float m_resolution_scale = 0.5f;

    bool m_dof_enabled = false;
    float m_dof_focus_point = 2.0f;
    float m_dof_focus_scale = 3.0;
    float m_dof_max_blur = 20.0;

    int m_tonemap_mode = 3;
    
public:
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    static Settings& Get() {
        static Settings instance;
        return instance;
    }

    static void Init() {
        Settings::Get();
    }

    void Update();

    float* GetResolutionScalePtr() { return &m_resolution_scale; }
    
    bool* GetDofEnabledPtr() { return &m_dof_enabled; }
    float* GetDofFocusPointPtr() { return &m_dof_focus_point; }
    float* GetDofFocusScalePtr() { return &m_dof_focus_scale; }
    float* GetDofMaxBlurPtr() { return &m_dof_max_blur; }

    int* GetTonemapModePtr() { return &m_tonemap_mode; }
};