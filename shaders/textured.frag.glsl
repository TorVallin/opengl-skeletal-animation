#version 450 core

uniform sampler2D InputTexture;

out vec4 FragColor;
in vec2 TexCoords;

void main() {
    vec3 sampled = texture(InputTexture, TexCoords).rgb;
    FragColor = vec4(sampled.xyz, 1.0);
}
