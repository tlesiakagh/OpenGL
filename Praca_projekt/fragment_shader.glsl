#version 330 core 

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform float lightStr;

out vec4 FragColor;

void main()
{
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

	//diffuse
	vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor * lightStr/length(lightPos-FragPos);
    vec3 result = (ambient + diffuse) * objectColor;

    FragColor = vec4(result, 1.0);
	//FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
} 