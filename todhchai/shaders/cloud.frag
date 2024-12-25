#version 330 core

in vec2 fragUV;
in vec3 fragNormal;
in vec3 fragWorldPos;

// Output fragment color
out vec4 FragColor;

// Uniforms
uniform sampler2D textureSampler;
uniform vec3 lightPos;            // Light position (e.g., (0.0, 4.0, 0.0))
uniform vec3 lightColor;          // Light color (e.g., vec3(1.0, 1.0, 1.0))
uniform vec3 objectColor;         // Object base color (can be vec3(1.0) if using textures exclusively)


void main()
{
  // Ambient lighting
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(fragNormal);

    vec3 lightDir = normalize(lightPos - fragWorldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    
    
    // Combine ambient and diffuse components
    vec3 lighting = ambient + diffuse;


    
    vec4 textureColor = texture(textureSampler, fragUV);
    vec3 result = lighting * textureColor.rgb;
    FragColor = vec4(result, textureColor.a);

}
