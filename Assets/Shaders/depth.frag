#version 410 core

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec3 gNormal;

in vec3 fragPos;
in vec3 normalWS;

uniform float farClip;
uniform float nearClip;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;  // 回到NDC
    return (2.0 * nearClip * farClip) / (farClip + nearClip - z * (farClip - nearClip));
}

void main()
{
    gNormal       = normalize(normalWS);
    gPosition.xyz = fragPos;
    gPosition.w   = LinearizeDepth(gl_FragCoord.z);
}
