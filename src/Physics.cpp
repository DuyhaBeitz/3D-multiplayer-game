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

void SolveCollision(BodyData &bA, BodyData &bB, const CollisionResult &collision_result) {
    const Vector3& normal = collision_result.normal;

    const float m1 = bA.inverse_mass;
    const float m2 = bB.inverse_mass;

    bA.position += normal*collision_result.penetration * (m1)/(m1+m2);
    bB.position -= normal*collision_result.penetration * (m2)/(m1+m2);

    Vector3 relative_velocity = bA.velocity - bB.velocity;
    float dot = Vector3DotProduct(relative_velocity, normal);
    if (dot > 0) return;

    float e = fmin(bA.restitution, bB.restitution);

    float j = -(1.f + e) * dot;
    j /= m2 + m1;
    
    Vector3 impulse = normal*j;
    
    bA.ApplyImulse(impulse);
    bB.ApplyImulse(impulse * -1);
}