#version 410 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec3 TexCoords;

uniform samplerCube EnvironmentMap;

void main()
{    
    vec3 color = texture(EnvironmentMap, TexCoords).rgb;
    FragColor = vec4(color, 1.0);

    BrightColor = vec4(color, 1.0);
}