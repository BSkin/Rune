#version 150

uniform sampler2D textureSampler;

in vec2 uvVar;

out vec4 outColour;

vec2 complexMultiply(vec2 a, vec2 b)
{
	return vec2(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

void main()
{
	vec4 pixel = texture(textureSampler, uvVar);
	vec2 normalK = normalize(uvVar - vec2(0.5f, 0.5f));
	outColour = vec4(complexMultiply(vec2(0, normalK.x), pixel.rg), complexMultiply(vec2(0, normalK.y), pixel.rg));
}