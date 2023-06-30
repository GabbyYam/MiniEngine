#version 410 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D albedo;

float map(vec3 p) {
    float d = distance(p, vec3(-1, 0, -5)) - 1.;     // sphere at (-1,0,5) with radius 1
    d = min(d, distance(p, vec3(2, 0, -3)) - 1.);    // second sphere
    d = min(d, distance(p, vec3(-2, 0, -2)) - 1.);   // and another
    d = min(d, p.y + 1.);                            // horizontal plane at y = -1
    return d;
}

//
// Calculate the normal by taking the central differences on the distance field.
//
vec3 calcNormal(in vec3 p) {
    vec2 e = vec2(1.0, -1.0) * 0.0005;
    return normalize(
        e.xyy * map(p + e.xyy) +
        e.yyx * map(p + e.yyx) +
        e.yxy * map(p + e.yxy) +
        e.xxx * map(p + e.xxx));
}

vec3 ACES_ToneMapping(const vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d ) + e), 0.0, 1.0);
}

vec4 mainImage(vec2 coord) {
    vec4 fragColor = vec4(vec3(0.0), 1.0);
    vec3 ro = vec3(0, 0, 1);                           // ray origin

    ivec2 iResolution = textureSize(albedo, 0);
    vec2 q = (iResolution * coord.xy - .5 * iResolution.xy ) / iResolution.y;
    vec3 rd = normalize(vec3(q, 0.) - ro);             // ray direction for fragCoord.xy

    // March the distance field until a surface is hit.
    float h, t = 1.;
    for (int i = 0; i < 256; i++) {
        h = map(ro + rd * t);
        t += h;
        if (h < 0.01) break;
    }

    if (h < 0.01) {
        vec3 p = ro + rd * t;
        vec3 N = calcNormal(p);
        vec3 light = vec3(1, 1, 1);
        vec3 L = normalize(light);
        
        // Calculate diffuse lighting by taking the dot product of 
        // the light direction (light-p) and the normal.
        float diffuse = clamp(dot(N, L), 0., 1.);
		
        // Multiply by light intensity (5) and divide by the square
        // of the distance to the light.
        diffuse *= 5. / dot(light - p, light - p);
        
        
        fragColor = vec4(vec3(diffuse), 1);     // Gamma correction
    } else {
        fragColor = vec4(0, 0, 0, 1) * 0.15;
    }
    
    fragColor.rgb = ACES_ToneMapping(fragColor.rgb);
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2));
    return fragColor;
}

void main()
{       
    FragColor = mainImage(TexCoords);
}
