#version 330 core

in vec2 fragPos;
out vec4 fragColor;

uniform vec2 circleCenter;
uniform float radius;

uniform bool colorSwap;

uniform bool drawLine;

void main() {

    // ---- VONAL ----
    if (drawLine) {
        fragColor = vec4(0.0,0.0,1.0, 1.0);
        return;
    }

    // ---- KÖR ----
    float dist = distance(fragPos, circleCenter);

    if (dist > radius) {
        discard;
    }

    float nd = dist / radius;

    vec3 centerColor = vec3(1.0, 0.0, 0.0);
    vec3 borderColor = vec3(0.0, 1.0, 0.0);

    if (colorSwap) {
        centerColor = vec3(0.0, 1.0, 0.0);
        borderColor = vec3(1.0, 0.0, 0.0);
    }

    fragColor = vec4(mix(centerColor, borderColor, nd), 1.0);
}