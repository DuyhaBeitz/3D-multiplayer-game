#pragma once

#include "Rendering.hpp"

struct ActorRenderData {
    ModelKey model_key;
    int frame;
    Vector3 offset = {0, 0, 0};

    int anim_id = 0;

    void Draw(Vector3 position, float yaw = 0.f, float pitch = 0.f) const {
        AnimatedModelAlias& model_aliased = Resources::Get().ModelFromKey(model_key);
        if (model_aliased.anim_count > 0) {
            model_aliased.SetAnim(anim_id);
            model_aliased.SetAnimFrame(frame);            
        }

        if (model_key != R_MODEL_DEFAULT) {
            Rendering::Get().RenderModel(
                model_key,
                position+offset, Vector3{0, 1, 0},
                -yaw + PI/2,
                Vector3{10, 10, 10}
            );
        }
    }    

    void UpdateAnim(bool running, float delta_time) {
        AnimatedModelAlias& model_aliased = Resources::Get().ModelFromKey(model_key);
        if (model_aliased.anim_count > 0) {
            anim_id = running ? 11 : 3;            
            frame++;
        }
    }

    template <class Archive>
    void serialize(Archive& ar) {
        ar(model_key, frame, offset, anim_id);
    }
};