#pragma once

#include "Physics.hpp"
#include <iostream>
#include "GameDrawingData.hpp"
#include "ActorRender.hpp"

struct ActorData {
    BodyData body{};
    float yaw = 0.0f;
    float pitch = 0.0f;
    
    ActorRenderData render_data;
    
    ActorData() = default;
    ActorData(const BodyData& body_) : body(body_), yaw(0.0f), pitch(0.0f), render_data(R_MODEL_DEFAULT)
    {
    }

    ActorData(const BodyData& body_, ModelKey model_key_) : body(body_), yaw(0.0f), pitch(0.0f), render_data(model_key_)
    {
    }

    void Update(float delta_time) {
        if (body.inverse_mass != 0.0) {
            body.ApplyForce({0, -gravity/body.inverse_mass, 0});
            body.ApplyForce(body.velocity * -2);
        }        
        //Vector2 hor_vel = {body.velocity.x, body.velocity.z};
        //render_data.UpdateAnim(Vector2Length(hor_vel) > 20, delta_time);
        body.Update(delta_time);
    }

#ifdef WITH_RENDER
    void Draw(GameDrawingData& drawing_data) const {
        Vector2 hor_vel = {body.velocity.x, body.velocity.z};
        bool running = Vector2Length(hor_vel) > 20;
        render_data.Draw(running, body, yaw, pitch);        
        if (render_data.model_key == R_MODEL_DEFAULT) body.DrawShapes();
    }
#endif

    Vector3 VForward() const {
        return Vector3{cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)};
    }

    Vector3 VRight() const {
        return Vector3{cos(yaw+PI/2) * cos(pitch), 0, sin(yaw+PI/2) * cos(pitch)};
    }

    template <class Archive>
    void serialize(Archive& ar) {
        ar(body, yaw, pitch, render_data);
    }
};