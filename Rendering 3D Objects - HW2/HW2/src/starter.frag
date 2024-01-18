#version 450

layout(location = 0) out vec4 color;
 
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragPosition; 
layout(location = 2) in vec3 cameraPosition; 
layout(location = 3) in vec3 lightPos;


void main() {
	
	vec3 objectColor = vec3(0.8, 0.6, 1.0);
	vec3 lightColor = vec3(1.0, 1.0, 1.0);

	// Implement Ambient Lighting
	float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

	// Implement Diffuse Lighting
	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(lightPos - fragPosition);  
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// Implement Specular Lighting
	float specularStrength = 1.0;
    float shininess = 64.0; // Adjust shininess based on your needs
    vec3 viewDir = normalize(cameraPosition - fragPosition); // Assuming camera at (0,0,0)
    vec3 reflectDir = reflect(-lightDir, norm);
	float spec;
	if(diff == 0)
	{
		spec = 0;
	}
	else
	{
		spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	}
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
	color = vec4(result, 1.0);
}

