#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D scene;     // the full HDR scene
uniform sampler2D bloomBlur; // blurred bright
uniform float exposure;

void main()
{
    // Retrieve HDR color
    vec3 hdrColor = texture(scene, TexCoords).rgb;
    // Retrieve Bloom color
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    // Combine HDR and Bloom
    hdrColor += bloomColor;

    // Exposure Tone Mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

    // Gamma Correction
    float gamma = 2.2;
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}
