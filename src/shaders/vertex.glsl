#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
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

void main()
{

    vec4 totalPosition = vec4(0.0f);
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
    }

    /*
    mat4 boneTransform =  gBones[BoneIDs[0]] * Weights[0];
         boneTransform += gBones[BoneIDs[1]] * Weights[1];
         boneTransform += gBones[BoneIDs[2]] * Weights[2];
         boneTransform += gBones[BoneIDs[3]] * Weights[3];

    vec4 finalPos = boneTransform * vec4(aPos, 1.0f);
    */

    Normal = mat3(transpose(inverse(world))) * aNormal;
    FragPos = vec3(world * vec4(aPos, 1.0f));
    TexCoord = aTexCoord;
    gl_Position = proj * view * world * totalPosition;
}
