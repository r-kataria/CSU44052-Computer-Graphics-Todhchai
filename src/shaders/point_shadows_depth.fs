#version 330 core
in vec4 GS_FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
    float lightDistance = length(GS_FragPos.xyz - lightPos);

    // Map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;

    // Write this as modified depth
    gl_FragDepth = lightDistance;
}