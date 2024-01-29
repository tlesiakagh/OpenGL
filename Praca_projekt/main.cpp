#include <glew.h>
#include <glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <cmath>
#include <random>
#include <ctime>

#include "shaderLoader.h"
#include "fileReader.h"
#include "terrainGenerator.h"
#include "globalVariables.h"

#include "glm/vec3.hpp" 
#include "glm/vec4.hpp" 
#include "glm/mat4x4.hpp" 
#include "glm/gtc/matrix_transform.hpp"

/*
TO DO:
	1. Przyjrzeæ siê generowaniu wektorów normalnych
	2. Pobieranie inputu od u¿ytkownika na temat: [V]
		a) maxRandomValue, 
		b) symetrycznoœci przedzia³u losowania (tj. 0<->max czy -max<->max), 
		c) tego czy s¹ te same naro¿niki czy nie [V]

	3. Zrobiæ porz¹dek z u³o¿eniem funkcji i zmiennych; na podobnej zasadzie do terrainGenerator.h [DONE]
	4. Zrobiæ tak, aby DiamondSquare mia³o wywo³ywa³o tylko 2 swoje funkcje, tj. InitGenerator, prepareElevationDataFromDiamondSquare (które zwraca dane od razu do co vertices, czyli potrzeba funkcji zwracaj¹cej kolejne wartoœci
		albo stworzyæ funkcje, która zwraca w ca³oœci wygenerowan¹ tablicê vertices (tak jak to wygl¹da w przypadku czytania z pliku) [V]
	5. Zrobiæ jedn¹ funkcjê, która przygotowuje nrows, ncols, vertices (w funkcji znajdowa³ by siê kontakt z u¿ytkownikem) 
	6. Ogarn¹æ aby u¿ytkownik móg³ kontrolowaæ to czy przy DiamondSquare s¹ te same naro¿niki (2. argument initGenerator), i z jakiego przedzia³u losowane s¹ wartoœci (3. argument initGenerator)

	#if defined PROJECT1
	{
		// some code
	}
	#endif
*/

//#define DEBUGGING
#define LISTING

//#define NORMVERT
#define NORMTRIANGLE

using namespace std;

// Ustawienia kamery
glm::vec3 cameraPos = glm::vec3(500.0f, 100.0f, 500.0f);
glm::vec3 cameraFront = glm::vec3(-250.0f, -100.0f, -250.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float fov = 50.0f;
const float cameraSpeed = 0.5;
const int rotationMultiplier = 0.5;

// Ustawienia oœwietlenia
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 objectColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 lightPos = glm::vec3(50.0f, 50.0f, 50.0f);
float lightStr = 25.0;
const float lightSpeed = 1.0;

int windowWidth, windowHeight;

glm::mat4 Model = glm::mat4(1.0f);

GLuint VAO, VBO, EBO, VBO_normals;

int nrows; // deklaracja zmiennej globalnej
int ncols; // deklaracja zmiennej globalnej
int verticesSize; // deklaracja zmiennej globalnej
float* vertices; // deklaracja zmiennej globalnej

unsigned int* indices;
int indicesSize;
float* normals;
float* elevationDataFromDiamondSquare;

//----------------------------------------OpenGL callbacks--------------------------------------
void error_callback(int code, const char* desc);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

//----------------------------------------Funkcje pomocnicze------------------------------------
void setWindowSize(int width, int height);
bool whichDataToDraw();
void prepareDataForDrawing();
void keyPressHandling(GLFWwindow* window);
void makeRandomPartOfTerrainFlat();
void randomWithMT();
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);

//-----------------------------------Funkcje przetwarzaj¹ce dane--------------------------------
unsigned int* prepareIndicies();
float* prepareNormals();
glm::vec3 calculateNormalForVertex(int row, int col, glm::vec3 centerVertex);
glm::vec3 calculateNormalForVertex2(int xIndex, glm::vec3 centerVertex);
glm::vec3 calculateNormalForTriangle(int xIndex, glm::vec3 centerVertex);

int main(){
	srand(time(NULL));
//--------------------------------------PRZYGOTOWANIE DANYCH------------------------------------
	prepareDataForDrawing();
	
	#if defined LISTING
	{
		cout << "Vertices amount: " << verticesSize << endl;
		cout << "Indices amount: " << indicesSize << endl;
		cout << "Triangles amount: " << indicesSize / 3 << endl;
	}
	#endif

//------------------------------------------INICJALIZACJA---------------------------------------
	/* GLFW */
	glfwSetErrorCallback(error_callback);
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	setWindowSize(968, 800);
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "ERROR: window creation error");
		exit(EXIT_FAILURE);
	}
	
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	/* GLEW */
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "ERROR: %s\n", glewGetErrorString(err));
	} 
		
	/* Informacja o wersji OpenGL */
	const GLubyte* renderer;
	const GLubyte* version;
	renderer = glGetString(GL_RENDERER);
	version = glGetString(GL_VERSION);
	int majorVersionGLFW, minorVersionGLFW;
	glfwGetVersion(&majorVersionGLFW, &minorVersionGLFW, NULL);

	printf("Renderer: %s\n", renderer);
	printf("OpenGL version: %s\n", version);
	printf("GLFW version: %d.%d\n", majorVersionGLFW, minorVersionGLFW);
	printf("GLEW version: %s\n", glewGetString(GLEW_VERSION));

	/* Uruchamianie mo¿liwoœci wyœwietlania b³êdów OpenGL */
	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
	}
	
