#version 330 core
#define NUM_LIGHTS 4

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Light {
    vec3 Position;
    vec3 Color;
};

uniform Light lights[NUM_LIGHTS];
uniform sampler2D diffuseTexture;
uniform samplerCube depthMaps[NUM_LIGHTS];
uniform vec3 viewPos;
uniform float far_plane;
uniform bool shadows;
uniform int lightCount;

vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);


float ShadowCalculation(vec3 fragPos, vec3 lightPos, int index)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);

    // Sample closest depth from depth map (shadow cubemap)
    float closestDepth = texture(depthMaps[index], fragToLight).r;
    closestDepth *= far_plane; // Undo mapping [0;1]

    // Check whether current frag pos is in shadow
    float bias = 0.25;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    // PCF
    int samples = 20;
    float diskRadius = (1.0 + (length(viewPos - fragPos) / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMaps[index], fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane; // Undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples + 1); // +1 for the initial shadow

    return shadow;
}

void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);

    // ambient
    vec3 ambient = 0 * color;

    // lighting
    vec3 lighting = vec3(0.0);
    for(int i = 0; i < lightCount; i++)
    {
        // diffuse
        vec3 lightDir = normalize(lights[i].Position - fs_in.FragPos);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 result = lights[i].Color * diff * color;      
        // attenuation (use quadratic as we have gamma correction)
        float distance = length(fs_in.FragPos - lights[i].Position);
        result *= 1.0 / (distance * distance);
        
        // Shadow
        float shadow = shadows ? ShadowCalculation(fs_in.FragPos, lights[i].Position, i) : 0.0;
        result *= (1.0 - shadow);
        
        lighting += result;
                
    }
    vec3 result = ambient + lighting;
    // Check brightness for bloom
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    FragColor = vec4(result, texture(diffuseTexture, fs_in.TexCoords).a);
}
