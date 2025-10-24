#pragma once

#include "Physics.hpp"
#include "Resources.hpp"
#include <iostream>

struct ActorData {
    BodyData body;
    float yaw;
    float pitch;
    
    ActorData(const BodyData& body_) : body(body_), yaw(0.0f), pitch(0.0f)
    {
    }

    void Update(float dt) {
        body.velocity.y -= gravity*dt;
        body.position += body.velocity;
        body.position.y = fmax(floor_lvl, body.position.y);
        body.velocity *= 0.7;
    }

    void Draw() const {
        DrawModelEx(Resources::Get().ModelFromKey(R_MODEL_PLAYER), body.position, Vector3{0, 1, 0}, -yaw*180/PI + 90, Vector3{10, 10, 10}, WHITE);
    }

    Vector3 VForward() const {
        return Vector3{cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)};
    }

    Vector3 VRight() const {
        return Vector3{cos(yaw+PI/2) * cos(pitch), 0, sin(yaw+PI/2) * cos(pitch)};
    }
};