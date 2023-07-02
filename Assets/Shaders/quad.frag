#version 410 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D albedo;

void main()
{       
    vec4 color = texture(albedo, TexCoords);
    // color = color / (color + vec3(1.0));
    // color = pow(color, vec3(1.0 / 2.2));
    FragColor = color;
}
