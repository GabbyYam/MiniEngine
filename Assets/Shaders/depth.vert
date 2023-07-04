#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 viewLS;
uniform mat4 projLS;

out vec3 fragPos;
out vec3 normalWS;

void main()
{
    gl_Position = projLS * viewLS * model * vec4(aPos, 1.0);

    fragPos  = (viewLS * model * vec4(aPos, 1.0)).xyz;
    normalWS = transpose(inverse(mat3(viewLS * model))) * aNormal;
}