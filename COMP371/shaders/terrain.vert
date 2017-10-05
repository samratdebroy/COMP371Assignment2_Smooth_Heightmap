#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Position;
out float amplitude;
void main()
{
	amplitude = 100;
	Position = aPos;
	Position.y *= amplitude;
	gl_Position = projection * view * model * vec4(Position,1.0);
}