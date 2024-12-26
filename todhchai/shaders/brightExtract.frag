// shaders/brightExtract.frag

#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D scene;

void main()
{
    vec3 color = texture(scene, TexCoords).rgb;
    // Calculate luminance
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float threshold = 1.5; // Try lowering to 0.4 or 0.3
    if(brightness > threshold)
        FragColor = vec4(color, 1.0);
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