//----------------------------------------SHADERY I BUFORY--------------------------------------
	unsigned int shaderProgram = glCreateProgram();
	shaderProgram = loadShaders("vertex_shader.glsl", "fragment_shader.glsl");

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenBuffers(1, &VBO_normals);

	#if defined LISTING
	{
		cout << "VBOID: " << VBO << endl;;
		cout << "EBOID: " << EBO << endl;
		cout << "VBO_normalsID: " << VBO_normals << endl;
	}
	#endif

	/*
	string trash;
	cout << "Enter sth ";
	cin >> trash;
	*/
//------------------------------------------MAIN LOOP-------------------------------------------
	while (!glfwWindowShouldClose(window)){
		keyPressHandling(window);
		
		glEnable(GL_DEPTH_TEST);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glLoadIdentity();
		
		glUseProgram(shaderProgram);

		glm::mat4 Projection = glm::perspective(glm::radians(fov), (float)windowWidth / (float)windowHeight, 0.1f, 5000.0f);
		glm::mat4 View = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 mvp = Projection * View * Model;
		
		GLuint mvpID = glGetUniformLocation(shaderProgram, "MVP");
		GLuint objectColorId = glGetUniformLocation(shaderProgram, "objectColor");
		GLuint lightColorId = glGetUniformLocation(shaderProgram, "lightColor");
		GLuint lightPosId = glGetUniformLocation(shaderProgram, "lightPos");
		GLuint lightStrId = glGetUniformLocation(shaderProgram, "lightStr");

		glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);
		glUniform3fv(objectColorId, 1, &objectColor[0]);
		glUniform3fv(lightColorId, 1, &lightColor[0]);
		glUniform3fv(lightPosId, 1, &lightPos[0]);
		glUniform1f(lightStrId, lightStr);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, verticesSize * sizeof(float), vertices, GL_DYNAMIC_DRAW); //GL_STATIC_DRAW

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize * sizeof(int), indices, GL_DYNAMIC_DRAW);//GL_STATIC_DRAW

		glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
		glBufferData(GL_ARRAY_BUFFER, verticesSize * sizeof(float), normals, GL_DYNAMIC_DRAW);//GL_STATIC_DRAW

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_normals);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindVertexArray(0);
		glBindVertexArray(VAO);
		
		//glDrawElements(GL_TRIANGLES, 6 * ((ncols - 1) * (nrows - 1)), GL_UNSIGNED_INT, 0);
		glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0);
		
		glFlush();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();

	return 0;
}

//----------------------------------------OpenGL callbacks--------------------------------------
void error_callback(int code, const char* desc) {
	fprintf(stderr, "ERROR [%d]: %s\n", code, desc);
	exit(EXIT_FAILURE); // Wyrzuca program przy niepoprawnym za³adowaniu GLFW
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	float minFOV = 10.0f;
	float maxFOV = 90.0f;
	int scrollSpeed = 2;

	fov -= scrollSpeed * (float)yoffset;
	if (fov < minFOV) {
		fov = minFOV;
	}
	if (fov > maxFOV) {
		fov = maxFOV;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		//cout << "F" << endl;
		//makeRandomPartOfTerrainFlat();
		//prepareNormals();
		//prepareIndicies();
		//randomWithMT();
	}
}

//---------------------------------------Funkcje pomocnicze-------------------------------------
void setWindowSize(int width, int height) {
	windowWidth = width;
	windowHeight = height;
}

bool whichDataToDraw() {
	int userInput;
	string userInputStream;
	bool drawingDataFromFile = true;

	cout << "Shown data will be: " << endl;
	cout << "generated with Diamond-Square algorithm [input 1]" << endl;
	cout << "loaded from one of files [input 2]" << endl;
	//userInputStream = "2";
	//cout << "your option: " << userInputStream;
	cout << "your option: ";
	//cout << endl;
	cin >> userInputStream;

	if (userInputStream.length() == 1) {
		userInput = stoi(userInputStream);

		switch (userInput) {
		case 1:
			drawingDataFromFile = false;
			break;
		case 2:
			
			cout << "Enter number in [] to draw area!" << endl;
			cout << "Tatry [1]" << endl;
			cout << "Gore [2]" << endl;
			cout << "Valdez [3]" << endl;
			cout << "Choosen number: ";
			cin >> userInputStream;
			
			//userInputStream = "1";
			drawingDataFromFile = true;
			setDrawDataPath(stoi(userInputStream));
			break;
		default:
			cout << "There is not such option!" << endl;
			exit;
			break;
		}
	}
	else {
		cout << "WARRNING: Wrong input!" << endl;
		exit;
	}

	return drawingDataFromFile;
}

