#version 330 core

layout(location = 0) in vec2 position;

uniform vec2 circleCenter;
uniform float radius;
uniform bool drawLine;
uniform float lineY;

out vec2 fragPos;   

void main() {
    vec2 transformedPosition;

    if (drawLine) {
        transformedPosition.x = position.x;
        transformedPosition.y = position.y + lineY;
    } else {
        transformedPosition = position * radius + circleCenter;
    }

    fragPos = transformedPosition;

    gl_Position = vec4(transformedPosition, 0.0, 1.0);
}