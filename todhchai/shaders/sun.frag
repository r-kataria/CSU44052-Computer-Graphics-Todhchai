#version 330 core

out vec4 FragColor;

void main()
{
    // Give the sun a very bright color so bloom can pick it up
FragColor = vec4(4.0, 4.0, 2.0, 1.0); // Even higher RGB values
}
