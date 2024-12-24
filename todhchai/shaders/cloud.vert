#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

// Uniforms
uniform mat4 MVP;        // Model-View-Projection for final vertex position
uniform mat4 Model;      // Model matrix for normal transformation (or ModelView if you prefer)
                        
// Outputs to the fragment shader
out vec2 fragUV;
out vec3 fragNormal;
out vec3 fragWorldPos;

void main()
{
    // Calculate final vertex position in clip space
    gl_Position = MVP * vec4(inPosition, 1.0);
    
    // Pass texture UV to fragment shader
    fragUV = inUV;

    // Compute the world-space position of the vertex (if needed for lighting)
    fragWorldPos = vec3(Model * vec4(inPosition, 1.0));

    // Correctly transform the normal into world space
    // Using transpose(inverse(Model)) is typical for normal transforms
    // (or transpose(inverse(ModelView)) if you do everything in view space)
    mat3 normalMatrix = mat3(transpose(inverse(Model)));
    fragNormal = normalize(normalMatrix * inNormal);
}
