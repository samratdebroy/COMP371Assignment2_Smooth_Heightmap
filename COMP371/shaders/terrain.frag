#version 330

out vec4 FragColor;

in vec3 Position;
in float amplitude;

void main()
{
	float height = Position.y/amplitude;
	FragColor = vec4(height,height,height,1.0f);
}