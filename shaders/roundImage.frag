#version 440
layout(location = 0) in vec2 coord;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float radius;
};

layout(binding = 1) uniform sampler2D src;

float roundedBox(in vec2 p, in vec2 b, in float r) {
    vec2 q = abs(p) - b + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

void main() {
    vec4 tex = texture(src, coord);
    vec2 center = coord * 2.0 - 1.0;

    float dist = roundedBox(center, vec2(1.0), radius);

    float edgeSoftness = fwidth(dist);
    float alpha = 1.0 - smoothstep(0.0, edgeSoftness, dist);

    fragColor = tex * qt_Opacity * alpha;
}
