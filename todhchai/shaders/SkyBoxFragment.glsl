#version 330 core
out vec4 color;
in vec3	texDir;					

uniform samplerCube skyBox;
void main()
{
	//vec3 t = vec3(texDir.y, texDir.x, texDir.z);
	color = texture(skyBox, texDir);
	//color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}