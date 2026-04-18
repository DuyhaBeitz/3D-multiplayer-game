void fragment() {
    vec2 uv = TEXCOORD;

    uv = uv * 2.0 - 1.0;

    float fishyness = 0.07;

    vec2 fishuv;
    fishuv.x = (1.0 - uv.y*uv.y) * fishyness * uv.x;
    fishuv.y = (1.0 - uv.x*uv.x) * fishyness * uv.y;

    uv = (uv+1.0) * 0.5;

    vec3 texA = SampleColor(uv - fishuv * 0.95);
    vec3 texB = SampleColor(uv - fishuv);

    float cr = texA.r;
    vec2 cgb = texB.gb;

    vec3 c = vec3(cr, cgb);

    float uvMagSqrd = dot(uv, uv);
    float vignette = 1.0 - uvMagSqrd * fishyness * 2.2;

    c *= vignette;

    COLOR = vec3(c);
}