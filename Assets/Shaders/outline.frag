#version 410

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;
layout(location = 2) out int EntityID;

uniform int entityID;

void main() {
    FragColor = vec4(1.0, 0.3, 0.0, 1.0);
    EntityID = entityID;
}