void prepareDataForDrawing() {
	int userInput;
	string dataReadFromFile, userInputStream;
	bool drawingDataFromFile = true;

	drawingDataFromFile = whichDataToDraw();

	cout << "Preparing vertices..." << endl;
	if (drawingDataFromFile) {
		ncols = getNcols();
		nrows = getNrows();
		vertices = loadVertices();
	}
	else {
		int startingStepSize = 0;

		// Wspó³rzêdne generowane przez Diamond-Square
		cout << "Size of array for Diamond-Square: 2^n+1" << endl;
		cout << "Please input value of n" << endl;
		cout << "n = ";
		cin >> userInputStream;

		//cout << endl;
		int n = 8;
		//cout << "n = " << n << endl;
		n = stoi(userInputStream);
		
		ncols = pow(2, n) + 1;
		nrows = pow(2, n) + 1;
		//startingStepSize = (ncols - 1) / 4;
		startingStepSize = ncols - 1;

		cout << "nrows: " << nrows << " ncols: " << ncols << endl;

		#if defined LISTINGDIAMONDSQUARE
		{
			cout << "nrows: " << nrows << endl;
			cout << "ncols: " << ncols << endl;
			cout << "startingStepSize: " << startingStepSize << endl;
		}
		#endif

		vertices = new float[3 * nrows * ncols];
		
		cout << "Same random value for all starting corners." << endl;
		cout << "Please input [1] if you want them to be same or [0] if you want different" << endl;
		//cout << "choosen option: " << userInputStream;
		cout << "choosen option: ";
		cin >> userInputStream;
		sameValueInCorners = false;
		//cout << "sameValueInCorners: " << ((sameValueInCorners) ? "TRUE" : "FALSE") << endl;
		
		if (stoi(userInputStream) == 1) {
			sameValueInCorners = true;
		}
		else if (stoi(userInputStream) == 0) {
			sameValueInCorners = false;
		}
		else {
			cout << "WARNING: There is not such option!" << endl;
		}
		

		cout << "Plese enter max value of range for randomization!" << endl;
		//cout << "max value: " << userInputStream;
		cout << "max value: ";
		cin >> userInputStream;
		maxRandomValue = 50;
		//cout << "maxRandomValue: " << maxRandomValue << endl;
		
		cout << "If you want values to be randomize from range 0 <-> " << maxRandomValue << " enter [0]" << endl;
		cout << "If you want them to be randomize from -"<< maxRandomValue << " <-> " << maxRandomValue << " enter [1]" << endl;
		//cout << "choosen option: " << userInputStream;
		cout << "choosen option: ";
		cin >> userInputStream;
		isRandomRangeSymmetrical = true;
		//cout << "isRandomRangeSymmetrical: " << ((isRandomRangeSymmetrical) ? "TRUE" : "FALSE") << endl;
		
		if (stoi(userInputStream) == 1) {
			isRandomRangeSymmetrical = true;
		}
		else if (stoi(userInputStream) == 0) {
			isRandomRangeSymmetrical = false;
		}
		else {
			cout << "WARNING: There is not such option!" << endl;
		}
		

		initGenerator(startingStepSize); // add randomRangeSymmetrical argument
		prepareElevationDataWithDiamondSquare(startingStepSize);

		int vertexIndex = 0;
		for (int row = nrows - 1; row >= 0; row--) {
			for (int col = 0; col < ncols; col++) {
				
				#if defined DEBUGGINGDIAMONDSQUARE
				{
					cout << "Vertex nr " << vertexIndex << " = ";
					cout << "(" << vertices[vertexIndex] << ", ";
					cout << vertices[vertexIndex + 1] << ", ";
					cout << vertices[vertexIndex + 2] << ")" << endl;
				}
				#endif

				vertexIndex = vertexIndex + 3;
			}
		}
	}
	cout << "Vertices prepared!" << endl;

	verticesSize = 3 * nrows * ncols;
	cout << "Preparing normals..." << endl;
	normals = prepareNormals();
	cout << "Normals prepared!" << endl;
	cout << "Preparing indicies..." << endl;
	indices = prepareIndicies();
	cout << "Indicies prepared!" << endl;
	indicesSize = 6 * ((ncols - 1) * (nrows - 1));
}

