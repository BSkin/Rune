#version 150

uniform sampler2D h0Sampler;
uniform float kConst;
uniform float time;

in vec2 uvVar;

out vec4 outColour;

vec2 ht(vec2 a, vec2 b)
{
	float k = length(kConst*(uvVar - vec2(0.5f, 0.5f)));
	float wt = sqrt(2.0f*9.81f * k * (1 + k*k*1.0f*1.0f)) * time * 0.001f;
	float c = cos(wt);
	float s = sin(wt);
	//return vec2(a.x*c - a.y*s + b.x*c + b.y*s, -(a.y*c + a.x*s + b.y*c - b.x*s));
	return vec2((a.x + b.x)*c + (-a.y + b.y)*s, -((a.y + b.y)*c + (a.x - b.x)*s));
	//return vec2((a.x + b.x)*c + (-a.y + b.y)*s, (a.y + b.y)*c + (a.x - b.x)*s);
}

void main()
{
	vec2 pos1 = uvVar;
	vec2 pos2 = vec2(1.0f, 1.0f) - pos1;
	vec2 input1 = texture(h0Sampler, pos1).xy;
	vec2 input2 = texture(h0Sampler, pos2).xy;
	vec2 htResult = ht(input1, input2);
	outColour = vec4(htResult, 0, 1);
}