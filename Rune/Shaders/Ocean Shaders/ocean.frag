#version 150

uniform vec3 cameraPosition;

in vec4 worldPosition;
in vec3 normal;
in float lighting;
in float height;

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
	float diffuse = diffuseLighting(normal, -sunDir);
	float specular = specularLighting(cameraPosition-worldPosition.xyz, normal, sunDir, 10.0f);

	//outColour = ((lighting >= 0.5f) ? vec4(intensity*vec3(0.0f, 0.0f, 0.5f), 1.0f) : vec4(0.0f, 0.0f, 0.5f + height/3.0f, 1.0f));
	outColour = vec4(diffuse*vec3(0.1f, 0.1f, 0.3f)+vec3(0.0f, 0.0f, 0.0f)+specular*vec3(0.3f, 0.3f, 0.3f), 1.0f);
}