#version 150

uniform mat4 projViewMatrix;
uniform mat4 worldMatrix;
uniform mat4 normalMatrix;
uniform float enableLighting;

attribute vec3 vertex;
attribute vec2 uv;
attribute vec3 normal;

out float lighting;
out vec2 uvVar;
out vec3 normalVar;
out vec4 worldPosition;

void main()
{
	lighting = enableLighting;
	uvVar = uv;
	
	worldPosition = worldMatrix * vec4(vertex, 1.0f);
	gl_Position = projViewMatrix * worldPosition;
	normalVar = vec3(normalMatrix * vec4(normal, 0.0f));
}