void keyPressHandling(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		//cout << "ESC" << endl;
		glfwSetWindowShouldClose(window, true); // Zamkniêcie okna
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		//cout << "W" << endl; 
		//cout << "cameraPos.y: " << cameraPos.y << endl;
		//cameraPos += cameraSpeed * cameraFront; // Przybli¿anie(?)
		cameraPos.y += cameraSpeed; // Ruch kamery w górê
		//cameraPos.z += cameraSpeed; // Ruch kamery do przodu
		//cameraPos = glm::normalize(glm::cross(cameraFront, glm::normalize(glm::cross(cameraFront, cameraUp))))* cameraSpeed;
		//cameraFront.y -= cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		//cout << "S" << endl;
		//cameraPos -= cameraSpeed * cameraFront; // Oddalanie(?)
		cameraPos.y -= cameraSpeed; // Ruch kamery w dó³
		//cameraPos.z -= cameraSpeed; // Ruch kamery do przodu
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		//cout << "A" << endl;
		//cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; // Ruch kamery w lewo
		cameraPos.x -= cameraSpeed; // Ruch kamery w górê
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		//cout << "D" << endl;
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; // Ruch kamery w prawo       
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		//cout << "Q" << endl;
		Model = glm::rotate(Model, glm::radians(rotationMultiplier * cameraSpeed), glm::vec3(0.0f, 1.0f, 0.0f)); // Obrót kamery w lewo      
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		//cout << "E" << endl;
		Model = glm::rotate(Model, glm::radians(rotationMultiplier * -cameraSpeed), glm::vec3(0.0f, 1.0f, 0.0f)); // Obrót kamery w lewo      
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		//cout << "Z" << endl; 
		cameraPos.z -= cameraSpeed; // Ruch kamery do przodu
		cameraFront.z += cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		//cout << "C" << endl;
		cameraPos.z += cameraSpeed; // Ruch kamery do ty³u
		cameraFront.z -= cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		//cout << "strza³ka w górê" << endl; 
		lightPos.z -= lightSpeed; // Ruch œwiat³a do przodu
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		//cout << "strza³ka w dó³" << endl;
		lightPos.z += lightSpeed; // Ruch œwiat³a do ty³u
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		//cout << "strza³ka w prawo" << endl; 
		lightPos.x += lightSpeed; // Ruch œwiat³a w prawo
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		//cout << "strza³ka w lewo" << endl;
		lightPos.x -= lightSpeed; // Ruch œwiat³a w lewo
	}
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		//cout << "U" << endl;
		lightPos.y += lightSpeed; // Ruch œwiat³a w górê
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		//cout << "J" << endl;
		lightPos.y -= lightSpeed; // Ruch œwiat³a w dó³
	}
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		//cout << "M" << endl;
		lightStr += 1; // Wzmocnienie si³y oœwietlenia
	}
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		//cout << "N" << endl;
		lightStr -= 1; // Os³abienie si³y oœwietlenia
	}
}

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

//----------------------------------Funkcje przetwarzaj¹ce dane---------------------------------
unsigned int* prepareIndicies() {
	unsigned int* indices = new unsigned int[6 * ((ncols - 1) * (nrows - 1))];
	int indexPos = 0, index = 0;
	//int trash;

	for (int row = 0; row < nrows - 1; row++) {
		for (int col = 0; col < ncols - 1; col++) {

			indices[indexPos] = (row * ncols) + col;
			indices[indexPos + 1] = (row * ncols) + col + 1;
			indices[indexPos + 2] = ((row + 1) * ncols) + col;
			indices[indexPos + 3] = (row * ncols) + col + 1;
			indices[indexPos + 4] = ((row + 1) * ncols) + col;
			indices[indexPos + 5] = ((row + 1) * ncols) + col + 1;
			/*
			cout << "Triangle: ";
			cout << indices[indexPos] << ", ";
			cout << indices[indexPos + 1] << "; ";
			cout << indices[indexPos + 2] << endl;

			cin >> trash;

			cout << "Triangle: ";
			cout << indices[indexPos + 3] << ", ";
			cout << indices[indexPos + 4] << "; ";
			cout << indices[indexPos + 5] << endl;

			cin >> trash;
			*/
			indexPos = indexPos + 6;

			//cout << "indexPos: " << indexPos << endl;

		}
	}

	return indices;
}

