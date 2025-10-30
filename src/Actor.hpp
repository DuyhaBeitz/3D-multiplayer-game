#pragma once

#include "Physics.hpp"
#include "Rendering.hpp"
#include "ActorRender.hpp"
#include <iostream>

struct ActorData {
    BodyData body;
    float yaw;
    float pitch;
    char name[64] = "\0";
    
    ActorRenderData render_data;
    ActorData() = default;
    ActorData(const BodyData& body_) : body(body_), yaw(0.0f), pitch(0.0f), render_data(R_MODEL_DEFAULT, 0)
    {
    }

    ActorData(const BodyData& body_, ModelKey model_key_) : body(body_), yaw(0.0f), pitch(0.0f), render_data(model_key_, 0)
    {
    }

    void Update(float delta_time) {
        if (body.inverse_mass != 0.0) {
            body.ApplyForce({0, -gravity/body.inverse_mass, 0});
            body.ApplyForce(body.velocity * -2);
        }        
        Vector2 hor_vel = {body.velocity.x, body.velocity.z};
        render_data.UpdateAnim(Vector2Length(hor_vel) > 20, delta_time);
        body.Update(delta_time);
    }

    void Draw() const {
        render_data.Draw(body, yaw, pitch, name);        
        if (render_data.model_key == R_MODEL_DEFAULT) body.DrawShapes();
    }

    Vector3 VForward() const {
        return Vector3{cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)};
    }

    Vector3 VRight() const {
        return Vector3{cos(yaw+PI/2) * cos(pitch), 0, sin(yaw+PI/2) * cos(pitch)};
    }

    template <class Archive>
    void serialize(Archive& ar) {
        ar(body, yaw, pitch, render_data, name);
    }
};