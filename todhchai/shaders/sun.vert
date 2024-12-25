#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 MVP; // Changed from separate matrices to MVP

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0);
}
