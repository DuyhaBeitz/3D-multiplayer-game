#pragma once

#include "Physics.hpp"
#include "Resources.hpp"
#include <iostream>

struct ActorData {
    BodyData body;
    float yaw;
    float pitch;
    uint16_t model_key;
    
    ActorData(const BodyData& body_) : body(body_), yaw(0.0f), pitch(0.0f), model_key(R_MODEL_DEFAULT)
    {
    }

    ActorData(const BodyData& body_, uint16_t model_key_) : body(body_), yaw(0.0f), pitch(0.0f), model_key(model_key_)
    {
    }

    void Update(float delta_time) {
        if (body.inverse_mass != 0.0) {
            body.ApplyForce({0, -gravity/body.inverse_mass, 0});
            body.ApplyForce(body.velocity * -2);
        }        
        
        body.Update(delta_time);
        //body.position.y = fmax(floor_lvl, body.position.y);
    }

    void Draw() const {
        // if (model_key != R_MODEL_DEFAULT) {
        //     DrawModelEx(
        //         Resources::Get().ModelFromKey(model_key),
        //         body.position, Vector3{0, 1, 0},
        //         -yaw*180/PI + 90,
        //         Vector3{10, 10, 10},
        //         WHITE
        //     );
        // }
        
        body.DrawShapes();
    }

    Vector3 VForward() const {
        return Vector3{cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)};
    }

    Vector3 VRight() const {
        return Vector3{cos(yaw+PI/2) * cos(pitch), 0, sin(yaw+PI/2) * cos(pitch)};
    }
};