#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;

out vec4 FragPos; // Pass the world-space position to the geometry shader

void main()
{
    FragPos = model * vec4(aPos, 1.0);
    gl_Position = FragPos; // Not important, will be overwritten in geometry shader
}