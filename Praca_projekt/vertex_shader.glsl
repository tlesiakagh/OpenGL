#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aNormal;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

out vec3 FragPos;
out vec3 Normal;
out float Height;

void main()
{
	FragPos = aPos;
	Normal = aNormal;
	Height = aPos.y;

    gl_Position = MVP * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}