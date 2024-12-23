#version 330 core

in vec2 uv;  // from vertex shader

uniform sampler2D textureSampler;

out vec4 finalColor;

void main()
{
    // Just fetch the texture
    finalColor = texture(textureSampler, uv);
}
