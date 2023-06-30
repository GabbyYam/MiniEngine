#version 410 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in vec2 TexCoord;
in vec3 normalWS;
in vec4 shadowCoord;
in vec3 fragPos;


uniform sampler2D DiffuseMap1;
uniform sampler2D DepthMap;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float lightIntensity;

uniform float bloomThreshold; 


uniform vec3 viewPos;

void main()
{   
    float glossness = 32;
    vec4 albedo = texture(DiffuseMap1, TexCoord);

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
    float diffuse = max(0.15, step(NoL, 0.3));

    // Specular
    float specular = pow(max(NoH, 0.0), 32);
    specular = smoothstep(0.0, 0.05, specular);

    vec4 specularColor = vec4(0.8, 0.8, 0.8, 1.0);
    
    float rim = 1.0 - VoN;
    float rimAmount = 0.716;
    float rimThreshold = 0.1;
    float rimIntensity = rim;
    float rimRange = 0.01;
    float rimAttenuation = 0.6;
    rimIntensity = smoothstep(rimAmount - rimRange, rimAmount + rimRange, rimIntensity);
    vec3 rimColor = albedo.rgb * rimIntensity * (lightColor * rimAttenuation);

    // Merge result
    vec3 color = diffuse * albedo.rgb + rimColor;

    // output color && gamma correction
    // color = color / (color + vec3(1.0));
    // color = vec3(1.0) - exp(-color * exposure);
    // color = pow(color, vec3(1.0 / 2.0)); 
    
    FragColor = vec4(color, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));

    // if(brightness > bloomThreshold) BrightColor = vec4(FragColor.rgb, 1.0);
    BrightColor = vec4(vec3(0.0), 1.0);
}