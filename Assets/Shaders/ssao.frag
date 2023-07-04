#version 410
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 samples[64];
uniform mat4 projection;

uniform sampler2D mainImage;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D ssaoNoise;

float SSAO(vec2 coord)
{
    // parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
    int   kernelSize = 64;
    float radius     = 10;
    float bias       = 0.5;

    // tile noise texture over screen based on screen dimensions divided by noise size
    vec2 noiseScale = textureSize(mainImage, 0) * 0.25;

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
    float ao  = SSAO(TexCoords);
    FragColor = vec4(vec3(ao), 1.0);
}