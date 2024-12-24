#version 330 core

layout(location=0) in vec2 inPosition; 
layout(location=1) in vec2 inUV;

out vec2 fragUV;

uniform mat4 MVP;

void main()
{
    // Position is 2D, we’ll treat it as a quad in XY plane
    vec4 pos = vec4(inPosition.x, inPosition.y, 0.0, 1.0);
    gl_Position = MVP * pos;
    fragUV = inUV;
}
