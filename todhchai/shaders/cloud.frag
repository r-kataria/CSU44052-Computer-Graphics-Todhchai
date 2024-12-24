#version 330 core

in vec2 fragUV;
in vec3 fragNormal;
in vec3 fragWorldPos;

// Output fragment color
out vec4 FragColor;

// Uniforms
uniform sampler2D textureSampler;

// For a simple directional light:
uniform vec3 lightDirection = vec3(0.0, 1.0, 1.0);  // example direction
uniform float ambientFactor  = 0.2;  // ambient light contribution
uniform int   numShades      = 3;    // how many “steps” for the toon shading

void main()
{
    // Sample the texture color
    vec4 texColor = texture(textureSampler, fragUV);
    
    // Normalize inputs
    vec3 norm      = normalize(fragNormal);
    vec3 lightDir  = normalize(-lightDirection); 
    // (if lightDirection is the direction from the light toward the scene, 
    // you might want to invert it with `-lightDirection` if you intend 
    // to measure the angle from the fragment to the light)

    // Basic diffuse term = max(dot(N, L), 0)
    float diffuse = max(dot(norm, lightDir), 0.0);

    // Apply discrete “stepping” to the diffuse factor
    // e.g. if numShades=3, we jump in increments of 1/3 
    float stepSize = 1.0 / float(numShades);
    float toonDiffuse = floor(diffuse / stepSize) * stepSize;

    // Add a small ambient factor, so it’s never totally black
    float finalIntensity = ambientFactor + (1.0 - ambientFactor) * toonDiffuse;

    // Combine with texture color
    FragColor = vec4(finalIntensity * texColor.rgb, texColor.a);
}
