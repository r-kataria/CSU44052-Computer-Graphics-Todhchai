// cloud.vert

#version 330 core

layout(location = 0) in vec3 vertexPosition; // Position attribute
layout(location = 1) in vec2 vertexUV;       // UV attribute
layout(location = 2) in vec3 vertexNormal;   // Normal attribute

uniform mat4 MVP;    // Model-View-Projection matrix
uniform mat4 Model;  // Model matrix

out vec2 fragUV;
out vec3 fragNormal;
out vec3 fragWorldPos;

void main()
{
    // Pass the UV coordinates to the fragment shader
    fragUV = vertexUV;

    // Transform the normal to world space
    fragNormal = mat3(transpose(inverse(Model))) * vertexNormal;

    // Calculate and pass the world position of the fragment
    fragWorldPos = vec3(Model * vec4(vertexPosition, 1.0));

    // Final position
    gl_Position = MVP * vec4(vertexPosition, 1.0);
}
