// shaders/blur.frag

#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform bool horizontal;

// Expanded Gaussian weights (9-tap)
const float weight[9] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216, 0.005424, 0.001356, 0.000341, 0.000085);

void main()
{
    vec2 texOffset = 1.0 / textureSize(image, 0); // Size of single texel
    vec3 result = texture(image, TexCoords).rgb * weight[0];
    for(int i = 1; i < 9; ++i)
    {
        if(horizontal)
        {
            result += texture(image, TexCoords + vec2(texOffset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(texOffset.x * i, 0.0)).rgb * weight[i];
        }
        else
        {
            result += texture(image, TexCoords + vec2(0.0, texOffset.y * i)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(0.0, texOffset.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}
