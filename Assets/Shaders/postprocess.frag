#version 410 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform vec3 samples[64];
uniform mat4 projection;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform sampler2D DepthMap;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D ssaoNoise;

uniform int   tonemappingType = 0;
uniform float exposure;

uniform int   enableBloom = 1;
uniform float bloomIntensity;

uniform int enableFXAA;
uniform int enableDoF;
uniform int enableSSAO;

uniform vec3  viewPos;
uniform int   fogType;
uniform float fogDensity;
uniform float fogStart;
uniform float fogEnd;

uniform float nearClip;
uniform float farClip;

#define FXAA_SPAN_MAX 8.0
#define FXAA_REDUCE_MUL (1.0 / FXAA_SPAN_MAX)
#define FXAA_REDUCE_MIN (1.0 / 128.0)
#define FXAA_SUBPIX_SHIFT (1.0 / 4.0)

vec3 Fxaa(vec4 uv, sampler2D tex, vec2 rcpFrame)
{
    vec3 rgbNW = texture(tex, uv.zw).xyz;
    vec3 rgbNE = texture(tex, uv.zw + vec2(1, 0) * rcpFrame.xy).xyz;
    vec3 rgbSW = texture(tex, uv.zw + vec2(0, 1) * rcpFrame.xy).xyz;
    vec3 rgbSE = texture(tex, uv.zw + vec2(1, 1) * rcpFrame.xy).xyz;
    vec3 rgbM  = texture(tex, uv.xy).xyz;

    vec3  luma   = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM, luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * rcpFrame.xy;

    vec3 rgbA = (1.0 / 2.0) * (texture(tex, uv.xy + dir * (1.0 / 3.0 - 0.5)).xyz + texture(tex, uv.xy + dir * (2.0 / 3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * (1.0 / 2.0) +
                (1.0 / 4.0) * (texture(tex, uv.xy + dir * (0.0 / 3.0 - 0.5)).xyz + texture(tex, uv.xy + dir * (3.0 / 3.0 - 0.5)).xyz);

    float lumaB = dot(rgbB, luma);

    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        return rgbA;

    return rgbB;
}

vec3 ACES_ToneMapping(const vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;  // Back to NDC
    return (2.0 * nearClip * farClip) / (farClip + nearClip - z * (farClip - nearClip));
}

vec3 GuassianBlur(sampler2D txr, vec2 uv)
{
    float Pi = 6.28318530718;  // Pi*2

    // GAUSSIAN BLUR SETTINGS {{{
    float Directions = 16.0;  // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
    float Quality    = 12.0;  // BLUR QUALITY (Default 4.0 - More is better but slower)
    float Size       = 8.0;   // BLUR SIZE (Radius)
    // GAUSSIAN BLUR SETTINGS }}}

    vec2 resolution = textureSize(txr, 0);
    vec2 Radius     = Size / resolution;

    // Pixel colour
    vec4 Color = texture(txr, uv);

    // Blur calculations
    for (float d = 0.0; d < Pi; d += Pi / Directions) {
        for (float i = 1.0 / Quality; i <= 1.0; i += 1.0 / Quality) {
            Color += texture(txr, uv + vec2(cos(d), sin(d)) * Radius * i);
        }
    }

    // Output to screen
    Color /= Quality * Directions - 15.0;
    return Color.rgb;
}

float SSAO(vec2 coord)
{
    // parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
    int   kernelSize = 64;
    float radius     = 10;
    float bias       = 0.5;

    // tile noise texture over screen based on screen dimensions divided by noise size
    vec2 noiseScale = textureSize(scene, 0) * 0.25;

    // Get input for SSAO algorithm
    vec3 fragPos   = texture(gPosition, TexCoords).xyz;
    vec3 normal    = texture(gNormal, TexCoords).rgb;
    vec3 randomVec = texture(ssaoNoise, TexCoords * noiseScale).xyz;
    // Create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);
    // Iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i) {
        // get sample position
        vec3 samplePos = TBN * samples[i];  // From tangent to view-space
        samplePos      = fragPos + samplePos * radius;

        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset      = projection * offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5;  // transform to range 0.0 - 1.0

        // get sample depth
        float sampleDepth = -texture(gPosition, offset.xy).w;  // Get depth value of kernel sample
        // float sampleDepth = LinearizeDepth(-texture(DepthMap, offset.xy).r);
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    return occlusion;
}

void main()
{
    // float depth = texture(gPosition, TexCoords).w;
    float depth = LinearizeDepth(texture(DepthMap, TexCoords).r);
    float ao    = SSAO(TexCoords);

    highp vec2 rcpFrame = 1.0 / (textureSize(scene, 0).xy);
    vec4       uv       = vec4(TexCoords, TexCoords - (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT)));

    vec3 guassian = GuassianBlur(scene, TexCoords);

    // to bloom or not to bloom
    vec3 origin = enableFXAA == 1 ? Fxaa(uv, scene, rcpFrame) : texture(scene, TexCoords).rgb;
    vec3 bloom  = texture(bloomBlur, TexCoords).rgb;

    vec3 color = enableBloom == 1 ? mix(origin, bloom, bloomIntensity) : origin;

    if (enableDoF != 0)
        color = mix(color, guassian, depth / farClip);

    if (enableSSAO != 0)
        color *= ao;

    {
        float fogFactor = 1.0;
        float dist      = depth / farClip;
        switch (fogType) {
            case 0: break;
            case 1: fogFactor = (1.0 - fogDensity * dist); break;
            case 2: fogFactor = exp(-(fogDensity * dist)); break;
            case 3: fogFactor = exp(-pow(fogDensity * dist, 2)); break;
        }
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        vec3 fogColor = vec3(0.5, 0.5, 0.5);
        color         = mix(fogColor, color, fogFactor);
    }

    // tone mapping
    switch (tonemappingType) {
        case 0: break;
        case 1: color = vec3(1.0) - exp(-color * exposure); break;
        case 2: color = ACES_ToneMapping(color); break;
    }

    // // also gamma correct while we're at it
    const float gamma = 2.2;
    color             = pow(color, vec3(1.0 / gamma));

    FragColor = vec4(color, 1.0);

    // Visualize depth
    // FragColor = vec4(vec3(1.0 - (depth / farClip)), 1.0);
    // FragColor = texture(gNormal, TexCoords);
    // FragColor = vec4(vec3(ao), 1.0);
    // FragColor = vec4(vec3(depth / farClip), 1.0);
    // FragColor = texture(gPosition, TexCoords);
    // FragColor = texture(ssaoNoise, TexCoords);
}