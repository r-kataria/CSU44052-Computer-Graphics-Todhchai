  #version 330 core
  in vec2 UV;
  out vec3 color;

  uniform int Flip;

  uniform sampler2D textureSampler;
  void main() 
  {
      if(Flip == 0) {
        color = texture(textureSampler, UV).rgb;
      }
         if(Flip == 1) {
        color = texture(textureSampler, 1.0f * UV).rgb;
      }

     // color = vec3(1.0f);
  }

  