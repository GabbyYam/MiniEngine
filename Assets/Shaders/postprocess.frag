#version 410 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;

uniform int tonemappingType = 0;
uniform float exposure;

uniform int enableBloom = 1;
uniform float bloomIntensity;

uniform int enableFXAA;

#define FXAA_SPAN_MAX 8.0
#define FXAA_REDUCE_MUL   (1.0 / FXAA_SPAN_MAX)
#define FXAA_REDUCE_MIN   (1.0 / 128.0)
#define FXAA_SUBPIX_SHIFT (1.0 / 4.0)

vec3 Fxaa( vec4 uv, sampler2D tex, vec2 rcpFrame) {
    
    vec3 rgbNW = texture(tex, uv.zw).xyz;
    vec3 rgbNE = texture(tex, uv.zw + vec2(1,0) * rcpFrame.xy).xyz;
    vec3 rgbSW = texture(tex, uv.zw + vec2(0,1) * rcpFrame.xy).xyz;
    vec3 rgbSE = texture(tex, uv.zw + vec2(1,1) * rcpFrame.xy).xyz;
    vec3 rgbM  = texture(tex, uv.xy).xyz;

    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
    
    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) * rcpFrame.xy;

    vec3 rgbA = (1.0/2.0) * (
        texture(tex, uv.xy + dir * (1.0/3.0 - 0.5)).xyz +
        texture(tex, uv.xy + dir * (2.0/3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        texture(tex, uv.xy + dir * (0.0/3.0 - 0.5)).xyz +
        texture(tex, uv.xy + dir * (3.0/3.0 - 0.5)).xyz);
    
    float lumaB = dot(rgbB, luma);

    if((lumaB < lumaMin) || (lumaB > lumaMax)) return rgbA;
    
    return rgbB; 
}



vec3 ACES_ToneMapping(const vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d ) + e), 0.0, 1.0);
}


void main()
{
    // to bloom or not to bloom
    vec2 rcpFrame = 1. / textureSize(scene, 0).xy;
    vec4 uv = vec4(TexCoords, TexCoords - (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT)));

    vec3 origin = enableFXAA == 1 ? Fxaa(uv, scene, rcpFrame) : texture(scene, TexCoords).rgb;
    vec3 bloom = texture(bloomBlur, TexCoords).rgb;

    vec3 color =  enableBloom == 1 ? mix(origin, bloom, bloomIntensity) : origin;

    // tone mapping
    switch (tonemappingType) {
        case 0:
            color = vec3(1.0) - exp(-color * exposure);
            break;
        case 1:
            color = ACES_ToneMapping(color);
            break;
        case 2:
            break;
    }

    // // also gamma correct while we're at it
    const float gamma = 2.2;
    color = pow(color, vec3(1.0 / gamma));

    FragColor = vec4(color, 1.0);

}