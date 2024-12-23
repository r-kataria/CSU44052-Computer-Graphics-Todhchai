#version 330 core
layout(location = 0) in vec3 position; // Assuming positions are vec3

out vec3 texDir; // Texture direction
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transform the position with view and projection matrices
    vec4 vertex = projection * view * vec4(position, 1.0f);
    gl_Position = vertex; // Assign the transformed position to gl_Position

    // Pass the texture direction (unchanged position)
    texDir = position;
}
