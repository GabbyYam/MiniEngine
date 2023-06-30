#version 410 core
out vec4 fragColor;

in vec2 TexCoord;
in vec3 normalWS;
in vec4 shadowCoord;

uniform sampler2D DiffuseMap1;
uniform sampler2D DepthMap;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float lightIntensity;

vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

float ShadowMapping(vec4 fragPosLightSpace)
{
    float bias = max(0.05 * ( dot(normalWS, lightDirection)), 0.005);    // 执行透视除法
    
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 变换到[0,1]的范围
    projCoords = projCoords * 0.5 + 0.5;
    // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = texture(DepthMap, projCoords.xy).r; 
    // 取得当前片段在光源视角下的深度
    float currentDepth = projCoords.z;
    // 检查当前片段是否在阴影中
    float shadow = currentDepth > closestDepth + bias ? 1.0 : 0.0;
    if (projCoords.z > 1.0) return 0;
    return shadow;
}

float PCF(vec4 fragPosLightSpace) {
    float bias = max(0.05 * ( -dot(normalWS, lightDirection)), 0.005);    // 执行透视除法
    
    vec3 coord = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 变换到[0,1]的范围
    coord = coord * 0.5 + 0.5;
    // if (coord.z > 1.0) return 0.0;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(DepthMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(DepthMap, coord.xy + vec2(x, y) * texelSize).r; 
            shadow += coord.z - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    
    shadow /= 9.0;
    return shadow;
}

void main()
{    
    float visibility = 1.0 - PCF(shadowCoord);
    // visibility = 1.0;

    // output color && gamma correction
    float gamma = 2.2; /* which is 1.0 / 2.2 */
    
    fragColor = vec4(vec3(max(0.15, visibility)), 1.0);
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / gamma));
    // fragColor = vec4(vec3(1.0), 1.0);
}