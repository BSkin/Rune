#version 150

uniform sampler2D textureSampler;

in vec2 uvVar;

out vec4 outColour;

void main()
{
	outColour = texture(textureSampler, uvVar);
	//float r = outColour.r*0.01f;
	//outColour = vec4(r,r,r,1);
}