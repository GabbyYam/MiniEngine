#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

out vec2 TexCoord;
out vec3 normalWS;
out vec4 shadowCoord;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 mvpLS;

const int    MAX_BONES          = 100;
const int    MAX_BONE_INFLUENCE = 4;
uniform mat4 boneTransform[MAX_BONES];

void main()
{
    vec4 bonePosition = vec4(0.0);
    // for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
    //     if (boneIds[i] < 0 || boneIds[i] >= MAX_BONES) {
    //         bonePosition = vec4(aPos, 1.0);
    //         break;
    //     }

    //     vec4 localPosition = boneTransform[boneIds[i]] * vec4(aPos, 1.0);
    //     bonePosition += localPosition * weights[i];
    //     vec3 localNormal = mat3(boneTransform[boneIds[i]]) * aNormal;
    // }
    gl_Position = proj * view * model * vec4(aPos, 1.0);

    TexCoord = aTexCoord;
    normalWS = transpose(inverse(mat3(model))) * aNormal;

    fragPos     = (model * vec4(aPos, 1.0)).xyz;
    shadowCoord = mvpLS * model * vec4(aPos, 1.0);
}