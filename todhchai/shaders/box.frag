#version 330 core

// Inputs from vertex shader
in vec2 UV;        // UV coordinates
in vec3 FragPos;   // Fragment position in world space
in vec3 Normal;    // Normal vector in world space

// Output color
out vec4 finalColor;

// Uniforms
uniform sampler2D textureSampler; // Texture sampler
uniform vec3 lightPos;            // Light position (e.g., (0.0, 4.0, 0.0))
uniform vec3 lightColor;          // Light color (e.g., vec3(1.0, 1.0, 1.0))
uniform vec3 objectColor;         // Object base color (can be vec3(1.0) if using textures exclusively)

void main()
{
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Combine ambient and diffuse components
    vec3 lighting = ambient + diffuse;

    // Sample the texture color
    vec3 textureColor = texture(textureSampler, UV).rgb;

    // Combine texture color with lighting
    vec3 result = lighting * textureColor;

    // Set the final fragment color
    finalColor = vec4(result, 1.0);
}
