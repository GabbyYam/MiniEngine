#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 normalWS;
out vec4 shadowCoord;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 mvpLS;

void main()
{
    gl_Position = proj * view * model * vec4(aPos, 1.0);
    TexCoord    = aTexCoord;
    normalWS    = transpose(inverse(mat3(model))) * aNormal;

    fragPos = (model * vec4(aPos, 1.0)).xyz;

    // normalWS    = aNormal;
    shadowCoord = mvpLS * model * vec4(aPos, 1.0);
}