#version 330 core

// Input vertex attributes
layout(location = 0) in vec3 vertexPosition; // Vertex position
layout(location = 1) in vec2 vertexUV;       // Vertex UV coordinates
layout(location = 2) in vec3 vertexNormal;   // Vertex normals

// Output to fragment shader
out vec2 UV;        // Pass UV coordinates to fragment shader
out vec3 FragPos;   // Pass fragment position in world space
out vec3 Normal;    // Pass normal vector to fragment shader

// Uniforms
uniform mat4 MVP;        // Combined Model-View-Projection matrix
uniform mat4 Model;      // Model matrix (for transforming normals)

void main()
{
    // Transform the vertex position to world space
    vec4 worldPosition = Model * vec4(vertexPosition, 1.0);
    FragPos = worldPosition.xyz;

    // Pass UV coordinates directly
    UV = vertexUV;

    // Transform the normal vector to world space and normalize
    Normal = mat3(transpose(inverse(Model))) * vertexNormal;

    // Calculate final vertex position
    gl_Position = MVP * vec4(vertexPosition, 1.0);
}


