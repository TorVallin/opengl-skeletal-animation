#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in ivec4 boneIds;

const int MAX_BONES = 128;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 finalBonesMatrices[MAX_BONES];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

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

        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos, 1.0);
        // TODO: use proper weights later, use uniform weights for now
        totalPosition += localPosition * 0.25;
    }

    gl_Position = projection * view * model * totalPosition;
}
