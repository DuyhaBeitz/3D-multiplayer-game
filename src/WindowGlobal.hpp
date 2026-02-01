#pragma once

class WindowGlobal {
private:
    WindowGlobal();

    ~WindowGlobal() {
    }

    bool m_running = true;

public:
    WindowGlobal(const WindowGlobal&) = delete;
    WindowGlobal& operator=(const WindowGlobal&) = delete;

    static WindowGlobal& Get() {
        static WindowGlobal instance;
        return instance;
    }

    static void Init() {
        WindowGlobal::Get();
    }

    void Update();

    void ToggleWindow();

    void ToggleCursor();


    void SetRunning(bool running) { m_running = running; }
    bool IsRunning();
};