#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 BoneIDs;
layout (location = 6) in vec4 Weights;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 world;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 gBones[MAX_BONES];

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;
out mat3 TBN;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if(BoneIDs[i] == -1)
            continue;
        if(BoneIDs[i] >= MAX_BONES)
        {
            totalPosition = vec4(aPos, 1.0f);
        }
        vec4 localPosition = gBones[BoneIDs[i]] * vec4(aPos, 1.0f);
        totalPosition += localPosition * Weights[i];
        vec3 localNormal = mat3(gBones[BoneIDs[i]]) * aNormal;
        totalNormal += localNormal * Weights[i];
    }
/*
    gl_Position = proj * view * world * totalPosition;
    TexCoord = aTexCoord;
    Normal = mat3(transpose(inverse(world))) * totalNormal;
    FragPos = vec3(world * totalPosition);
*/
    gl_Position = proj * view * world * vec4(aPos, 1.0f);
    TexCoord = aTexCoord;
    Normal = mat3(transpose(inverse(world))) * aNormal;
    FragPos = vec3(world * vec4(aPos, 1.0f));
    vec3 T = normalize(vec3(world * vec4(aTangent, 0.0f)));
    vec3 B = normalize(vec3(world * vec4(aBitangent, 0.0f)));
    vec3 N = normalize(vec3(world * vec4(aNormal, 0.0f)));
    TBN = mat3(T, B, N);
}