float* prepareNormals() {
	glm::vec3 tmpNormal;
	int xIndexInNormals = 0;
	int sizeOfNormals = 3 * nrows * ncols;
	cout << "sizeOfNormals: " << sizeOfNormals << endl;
	normals = new float[sizeOfNormals];

	/*
	for (int row = nrows-1; row >= 0; row--) {
		for (int col = 0; col < ncols; col++) {
			tmpNormal = calculateNormalForVertex(row, col, glm::vec3(vertices[xIndexInNormals], vertices[xIndexInNormals + 1], vertices[xIndexInNormals + 2]));
			//cout << "tmpNormal: (" << tmpNormal.x << ", " << tmpNormal.y << ", " << tmpNormal.z << ")" << endl;
			cout << "xIndexInNormals = " << xIndexInNormals << endl;
			normals[xIndexInNormals] = tmpNormal.x;
			normals[xIndexInNormals + 1] = tmpNormal.y;
			normals[xIndexInNormals + 2] = tmpNormal.z;
			//cout << "normals: (" << normals[xIndexInNormals] << ", " << normals[xIndexInNormals + 1] << ", " << normals[xIndexInNormals + 2] << ")" << endl;
			xIndexInNormals += 3;
		}
	}
	*/

	/*
	for (int xIndex = 0; xIndex < sizeOfNormals; xIndex += 3) {
		cout << "xIndex: " << xIndex << endl;
		tmpNormal = calculateNormalForVertex2(xIndex, glm::vec3(vertices[xIndex], vertices[xIndex + 1], vertices[xIndex + 2]));
		normals[xIndex] = tmpNormal.x;
		normals[xIndex + 1] = tmpNormal.y;
		normals[xIndex + 2] = tmpNormal.z;
		//cout << "normals: (" << normals[xIndex] << ", " << normals[xIndex + 1] << ", " << normals[xIndex + 2] << ")" << endl;
	}
	*/

	#if defined NORMVERT
	{
		// Wektory normalne s¹ wspó³rzêdnymi wierzcho³ków
		for (int xIndex = 0; xIndex < sizeOfNormals; xIndex += 3) {
			//cout << "xIndex = " << xIndex << endl;
			glm::vec3 vertex(vertices[xIndex], vertices[xIndex + 1], vertices[xIndex + 2]);
			//vertex = glm::normalize(vertex);
			normals[xIndex] = vertex.x;
			normals[xIndex + 1] = vertex.y;
			normals[xIndex + 2] = vertex.z;
			//cout << "normals: (" << normals[xIndex] << ", " << normals[xIndex + 1] << ", " << normals[xIndex + 2] << ")" << endl;
		}
	}
	#endif

	#if defined NORMTRIANGLE
	{
		// Wektory normalny jest jeden dla ka¿dego trójk¹ta (bez liczenia œredniej)
		for (int xIndex = 0; xIndex < sizeOfNormals; xIndex += 3) {
			tmpNormal = calculateNormalForTriangle(xIndex, glm::vec3(vertices[xIndex], vertices[xIndex + 1], vertices[xIndex + 2]));
			//cout << "tmpNormal: (" << tmpNormal.x << ", " << tmpNormal.y << ", " << tmpNormal.z << ")" << endl;
			//cout << "xIndex = " << xIndex << endl;
			normals[xIndex] = tmpNormal.x;
			normals[xIndex + 1] = tmpNormal.y;
			normals[xIndex + 2] = tmpNormal.z;
			//cout << "normals: (" << normals[xIndex] << ", " << normals[xIndex + 1] << ", " << normals[xIndex + 2] << ")" << endl;
		}
	}
	#endif

	/*
	// Wektory normalne liczone jako œrednia z wektorów dla przyleg³ych trójk¹tów 
	for (int xIndex = 0; xIndex < sizeOfNormals; xIndex += 3) {
		tmpNormal = calculateNormalForTriangle(xIndex, glm::vec3(vertices[xIndex], vertices[xIndex + 1], vertices[xIndex + 2]));
		//cout << "tmpNormal: (" << tmpNormal.x << ", " << tmpNormal.y << ", " << tmpNormal.z << ")" << endl;
		//cout << "xIndex = " << xIndex << endl;
		normals[xIndex] = tmpNormal.x;
		normals[xIndex + 1] = tmpNormal.y;
		normals[xIndex + 2] = tmpNormal.z;
		//cout << "normals: (" << normals[xIndex] << ", " << normals[xIndex + 1] << ", " << normals[xIndex + 2] << ")" << endl;
	}
	*/

	return normals;
}

