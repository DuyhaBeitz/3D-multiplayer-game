#include "Settings.hpp"

#include "Rendering.hpp"

Settings::Settings() {

}

void Settings::Update() {
    R3D_UpdateResolution(GetScreenWidth() * m_resolution_scale, GetScreenHeight() * m_resolution_scale);
    
    if (m_dof_enabled) {
        R3D_ENVIRONMENT_SET(dof.mode, R3D_DOF_ENABLED);
        R3D_ENVIRONMENT_SET(background.color, BLACK);        
        R3D_ENVIRONMENT_SET(dof.focusPoint, m_dof_focus_point*100);
        R3D_ENVIRONMENT_SET(dof.focusScale, m_dof_focus_scale);
        R3D_ENVIRONMENT_SET(dof.maxBlurSize, m_dof_max_blur);
        R3D_ENVIRONMENT_SET(dof.debugMode, false);
    }
    else {
        R3D_ENVIRONMENT_SET(dof.mode, R3D_DOF_DISABLED);
    }

    R3D_ENVIRONMENT_SET(tonemap.mode, R3D_Tonemap(m_tonemap_mode));
}