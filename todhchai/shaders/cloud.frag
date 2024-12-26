#version 330 core

// Inputs from vertex shader
in vec2 fragUV;        // UV coordinates
in vec3 fragNormal;    // Normal vector in world space
in vec3 fragWorldPos;  // Fragment position in world space

// Output fragment color
out vec4 FragColor;

// Uniforms
uniform sampler2D textureSampler; // Texture sampler
uniform vec3 lightPos;            // Light position (e.g., (0.0, 4.0, 0.0))
uniform vec3 lightColor;          // Light color (e.g., vec3(1.0, 1.0, 1.0))
uniform vec3 viewPos;             // Camera/View position (e.g., (0.0, 0.0, 5.0))
uniform vec3 objectColor;         // Object base color (can be vec3(1.0) if using textures exclusively)

void main()
{
    // ----------------------------
    // Ambient Lighting
    // ----------------------------
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // ----------------------------
    // Diffuse Lighting
    // ----------------------------
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragWorldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // ----------------------------
    // Specular Lighting (Blinn-Phong)
    // ----------------------------
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragWorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float shininess = 32.0; // Hardcoded shininess exponent
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // ----------------------------
    // Combine Lighting Components
    // ----------------------------
    vec3 lighting = ambient + diffuse + specular;

    // ----------------------------
    // Texture Mapping
    // ----------------------------
    vec3 textureColor = texture(textureSampler, fragUV).rgb;

    // ----------------------------
    // Final Color Calculation
    // ----------------------------
    vec3 result = lighting * textureColor;
    FragColor = vec4(result, 1.0);
}