void makeRandomPartOfTerrainFlat() {
	cout << "Press F" << endl;
	int nrowsToFlatten = 10;
	int ncolsToFlatten = 10;
	//int randomStartingRow = ((nrows - 2 * nrowsToFlatten) * ((int)rand() / (RAND_MAX))) + nrowsToFlatten;
	//int randomStartingCol = ((ncols - 2 * ncolsToFlatten) * ((int)rand() / (RAND_MAX))) + ncolsToFlatten;
	int randomStartingRow = 120;
	//int randomStartingRow = rand() % ((nrows - nrowsToFlatten) - nrowsToFlatten + 1) + nrowsToFlatten;
	//int randomStartingCol = rand() % ((ncols - ncolsToFlatten) - ncolsToFlatten + 1) + ncolsToFlatten;
	int randomStartingCol = 120;
	int randomStartingPos = ((randomStartingRow * ncols) + randomStartingCol) * 3;
	cout << "randomStartingRow: " << randomStartingRow  <<  endl;
	cout << "randomStartingCol: " << randomStartingCol  << endl;
	int offsetX = 0;
	int offsetY = 0;
	
	for (int y = 0; y < nrowsToFlatten*2; y++) { // int y = randomStartingPos*3; y < randomStartingPos+5*3; y+=3 
		for (int x = 0; x < ncolsToFlatten*2; x++) { // int x = randomStartingPos*3; x < randomStartingPos+5*3; x+=3 
			offsetX = 3 * x;
			offsetY = (ncols-1) * y;
			vertices[randomStartingPos + 0 + offsetX + offsetY] = 0;
			vertices[randomStartingPos + 1 + offsetX + offsetY] = 0;
			vertices[randomStartingPos + 2 + offsetX + offsetY] = 0;
			
			//cout << "(" << randomStartingRow+y << ", " << randomStartingCol+x << ")" << endl;
			//cout << "randomStartingPos: " << randomStartingPos + 1 + offsetX + offsetY << endl;
			// vertices[(y*ncols)+x+1] = 0
		}
		//cout << "New row" << endl;
	}

	/*
	for (int y = -5; y < nrowsToFlatten+15; y++) { // int y = randomStartingPos*3; y < randomStartingPos+5*3; y+=3 
		cout << "-----------"<< y <<"------------" << endl;
		for (int x = -5; x < ncolsToFlatten+15; x++) { // int x = randomStartingPos*3; x < randomStartingPos+5*3; x+=3 
			offsetX = 3 * x;
			offsetY = (ncols-1) * y;
			cout << x <<": " << vertices[randomStartingPos + 1 + offsetX + offsetY] << ", ";
			// vertices[(y*ncols)+x+1] = 0
		}
		cout << endl;
	}
	*/

	cout << "Unpress F" << endl;
}

void randomWithMT() {
	random_device rd;   // non-deterministic generator
	mt19937 gen(rd());  // to seed mersenne twister.
	uniform_int_distribution<> dist(5, 251); // distribute results between 1 and 6 inclusive.

	for (int i = 0; i < 5; ++i) {
		cout << dist(gen) << " "; // pass the generator to the distribution.
	}
	cout << endl;
}

