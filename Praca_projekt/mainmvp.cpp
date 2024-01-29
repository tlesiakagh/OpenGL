#include <glew.h>
#include <glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>

#include "shaderLoader.h"
#include "fileReader.h"

//funkcje algebry liniowej
#include "glm/vec3.hpp" 
#include "glm/vec4.hpp" 
#include "glm/mat4x4.hpp" 
#include "glm/gtc/matrix_transform.hpp"

using namespace std;

glm::vec3 cameraPos = glm::vec3(1.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(-1.0f, -0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void processInput(GLFWwindow* window);

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLint id, GLenum severity,
	GLsizei length, const char* message, const void* userParam);

void error_callback(int code, const char* desc) {
	fprintf(stderr, "ERROR [%d]: %s\n", code, desc);
	exit(EXIT_FAILURE); // Wyrzuca program przy niepoprawnym za³adowaniu GLFW
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
} 

GLint* prepareIndicies(int ncols, int nrows) {
	ncols = ncols / 4;
	nrows = nrows / 4;
	GLint* indices = new GLint[6 * ((ncols - 1) * (nrows - 1))];
	int indexPos = 0;
	int index = 0;

	std::cout << "Triangles amount: " << 2 * ((ncols - 1) * (nrows - 1)) << endl;

	for (int i = 0; i < nrows - 1; i++) {
		for (int j = 0; j < ncols - 1; j++) {
			
			indices[indexPos] = (i * nrows) + j;
			indices[indexPos + 1] = (i * nrows) + j+ 1;
			indices[indexPos + 2] = ((i + 1) * nrows) + j;
			indices[indexPos + 3] = (i * nrows) + j + 1;
			indices[indexPos + 4] = ((i + 1) * nrows) + j;
			indices[indexPos + 5] = ((i + 1) * nrows) + j + 1;

			indexPos = indexPos + 6;

			//cout << "indexPos: " << indexPos << endl;

		}
	}

	return indices;
}

int main(){
	int ncols, nrows;
	float* verticesF;
	GLint* indicesF;
	
	//if(daneZPliku == true)
	// Wczytanie informacji z pliku
	ncols = getNcols();
	nrows = getNrows();
	verticesF = loadVertices();
	//else{}
	/*
		1. Podanie z klawiatury wymiarów generowanego terenu i przypisanie do ncols, nrows
		2. Uruchomienie algorytmu generuj¹cego wysokoœci
	*/
	
	indicesF = prepareIndicies(ncols, nrows);

	/*for (int i = 0; i < 6 * ((5 - 1) * (5 - 1)); i++) {
		cout << indicesF[i] << ",";
	}*/

	int width=968, height=800;
	//int width=1920, height=1080; 

	glfwSetErrorCallback(error_callback); 
	//glfwPollEvents(); //<- wywo³uje b³¹d inicjalizacji i rzuca b³¹d, który obs³uguje mój callback

// GLFW
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	//sglfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // Wy³¹czenie zmiany rozmiaru okna
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Coœ dla Maca

	//GLFWwindow* window = NULL;
	//GLFWwindow* window = glfwCreateWindow(0, 0, "OpenGL", NULL, NULL); 
	GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL); 
	if (window == NULL)
	{
		//return -1;
		fprintf(stderr, "ERROR: window creation error");
		exit(EXIT_FAILURE);
	}
	
	glfwMakeContextCurrent(window);
	//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

// GLEW
	//glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		// Problem: glewInit failed, something is seriously wrong.
		fprintf(stderr, "ERROR: %s\n", glewGetErrorString(err));
	} 
		
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glFrontFace(GL_CW);

//^^^ Init code

// OpenGL settings
	
	/* Info about OpenGL */
	
	
	const GLubyte* vendor;
	const GLubyte* renderer;
	const GLubyte* version; 
	vendor = glGetString(GL_VENDOR);
	renderer = glGetString(GL_RENDERER);
	version = glGetString(GL_VERSION);
	printf("Vendor: %s\n", vendor);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);
	

	float vertices[] = {
	-1.0f,-1.0f,-1.0f, // triangle 1 : begin
	-1.0f,-1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f, // triangle 1 : end
	1.0f, 1.0f,-1.0f, // triangle 2 : begin
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f, // triangle 2 : end
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f
	};


	GLint indices[] = {  // note that we start from 0!
			0, 1, 2,
	};

	GLint shaderProgram = glCreateProgram();
	
	shaderProgram = loadShaders("vertex_shader.vert", "fragment_shader.glsl");

	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesF) / sizeof(float), vertices, GL_STATIC_DRAW);
	
	
	printf("VBOID: %u\n", VBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesF) / sizeof(GLint), indices, GL_STATIC_DRAW);
	
	
	printf("EBOID: % u\n", EBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	

	glBindVertexArray(0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//macierze
	glm::mat4 model = glm::mat4(1.0f); // macierz pozycja modeli (czyli tego co ogl¹damy)
	glm::mat4 view; // macierz z parametrami kamery
	glm::mat4 projection;  // macierz projekcji, czyli naszej perspektywy

// Main loop
	while (!glfwWindowShouldClose(window)){
		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
		
		glUseProgram(shaderProgram);

		// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
		projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 100.0f);
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		
		GLuint modelId = glGetUniformLocation(shaderProgram, "model");
		GLuint projectionId = glGetUniformLocation(shaderProgram, "projection");
		GLuint viewId = glGetUniformLocation(shaderProgram, "view");

		glUniformMatrix4fv(modelId, 1, GL_FALSE, &(model[0][0]));
		glUniformMatrix4fv(projectionId, 1, GL_FALSE, &(projection[0][0]));
		glUniformMatrix4fv(viewId, 1, GL_FALSE, &(view[0][0]));

		glBindVertexArray(VAO);

		//glDrawElements(GL_TRIANGLE_STRIP, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_INT, 0);
		//cout << sizeof(indices) / sizeof(indices[0]);

		glDrawElements(GL_TRIANGLE_STRIP, sizeof(indicesF)/sizeof(indicesF[0]), GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
		//cout << glGetError() << endl;

		glBindVertexArray(0);

		//cout << "errorAfterDraw" << endl;

		//glFlush();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();

	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	const float cameraSpeed = 0.05f; // adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		//cout << "W" << endl;
		cameraPos += cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		//cout << "S" << endl;
		cameraPos -= cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		//cout << "A" << endl;
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		//cout << "D" << endl;
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
}