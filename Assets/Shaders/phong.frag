#version 410 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec2 TexCoord;
in vec3 normalWS;
in vec3 fragPos;
in vec4 shadowCoord;

uniform sampler2D DiffuseMap;
uniform sampler2D DepthMap;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float lightIntensity;

uniform vec3 viewPos;

uniform float bloomThreshold; 


#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586

// Shadow map related variables
#define NUM_SAMPLES 20
#define BLOCKER_SEARCH_NUM_SAMPLES NUM_SAMPLES
#define PCF_NUM_SAMPLES NUM_SAMPLES
#define NUM_RINGS 10

// magic number
#define VISIBLE_VALUE 1.0
#define INVISIBLE_VALUE 0.6
#define BIAS 0.005
#define RESOLUTION 2048.0
#define SEARCH_RADIUS 1024.0
#define LIGHT_WIDTH 1.0
#define USE_POISSON 1


vec2 poissonDisk[NUM_SAMPLES];

float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    return dot(rgbaDepth, bitShift);
}

highp float rand_2to1(vec2 uv) { 
  // 0 - 1
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}

void poissonDiskSamples( const in vec2 randomSeed ) {

    float ANGLE_STEP = PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
    float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

    float angle = rand_2to1( randomSeed ) * PI2;
    float radius = INV_NUM_SAMPLES;
    float radiusStep = radius;

    for( int i = 0; i < NUM_SAMPLES; i ++ ) {
        poissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
        radius += radiusStep;
        angle += ANGLE_STEP;
    }
}

float ShadowMapping(vec4 fragPosLightSpace)
{
    float bias = max(0.05 * ( dot(normalWS, lightDirection)), 0.005);
    
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

float PCF(vec4 coords, float filterRadius) {
    #ifdef USE_POISSON
        poissonDiskSamples(coords.xy);
    #else
        uniformDiskSamples(coords.xy);
    #endif
    float result = 0.0;
    for(int i = 0; i < PCF_NUM_SAMPLES; ++i) {
        float depth = unpack(texture(DepthMap, coords.xy + poissonDisk[i] * filterRadius / RESOLUTION)) + BIAS;
        result += depth > coords.z ? VISIBLE_VALUE : INVISIBLE_VALUE;
    }
    return result / float(PCF_NUM_SAMPLES); 
}

float findBlocker(vec2 uv, float zReceiver ) {
    #ifdef USE_POISSON
        poissonDiskSamples(uv);
    #else
        uniformDiskSamples(uv);
    #endif

    float bias = max(0.05 * ( dot(normalWS, lightDirection)), 0.005);

    float blockerSum = 0.0;
    int blockerCnt = 0;
    for (int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i) {
    float depth = unpack(texture(DepthMap, uv + poissonDisk[i] / RESOLUTION)) + bias;
        if (depth < zReceiver) {
            ++blockerCnt;
            blockerSum += depth;
        }
    }
    if (blockerCnt == 0) return 2.0;
    if (blockerCnt == BLOCKER_SEARCH_NUM_SAMPLES) return -1.0;
    return blockerSum / float(blockerCnt);
}

float PCSS(vec4 coords) {
    // STEP 1: avgblocker depth
    float zBlocker = findBlocker(coords.xy, coords.z);
    // 没有遮挡
    if (abs(zBlocker - 2.0) < EPS) return VISIBLE_VALUE;
    // 完全遮挡
    if (abs(zBlocker - -1.0) < EPS) return INVISIBLE_VALUE;

    // STEP 2: penumbra size
    float penumbra = (coords.z - zBlocker) * LIGHT_WIDTH / zBlocker;
    // STEP 3: filtering
    return PCF(coords, penumbra);
}


float CalculateShadow() {
    vec3 coords = shadowCoord.xyz / shadowCoord.w;
    coords = coords * 0.5 + 0.5;

    // return PCSS(vec4(coords, 1.0));
    return PCF(vec4(coords, 1.0), 2);
}

void main()
{    
    vec3 albedo = texture(DiffuseMap, TexCoord).rgb;
    float glossness = 32;

    // For seperate color step
    vec3 N = normalize(normalWS);
    vec3 L = normalize(lightDirection);
    vec3 V = normalize(viewPos - fragPos);
    vec3 R = reflect(-L, N);
    vec3 H = normalize(V + L);

    float NoL = dot(N, L);
    float NoH = dot(N, H);
    float VoN = dot(V, N);

    // Ambient
    float ambient = 0.15;

    // Diffuse
    float diffuse = 0.5 * max(NoL, 0) + 0.5;

    // Specular
    float specular = pow(clamp(NoH, 0.0, 1.0), 32);
    // specular = smoothstep(0.005, 0.01, specular);

    // Merge result
    vec3 color = (ambient + diffuse + specular) * albedo;

    float visibility = CalculateShadow();
    color.rgb *= visibility;

    FragColor = vec4(color, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));

    if(brightness > bloomThreshold) BrightColor = vec4(FragColor.rgb, 1.0);
}