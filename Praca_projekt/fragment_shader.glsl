#version 330 core 

in vec3 Normal;
in vec3 FragPos;
in float Height;

uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform float lightStr;
uniform int displayMode;
uniform int maxHeight;

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

	switch (displayMode)
	{
		case 1:
			FragColor = vec4(result, 1.0);
			break;
		case 2:
			FragColor  = vec4(0.5, 0.5, 0.5, 1.0);
			break;
		case 3:
			if((Height/maxHeight)>0.4)
				FragColor  = vec4(0.8,(1-0.7*Height/maxHeight),0,1);
			else
				FragColor  = vec4(2*(Height/maxHeight),0.72,0,1);	
			break;
	}
} 