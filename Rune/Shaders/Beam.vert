#version 150

uniform mat4 projViewMatrix;
uniform mat4 worldMatrix;
//uniform mat4 normalMatrix;
uniform vec4 beamColour;

attribute vec3 vertex;
attribute vec2 uv;
//attribute vec3 normal;

out vec4 colour;

void main()
{
	colour = beamColour;
	
	//worldPosition = worldMatrix * vec4(vertex, 1.0f);
	gl_Position = projViewMatrix * worldMatrix * vec4(vertex, 1.0f);
	//normalVar = vec3(normalMatrix * vec4(normal, 0.0f));
}