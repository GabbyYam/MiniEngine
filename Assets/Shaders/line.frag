#version 410

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

void main() {
    FragColor = vec4(vec3(0.2), 1.0);
    BrightColor = vec4(0.0);
}