glm::vec3 calculateNormalForVertex(int row, int col, glm::vec3 centerVertex) {
	int tmpRow, tmpCol, posX;
	tmpRow = row;
	tmpCol = col;

	glm::vec3 above(0, 0, 0);
	glm::vec3 below(0, 0, 0);
	glm::vec3 left(0, 0, 0);
	glm::vec3 right(0, 0, 0);
	glm::vec3 normalAL(0, 0, 0);
	glm::vec3 normalBL(0, 0, 0);
	glm::vec3 normalAR(0, 0, 0);
	glm::vec3 normalBR(0, 0, 0);
	glm::vec3 normal(0, 0, 0);

	// AdjacentVertices
	if ((tmpRow - 1) >= 0) {
		// above
		posX = ((row - 1) * ncols + col) * 3;
		above = glm::vec3(vertices[posX], vertices[posX + 1], vertices[posX + 2]);
		//cout << "above: (" << above.x << ", " << above.y << ", " << above.z << ")" << endl;
	}
	if ((tmpRow + 1) <= nrows) {
		// below
		posX = ((row + 1) * ncols + col) * 3;
		below = glm::vec3(vertices[posX], vertices[posX + 1], vertices[posX + 2]);
		//cout << "below: (" << below.x << ", " << below.y << ", " << below.z << ")" << endl;
	}
	if ((tmpCol - 1) >= 0) {
		// left
		posX = ((row * ncols) + col - 1) * 3;
		left = glm::vec3(vertices[posX], vertices[posX + 1], vertices[posX + 2]);
		//cout << "left: (" << left.x << ", " << left.y << ", " << left.z << ")" << endl;
	}
	if ((tmpCol + 1) <= ncols) {
		// right
		posX = ((row * ncols) + col + 1) * 3;
		right = glm::vec3(vertices[posX], vertices[posX + 1], vertices[posX + 2]);
		//right = glm::vec3(vertices[(row * ncols + (ncols + 1))], vertices[(row * ncols + (ncols + 1)) + 1], vertices[(row * ncols + (ncols + 1)) + 2]);
		//cout << "right: (" << right.x << ", " << right.y << ", " << right.z << ")" << endl;
	}
	//cout << "After adjacent vertices" << endl;
	// NormalsForDirections
	if (above != glm::vec3(0, 0, 0) && left != glm::vec3(0, 0, 0)) {
		// normalAL
		normalAL = glm::normalize(glm::cross(above - centerVertex, left - centerVertex));
		//cout << "normalAL: (" << normalAL.x << ", " << normalAL.y << ", " << normalAL.z << ")" << endl;
	}
	if (below != glm::vec3(0, 0, 0) && left != glm::vec3(0, 0, 0)) {
		// normalBL
		normalBL = glm::normalize(glm::cross(below - centerVertex, left - centerVertex));
		//cout << "normalBL: (" << normalBL.x << ", " << normalBL.y << ", " << normalBL.z << ")" << endl;
	}
	if (above != glm::vec3(0, 0, 0) && right != glm::vec3(0, 0, 0)) {
		// normalAR
		normalAR = glm::normalize(glm::cross(above - centerVertex, right - centerVertex));
		//cout << "normalAR: (" << normalAR.x << ", " << normalAR.y << ", " << normalAR.z << ")" << endl;
	}
	if (below != glm::vec3(0, 0, 0) && right != glm::vec3(0, 0, 0)) {
		// normalBR
		normalBR = glm::normalize(glm::cross(below - centerVertex, right - centerVertex));
		//cout << "normalBR: (" << normalBR.x << ", " << normalBR.y << ", " << normalBR.z << ")" << endl;
	}

	// NormalForCenterVertex
	if (normalAL != glm::vec3(0, 0, 0)) {
		//cout << "normalAL" << endl;
		normal.x += normalAL.x;
		normal.y += normalAL.y;
		normal.z += normalAL.z;
	}
	if (normalAR != glm::vec3(0, 0, 0)) {
		//cout << "normalAR" << endl;
		normal.x += normalAR.x;
		normal.y += normalAR.y;
		normal.z += normalAR.z;
	}
	if (normalBL != glm::vec3(0, 0, 0)) {
		//cout << "normalBL" << endl;
		normal.x += normalBL.x;
		normal.y += normalBL.y;
		normal.z += normalBL.z;
	}
	if (normalBR != glm::vec3(0, 0, 0)) {
		//cout << "normalBR" << endl;
		normal.x += normalBR.x;
		normal.y += normalBR.y;
		normal.z += normalBR.z;
	}

	//cout << "normal nr: " << ((row * ncols) + col) / 3 << endl;
	//cout << "normal: (" << normal.x << ", " << normal.y << ", " << normal.z << ")" << endl;
	
	return normal;
}

