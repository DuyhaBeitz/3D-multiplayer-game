#include "Physics.hpp"

CollisionResult CollideSphereSphere(const SphereData &a, const SphereData &b) {
    Vector3 diff = Vector3Subtract(a.center, b.center);
    float dist = Vector3Length(diff);
    float rsum = a.radius + b.radius;

    CollisionResult res;
    res.penetration = rsum - dist;
    res.normal = Vector3Normalize(diff);

    return res;
}

CollisionResult CollideSphereBox(const SphereData &s, const BoxData &b) {
    Vector3 closest = {
        Clamp(s.center.x, b.Min().x, b.Max().x),
        Clamp(s.center.y, b.Min().y, b.Max().y),
        Clamp(s.center.z, b.Min().z, b.Max().z)
    };
    
    Vector3 diff = Vector3Subtract(s.center, closest);
    float dist = Vector3Length(diff);

    CollisionResult res;
    res.penetration = s.radius - dist;
    res.normal = Vector3Normalize(diff);

    return res;
}

CollisionResult CollideBoxBox(const BoxData &a, const BoxData &b) {
    CollisionResult res;
    Vector3 overlap = {
        fminf(a.Max().x, b.Max().x) - fmaxf(a.Min().x, b.Min().x),
        fminf(a.Max().y, b.Max().y) - fmaxf(a.Min().y, b.Min().y),
        fminf(a.Max().z, b.Max().z) - fmaxf(a.Min().z, b.Min().z)
    };
    if (overlap.x > 0 && overlap.y > 0 && overlap.z > 0) {
        // Pick smallest axis
        if (overlap.x < overlap.y && overlap.x < overlap.z) {
            res.normal = { ((a.Max().x > b.Max().x) ? 1.f : -1.f), 0, 0 };
            res.penetration = overlap.x;
        }
        else if (overlap.y < overlap.z) {
            res.normal = { 0, ((a.Max().y > b.Max().y) ? 1.f : -1.f), 0 };
            res.penetration = overlap.y;
        }            
        else {
            res.normal = { 0, 0, ((a.Max().z > b.Max().z) ? 1.f : -1.f) };
            res.penetration = overlap.z;
        }            
    }
    return res;
}