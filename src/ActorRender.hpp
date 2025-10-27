#pragma once

#include "Rendering.hpp"

struct ActorRenderData {
    ModelKey model_key;
    float sec_count = 0;
    Vector3 offset = {0, 0, 0};

    int anim_id = 0;

    void Draw(Vector3 position, float yaw = 0.f, float pitch = 0.f) const {
        ModelAnimationData anim_data = Resources::Get().ModelAnimationDataFromKey(model_key);
        if (anim_data.anim_count > 0 && anim_data.model_animations != nullptr) {
            Model model = Resources::Get().ModelFromKey(model_key);
            ModelAnimation& anim = anim_data.model_animations[anim_id%anim_data.anim_count];
            int frame = int(sec_count*100)%anim.frameCount;
            UpdateModelAnimation(model, anim, frame);
        }

        if (model_key != R_MODEL_DEFAULT) {
            Rendering::Get().RenderModel(
                model_key,
                position+offset, Vector3{0, 1, 0},
                -yaw*180/PI + 90,
                Vector3{10, 10, 10},
                WHITE
            );
        }
    }    

    void UpdateAnim(bool running, float delta_time) {
        ModelAnimationData anim_data = Resources::Get().ModelAnimationDataFromKey(model_key);
        if (anim_data.anim_count > 0 && anim_data.model_animations != nullptr) {
            anim_id = running ? 11 : 3;
            sec_count += delta_time;
        }
    }
};