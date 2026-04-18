// Simple vignette effect
// u_intensity: Controls vignette strength (0.0 = none, 1.0 = full)
// u_radius: Controls vignette size (0.0 = tight, 1.0 = wide)

float u_intensity = 0.5;
float u_radius = 0.5;

void fragment() {
    vec3 color = SampleColor(TEXCOORD);

    // Calculate distance from center
    vec2 center = TEXCOORD - vec2(0.5);
    //center.x *= ASPECT; // Aspect correction
    float dist = length(center);

    // Apply vignette
    float vignette = smoothstep(u_radius, u_radius * 0.5, dist);
    COLOR = color * mix(1.0, vignette, u_intensity);
}