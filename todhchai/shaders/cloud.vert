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
    fragUV = vertexUV;
    fragNormal = mat3(transpose(inverse(Model))) * vertexNormal;
    fragWorldPos = vec3(Model * vec4(vertexPosition, 1.0));
    gl_Position = MVP * vec4(vertexPosition, 1.0);
}
