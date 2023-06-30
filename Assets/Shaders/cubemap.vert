#version 410 core
layout(location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 proj;
uniform mat4 view;

void main()
{
    TexCoords = aPos;

    mat4 rotView = mat4(mat3(view));
    vec4 pos     = proj * rotView * vec4(aPos, 1.0);
    gl_Position  = pos.xyww;
}