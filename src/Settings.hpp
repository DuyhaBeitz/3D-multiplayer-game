#pragma once

class Settings {
private:
    Settings();

    ~Settings() {
    }

    float m_resolution_scale = 1.0f;
    
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
};