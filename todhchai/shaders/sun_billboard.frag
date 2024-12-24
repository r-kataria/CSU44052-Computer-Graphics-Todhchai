#version 330 core

in vec2 fragUV;
out vec4 finalColor;

uniform sampler2D textureSampler;

void main()
{
    // Typically, your sun texture might be a bright disc with an alpha channel
    vec4 texColor = texture(textureSampler, fragUV);

    // In a bloom pipeline, this color is extremely bright, e.g.:
    // finalColor = vec4(texColor.rgb * 10.0, texColor.a);

    // For a simpler approach:
    finalColor = texColor;
}
