#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

in vec4 FragPos[]; // Input from vertex shader

out vec4 GS_FragPos; // Output to fragment shader

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // Specify the cube map face to render to
        for(int i = 0; i < 3; ++i) // For each vertex of the triangle
        {
            GS_FragPos = FragPos[i];
            gl_Position = shadowMatrices[face] * GS_FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}