 #version 150

uniform sampler2D oceanHeightSampler;
uniform sampler2D boatHeightSampler;
uniform vec3 cameraPosition;
uniform float gridSize;
uniform float boatTextureSize;

in vec2 uvVar;

out vec4 outColour;

bool withinBoatTexture()
{
	vec2 dx = uvVar - mod(cameraPosition.xz, gridSize) / gridSize;
	float w = boatTextureSize / gridSize;
	//return dx.x < w*0.5f && dx.x > -w*0.5f && dx.y < w*0.5f && dx.y > -w*0.5f;

	return !(dx.x > w*0.5f && dx.x < (1.0f - w*0.5f)) && !(dx.y > w*0.5f && dx.y < (1.0f - w*0.5f)); 	
}

vec2 calculateBoatUV()
{
	vec2 camUV = mod(cameraPosition.xz, gridSize) / gridSize;
	vec2 outUV = (uvVar - camUV) * gridSize / boatTextureSize + vec2(0.5f, 0.5f);
	//outUV.x = mod(outUV.x, 1.0f);
	//outUV.y = mod(outUV.y, 1.0f);
	return outUV;
}

void main()
{
	vec4 pixel = texture(oceanHeightSampler, uvVar);
	bool within = withinBoatTexture();
	float height = within ? min(pixel.r, -52.0f + 100.0f * texture(boatHeightSampler, calculateBoatUV()).r) : pixel.r;
	height = min(pixel.r, -52.0f + 100.0f * texture(boatHeightSampler, calculateBoatUV()).r);
	outColour = vec4(height, pixel.gba);
}