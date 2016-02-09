#version 150

uniform sampler2D textureSampler;
uniform vec3 cameraPosition;

in float lighting;
in vec2 uvVar;
in vec3 normalVar;
out vec4 worldPosition;

out vec4 outColour;

float diffuseLighting(vec3 surfaceNormal, vec3 toLight)
{
	return max(0.0f, dot(surfaceNormal, toLight));
}

float specularLighting(vec3 toCamera, vec3 surfaceNormal, vec3 toLight, float shininess)
{
	vec3 reflectedVector = reflect(normalize(toLight), normalize(surfaceNormal));
	float cosAngle = max(0.0f, dot(normalize(toCamera), normalize(reflectedVector)));
	return pow(cosAngle, shininess);
}

void main()
{
	vec3 sunDir = normalize(vec3(0.5f, -0.5f, 0.5f));
	float diffuse = diffuseLighting(normalVar, -sunDir);
	float specular = specularLighting(cameraPosition-worldPosition.xyz, normalVar, sunDir, 1.0f);
	vec4 pixel = texture(textureSampler, uvVar);
	outColour = ((lighting >= 0.5f) ? vec4(pixel.rgb*(diffuse*0.95f + 0.05f) + specular*vec3(0.1f, 0.1f, 0.1f), pixel.a) : pixel.rgba);
}