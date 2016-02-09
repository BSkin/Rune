#version 130

uniform mat4 projViewMatrix;
uniform mat4 worldMatrix;

attribute vec3 vertex;

void main()
{
	gl_Position = projViewMatrix * worldMatrix * vec4(vertex, 1.0f);
}