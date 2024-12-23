#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;   // <-- Moved UV to "1" for simplicity

out vec2 uv;  // pass to fragment

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(vertexPosition, 1.0);
    uv          = vertexUV;
}
