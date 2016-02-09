 #version 150

uniform sampler2D FFTSampler;
uniform float scaleFactor;
uniform float textureSize;

in vec2 uvVar;

out vec4 outColour;

void main()
{
	outColour = scaleFactor * texture2D(FFTSampler, uvVar);
	outColour = ((mod((textureSize*(uvVar.x + uvVar.y)), 2.0f)) == 1.0f) ? outColour : -outColour;
}