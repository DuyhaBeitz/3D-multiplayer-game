#pragma once

#include <raylib.h>
#include "ResourceData.hpp"
#include "Physics.hpp"

#if WITH_RENDER
#include "Rendering.hpp"
#endif

struct ActorRenderData {
    ModelKey model_key{};
    Vector3 offset{};
    ActorRenderData() = default;
    ActorRenderData(ModelKey model_key_, Vector3 offset_ = Vector3{0, 0, 0}) : model_key(model_key_), offset(offset_) {}

#if WITH_RENDER
    void Draw(bool running, const BodyData& body, const float yaw, const float pitch) const {
        
        ModelAliased& model_aliased = Resources::Get().ModelFromKey(model_key);
        if (model_aliased.IsAnimated()) {
            int anim_id = running ? 5 : 15;            
            model_aliased.SetAnim(anim_id);
            model_aliased.Update(dt);
        }

        if (model_key != R_MODEL_DEFAULT) {
            Rendering::Get().RenderModel(
                model_key,
                body.position+offset, Vector3{0, 1, 0},
                -yaw + PI/2,
                Vector3{10, 10, 10}
            );
        }
    }    
#endif

    template <class Archive>
    void serialize(Archive& ar) {
        ar(model_key, offset);
    }
};