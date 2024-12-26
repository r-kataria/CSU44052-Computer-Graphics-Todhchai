#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
    // Calculate distance between fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);

    // Map to [0,1] range for storing in depth map
    lightDistance = lightDistance / far_plane;

    // Write to depth buffer
    gl_FragDepth = lightDistance;
}