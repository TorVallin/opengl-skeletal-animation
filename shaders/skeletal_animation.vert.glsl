#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in ivec4 boneIds;
layout (location = 3) in vec4 boneWeights;

const int MAX_BONES = 128;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 skinning_matrices[MAX_BONES];
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoords;

void main() {
    vec4 totalPosition = vec4(0.0f);

    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if (boneIds[i] == -1) {
            continue;
        }
        if (boneIds[i] >= MAX_BONES) {
            totalPosition = vec4(pos, 1.0);
            break;
        }
        vec4 localPosition = skinning_matrices[boneIds[i]] * vec4(pos, 1.0);
        totalPosition += localPosition * boneWeights[i];
    }

    gl_Position = projection * view * model * totalPosition;
    TexCoords = tex;
}


