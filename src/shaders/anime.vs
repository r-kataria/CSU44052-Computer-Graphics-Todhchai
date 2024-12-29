#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;
layout(location = 5) in ivec4 aBoneIDs;
layout(location = 6) in vec4 aWeights;

// Match the structure that bloom.fs expects
out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

// Uniforms for camera & transformations
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Uniforms for bone transformations
const int MAX_BONES = 100;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    // (1) Apply bone transforms to the vertex position
    vec4 totalPosition = vec4(0.0);
    for(int i = 0; i < 4; ++i)
    {
        if(aBoneIDs[i] == -1)
            continue;
        // Multiply by each bone’s transformation & accumulate
        vec4 weightedPos = finalBonesMatrices[aBoneIDs[i]] * vec4(aPos, 1.0);
        totalPosition += weightedPos * aWeights[i];
    }
    // Fallback if no valid bones
    if(totalPosition == vec4(0.0))
        totalPosition = vec4(aPos, 1.0);

    // (2) Basic normal approach (not applying bone transforms to the normal, 
    //     just the model matrix as in bloom.vs).
    vec3 normal = aNormal;

    // (3) Compute world-space position
    vec4 worldPosition = model * totalPosition;
    vs_out.FragPos = vec3(worldPosition);

    // (4) Normal: same normalMatrix approach as bloom.vs
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalize(normalMatrix * normal);

    // (5) Pass texture coordinates
    vs_out.TexCoords = aTexCoords;

    // (6) Final clip-space position
    gl_Position = projection * view * worldPosition;
}
