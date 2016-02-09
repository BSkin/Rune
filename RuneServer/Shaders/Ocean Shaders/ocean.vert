#version 150

uniform sampler2D heightSampler;
uniform sampler2D choppySampler;
uniform sampler2D finalHeightSampler;

uniform mat4 projViewMatrix;
uniform mat4 worldMatrix;
uniform vec3 cameraPosition;
uniform float enableLighting;
uniform vec2 windDirection;
uniform float textureSize;
uniform float boatTextureSize;

attribute vec3 vertex;

out vec4 worldPosition;
out vec3 normal;
out float lighting;
out float height;

bool withinBoatTexture()
{
	vec2 dx = vertex.xz;// - cameraPosition.xz;
	return (dx.x <= boatTextureSize*0.5f && dx.x >= -boatTextureSize*0.5f) &&
		(dx.y <= boatTextureSize*0.5f && dx.y >= -boatTextureSize*0.5f); 	
}

vec2 worldPosToUV()
{
	return vec2(cameraPosition.x+vertex.x+0.5f, cameraPosition.z+vertex.z+0.5f) / 512.0f;//512.0f;
}

vec3 calculateNormal(vec2 uv)
{
	float slopeX = texture(heightSampler, uv+vec2(0.5f/textureSize,0)).r - texture(heightSampler, uv-vec2(0.5f/textureSize,0)).r;
	float slopeY = texture(heightSampler, uv+vec2(0,0.5f/textureSize)).r - texture(heightSampler, uv-vec2(0,0.5f/textureSize)).r;
	return normalize(vec3(-slopeX, 1.0f, -slopeY));
}

void main()
{
	lighting = enableLighting;

	vec2 uv = worldPosToUV();
	normal = calculateNormal(uv);
	vec4 pixel = withinBoatTexture() ? texture(finalHeightSampler, uv) : texture(heightSampler, uv);
	height = pixel.r;
	vec4 choppyVector = texture(choppySampler, uv);

	//worldPosition = worldMatrix * vec4(vertex.x + windDirection.x*pixel.b, height, vertex.z + windDirection.y*pixel.b, 1.0f);
	worldPosition = worldMatrix * vec4(vertex.x - choppyVector.r*100.0f, height, vertex.z - choppyVector.b*100.0f, 1.0f);
	//worldPosition = worldMatrix * vec4(vertex.x, height, vertex.z, 1.0f);
	gl_Position = projViewMatrix * worldPosition;
}