glm::vec3 calculateNormalForVertex2(int xIndex, glm::vec3 centerVertex) {
	int tmpRow, tmpCol, posX;
	tmpRow = (int)(((xIndex/3)/(nrows)));
	tmpCol = (xIndex/3)%ncols;

	cout << "centerVertexPos: (" << tmpRow << ", " << tmpCol << ")" << endl;

	glm::vec3 above(0, 0, 0);
	glm::vec3 below(0, 0, 0);
	glm::vec3 left(0, 0, 0);
	glm::vec3 right(0, 0, 0);
	glm::vec3 normalAL(0, 0, 0);
	glm::vec3 normalBL(0, 0, 0);
	glm::vec3 normalAR(0, 0, 0);
	glm::vec3 normalBR(0, 0, 0);
	glm::vec3 normal(0, 0, 0);

	// AdjacentVertices
	if ((tmpRow - 1) >= 0) {
		// above
		posX = ((tmpRow - 1) * ncols + tmpCol) * 3;
		above = glm::vec3(vertices[posX], vertices[posX + 1], vertices[posX + 2]);
		//cout << "above: (" << above.x << ", " << above.y << ", " << above.z << ")" << endl;
	}
	if ((tmpRow + 1) <= nrows) {
		// below
		posX = ((tmpRow + 1) * ncols + tmpCol) * 3;
		below = glm::vec3(vertices[posX], vertices[posX + 1], vertices[posX + 2]);
		//cout << "below: (" << below.x << ", " << below.y << ", " << below.z << ")" << endl;
	}
	if ((tmpCol - 1) >= 0) {
		// left
		posX = ((tmpRow * ncols) + tmpCol - 1) * 3;
		left = glm::vec3(vertices[posX], vertices[posX + 1], vertices[posX + 2]);
		//cout << "left: (" << left.x << ", " << left.y << ", " << left.z << ")" << endl;
	}
	if ((tmpCol + 1) <= ncols) {
		// right
		posX = ((tmpRow * ncols) + tmpCol + 1) * 3;
		right = glm::vec3(vertices[posX], vertices[posX + 1], vertices[posX + 2]);
		//right = glm::vec3(vertices[(tmpRow * ncols + (ncols + 1))], vertices[(tmpRow * ncols + (ncols + 1)) + 1], vertices[(tmpRow * ncols + (ncols + 1)) + 2]);
		//cout << "right: (" << right.x << ", " << right.y << ", " << right.z << ")" << endl;
	}
	//cout << "After adjacent vertices" << endl;
	// NormalsForDirections
	if (above != glm::vec3(0, 0, 0) && left != glm::vec3(0, 0, 0)) {
		// normalAL
		normalAL = glm::normalize(glm::cross(above - centerVertex, left - centerVertex));
		//cout << "normalAL: (" << normalAL.x << ", " << normalAL.y << ", " << normalAL.z << ")" << endl;
	}
	if (below != glm::vec3(0, 0, 0) && left != glm::vec3(0, 0, 0)) {
		// normalBL
		normalBL = glm::normalize(glm::cross(below - centerVertex, left - centerVertex));
		//cout << "normalBL: (" << normalBL.x << ", " << normalBL.y << ", " << normalBL.z << ")" << endl;
	}
	if (above != glm::vec3(0, 0, 0) && right != glm::vec3(0, 0, 0)) {
		// normalAR
		normalAR = glm::normalize(glm::cross(above - centerVertex, right - centerVertex));
		//cout << "normalAR: (" << normalAR.x << ", " << normalAR.y << ", " << normalAR.z << ")" << endl;
	}
	if (below != glm::vec3(0, 0, 0) && right != glm::vec3(0, 0, 0)) {
		// normalBR
		normalBR = glm::normalize(glm::cross(below - centerVertex, right - centerVertex));
		//cout << "normalBR: (" << normalBR.x << ", " << normalBR.y << ", " << normalBR.z << ")" << endl;
	}

	// NormalForCenterVertex
	if (normalAL != glm::vec3(0, 0, 0)) {
		//cout << "normalAL" << endl;
		normal.x += normalAL.x;
		normal.y += normalAL.y;
		normal.z += normalAL.z;
	}
	if (normalAR != glm::vec3(0, 0, 0)) {
		//cout << "normalAR" << endl;
		normal.x += normalAR.x;
		normal.y += normalAR.y;
		normal.z += normalAR.z;
	}
	if (normalBL != glm::vec3(0, 0, 0)) {
		//cout << "normalBL" << endl;
		normal.x += normalBL.x;
		normal.y += normalBL.y;
		normal.z += normalBL.z;
	}
	if (normalBR != glm::vec3(0, 0, 0)) {
		//cout << "normalBR" << endl;
		normal.x += normalBR.x;
		normal.y += normalBR.y;
		normal.z += normalBR.z;
	}

	cout << "normal nr: " << ((tmpRow * ncols) + tmpCol) << endl;
	//cout << "normal: (" << normal.x << ", " << normal.y << ", " << normal.z << ")" << endl;

	return normal;
}

glm::vec3 calculateNormalForTriangle(int xIndex, glm::vec3 centerVertex) {
	int vertexNumber = (int)((xIndex / 3));
	glm::vec3 adjVertex(0, 0, 0), adjVertex2(0, 0, 0);
	glm::vec3 normal2(0, 0, 0);
	
	//cout << "Try calculate normal for triangle" << endl;
	//cout << "xIndex: " << (int)((xIndex / 3)) << endl;
	if (vertexNumber % ncols != ncols - 1) {
		adjVertex = glm::vec3(vertices[xIndex + 3], vertices[xIndex + 4], vertices[xIndex + 5]);
		adjVertex2 = glm::vec3(vertices[xIndex + (3 * ncols)], vertices[xIndex + (3 * ncols) + 1], vertices[xIndex + (3 * ncols) + 2]);
	}
	else {
		//if (vertices[xIndex] == (ncols - 1)) {
			//cout << "if == ncols - 1: " << xIndex << endl;
		//cout << "vertex below left center: " << (xIndex + (3 * ncols) - 3) / 3 << endl;
		//cout << "vertex below center: " << (xIndex + (3 * ncols)) / 3  << endl;
			adjVertex = glm::vec3(vertices[xIndex + (3 * ncols) - 3], vertices[xIndex + (3 * ncols) - 3 + 1], vertices[xIndex + (3 * ncols) - 3 + 2]);
			adjVertex2 = glm::vec3(vertices[xIndex + (3 * ncols)], vertices[xIndex + (3 * ncols) + 1], vertices[xIndex + (3 * ncols) + 2]);
			//cout << "adjVertex: (" << adjVertex.x << ", " << adjVertex.y << ", " << adjVertex.z << ")" << endl;
			//cout << "adjVertex2: (" << adjVertex2.x << ", " << adjVertex2.y << ", " << adjVertex2.z << ")" << endl;
		//}
	}

	glm::vec3 normal = glm::normalize(glm::cross(adjVertex - centerVertex, adjVertex2 - centerVertex));
	//cout << "normalForTriangle: (" << normal.x << ", " << normal.y << ", " << normal.z << ")" << endl;

	return normal;
}