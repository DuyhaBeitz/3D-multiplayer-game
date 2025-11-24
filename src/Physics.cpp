#include "Physics.hpp"
#include <assert.h>

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
    if (dot > 0.f) return;

    float e = fmin(bA.restitution, bB.restitution);

    float j = -(1.f + e) * dot;
    j /= m2 + m1;
    
    Vector3 impulse = normal*j;
    
    bA.ApplyImulse(impulse);
    bB.ApplyImulse(impulse * -1);   
}



void HeightmapData::Load(Image heightmap_image, Vector3 center, Vector3 scale) {
    m_position = center-scale/2;
    m_position.y = center.y;

    m_scale = scale;

    m_samples = std::min(heightmap_image.width, heightmap_image.height);
    m_heights.resize(m_samples * m_samples);

    Color* pixels = LoadImageColors(heightmap_image);

    int imgW = heightmap_image.width;

    for (int z = 0; z < m_samples; z++) {
        for (int x = 0; x < m_samples; x++) {
            int imgIndex = z * imgW + x;
            m_heights[z * m_samples + x] = pixels[imgIndex].r / 255.f;
        }
    }

    UnloadImageColors(pixels);
}

float HeightmapData::GetHeightAtGrid(float ix, float iz) const {
    int idx = iz * m_samples + ix;

    return m_heights[idx] * m_scale.y;
}

float HeightmapData::GetHeightAt(float x, float z) const {
    Vector3 min = Min();
    Vector3 max = Max();

    if (x < min.x || z < min.z || x > max.x || z > max.z)
        return 0.0f;

    float u = (x - m_position.x) / m_scale.x;
    float v = (z - m_position.z) / m_scale.z;

    float fx = u * (m_samples - 1);
    float fz = v * (m_samples - 1);

    int x0 = (int)floorf(fx);
    int z0 = (int)floorf(fz);
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    // Clamp to bounds
    x0 = Clamp(x0, 0, m_samples - 1);
    x1 = Clamp(x1, 0, m_samples - 1);
    z0 = Clamp(z0, 0, m_samples - 1);
    z1 = Clamp(z1, 0, m_samples - 1);

    // Interpolation weights
    float tx = fx - x0;
    float tz = fz - z0;

    // Sample neighbors using your old function
    float h00 = GetHeightAtGrid(x0, z0);
    float h10 = GetHeightAtGrid(x1, z0);
    float h01 = GetHeightAtGrid(x0, z1);
    float h11 = GetHeightAtGrid(x1, z1);

    // Bilinear interpolation
    float hx0 = Lerp(h00, h10, tx);
    float hx1 = Lerp(h01, h11, tx);
    return Lerp(hx0, hx1, tz);
}

Vector3 HeightmapData::GetNormalAt(float x, float z) const {

    // heights around the point

    float eps = 0.1;

    Vector3 p0;
    p0.x = x + eps;
    p0.z = z + eps;
    p0.y = GetHeightAt(p0.x, p0.z);

    Vector3 p1;
    p1.x = x + eps;
    p1.z = z - eps;
    p1.y = GetHeightAt(p1.x, p1.z);

    Vector3 p2;
    p2.x = x - eps;
    p2.z = z - eps;
    p2.y = GetHeightAt(p2.x, p2.z);

    Vector3 p3;
    p3.x = x - eps;
    p3.z = z + eps;
    p3.y = GetHeightAt(p3.x, p3.z);

    Vector3 v0 = p0-p2;
    Vector3 v1 = p1-p3;
    
    Vector3 normal = Vector3CrossProduct(v0, v1);

    return Vector3Normalize(normal);
}

void HeightmapData::Draw() const {
    DrawCubeWiresV(m_position+m_scale/2, m_scale, GOLD);
    
    for (int z = 0; z < m_samples; z++) {
        for (int x = 0; x < m_samples; x++) {
            Vector3 p;
            p.x = m_position.x + m_scale.x * x / (m_samples - 1);
            p.z = m_position.z + m_scale.z * z / (m_samples - 1);

            p.y = GetHeightAt(p.x, p.z);

            DrawSphereEx(p, 0.5, 2, 2, PURPLE);
        }
    }
}

void HeightmapData::SolveCollisionWith(BodyData &other) const{

    if (other.inverse_mass > 0) {
        float height = GetHeightAt(other.position.x, other.position.z);
        float min = other.Min().y;
        
        float penetration = height-min;
        if (penetration > 0) {
            const Vector3 normal = GetNormalAt(other.position.x, other.position.z);//Vector3{0, 1, 0};

            const float m2 = other.inverse_mass;

            other.position += normal*penetration;

            Vector3 relative_velocity = other.velocity;
            float dot = Vector3DotProduct(relative_velocity, normal);
            if (dot > 0.f) return;

            float e = other.restitution;

            float j = -(1.f + e) * dot;
            j /= m2;
            
            Vector3 impulse = normal*j;
        
            other.ApplyImulse(impulse);  
        }
    }
}