#include "Settings.hpp"

#include "Rendering.hpp"

Settings::Settings() {

}

void Settings::Update() {
    R3D_UpdateResolution(GetScreenWidth() * m_resolution_scale, GetScreenHeight() * m_resolution_scale);
}