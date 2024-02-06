#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <algorithm>

#include "globalVariables.h"

/*
--------------OPIS PLIKU WEJŒCIOWEGO--------------
 1. Pierwsze 2 linie to odpowiednio iloœæ kolumn i wierszy
 2. Pierwsze 2 linie zaczynaj¹ siê od s³ów: nrows i ncols, zawieraj¹ odpowiednio informacje o iloœci rzêdów (wsp. X) i kolumn (wsp. Y)
 3. Reszta linii zawiera tylko informacje dotycz¹ce elewacji
 4. Dane s¹ u³o¿one poziomo (wsp. X to rzêd, wsp. Y to kolumny)
 5. Oddzielone s¹ spacjami
*/

//#define DEBUGGINGFILE
#define LISTINGFILE

/* 
TO DO:
	1. Dodaæ #if def

	#if defined DEBUGGINGFILE
	{
		// some code
	}
	#endif
*/

using namespace std;

int stepForColsAndRows = 1;
float multiplierOfColsAndRows = 1;
size_t startPos = 0, endPos = 0;
fstream elevDataStream;
string path = "draw_data/tatry2.asc";
string pathGore = "draw_data/draw2.asc";
string pathValdez = "draw_data/draw3.asc";
enum FileNames
{
	tatry,			//tatry2
	gore,			//draw2
	valdez,			//draw3
	invalidName
};

//--------------------------------Funkcje pomocniczne - Nag³ówki--------------------------------
int getNcols();
int getNrows();
void setDrawDataPath(int option);
FileNames resolveFileName(string input);

//-----------------------------------Funkcje g³ówne - Nag³ówki----------------------------------
vector<float> getElevDataFromFile();
vector<float> normalize(vector<float> elevData, float oldMin, float oldMax, int newMin, int newMax);
void loadParamsFromFile();
float* loadVertices();

//-----------------------------Funkcje pomocniczne - Cia³o funkcji------------------------------
int getNcols() {
	loadParamsFromFile();
	return ncols;
}

int getNrows() {
	loadParamsFromFile();
	return nrows;
}

void setDrawDataPath(int option) {
	switch (option) {
	case 1:
		cout << "path: " << path << endl;
		break;
	case 2:
		path = pathGore;
		cout << "path: " << path << endl;
		break;
	case 3:
		path = pathValdez;
		cout << "path: " << path << endl;
		break;
	default:
		cout << "There is not such area to draw!" << endl;
		exit;
		break;
	}
}

FileNames resolveFileName(string input) {
	if (input == "tatry2.asc") return tatry;
	if (input == "draw2.asc") return gore;
	if (input == "draw3.asc") return valdez;

	return invalidName;
}

//--------------------------------Funkcje g³ówne - Cia³o funkcji--------------------------------
void loadParamsFromFile() {
	int lineNum = 1;
	string token = "", data = "", delim = " ";
	size_t startPosFileName = 10;
	elevDataStream.open(path, ios::in);

	while (getline(elevDataStream, data) && lineNum <= 2)
	{
		endPos = data.find(delim);
		startPos = endPos + delim.length();
		token = data.substr(startPos, data.length() - startPos);

		switch (lineNum)
		{
		case 1:
			ncols = stoi(token);
			break;
		case 2:
			nrows = stoi(token);
			break;
		default:
			break;
		}

		lineNum++;
	}

	string drawDataFileName = path.substr(startPosFileName, path.length() - startPosFileName);
	switch (resolveFileName(drawDataFileName)) {
	case tatry:
		multiplierOfColsAndRows = 1;
		maxNormalizedHeight = 10;
		break;
	case gore:
		multiplierOfColsAndRows = 0.5;
		stepForColsAndRows = 2;
		maxNormalizedHeight = 20;
		break;
	case valdez:
		multiplierOfColsAndRows = 0.5;
		stepForColsAndRows = 2;
		maxNormalizedHeight = 20;
		break;
	default:
		cout << "WARNING: something wrong with preparing multiplier for cols and rows!" << endl;
		break;
	}

	ncols /= stepForColsAndRows;
	nrows /= stepForColsAndRows;

	elevDataStream.close();
}

vector<float> getElevDataFromFile() {
	int counter = 1, lineNum = 1;
	float tmp = 0.0;
	string token = "", data = "", delim = " ";
	vector<float> elevData;

	while (getline(elevDataStream, data) && lineNum <= ((nrows * stepForColsAndRows) + 2))
	{
		if (lineNum > 2) {
			startPos = 0;
			endPos = data.find(delim);
			counter = 1;
			do {
				token = data.substr(startPos, endPos - startPos);
				startPos = endPos + delim.length();
				endPos = data.find(delim, startPos);
				if (counter % stepForColsAndRows == 0) {
					tmp = stof(token.c_str());
					elevData.push_back(tmp);
				}
				counter++;
			} while (counter <= (ncols * stepForColsAndRows));
		}
		lineNum++;
	}

	return elevData;
}

vector<float> normalize(vector<float> elevData, float oldMin, float oldMax, int newMin, int newMax) {
	float normalizedValue = 0;
	for (int i = 0; i < elevData.size(); i++) {
		if (elevData[i] == -9999) { // -9999 <- NO_DATA_VALUE
			elevData[i] = 0;
		}
		else {
			normalizedValue = (((elevData[i] - oldMin) / (oldMax - oldMin)) * (newMax - (newMin))) + (newMin);
			elevData[i] = normalizedValue;
		}
	}

	//x' = (((x - min)/(max - min)) * (newmax-newmin)) + newmin
	return elevData;
}

float* loadVertices() {
	vector<float> elevData;
	elevDataStream.open(path, ios::in);

	if (elevDataStream.is_open()) {
		elevData = getElevDataFromFile();

		elevDataStream.close();
		
		#if defined LISTINGFILE
		{
			cout << "nrows: " << nrows << " ncols: " << ncols << " size: " << elevData.size() << endl;
		}
		#endif

		#if defined DEBUGGINGFILE
		{
			for (int i = 0; i < elevData.size(); i++) {
				//cout << "elevData from file at pos " << i << " = " << elevData[i] << endl;
			}
		}
		#endif
		
		float elevDataMin = *min_element(elevData.begin(), elevData.end());
		float elevDataMax = *max_element(elevData.begin(), elevData.end());
	
		#if defined DEBUGGINGFILE
		{
			//cout << "elevData from file min: " << *min_element(elevData.begin(), elevData.end()) << endl;
			//cout << "elevData from file max: " << *max_element(elevData.begin(), elevData.end()) << endl;
		}
		#endif

		vector<float> normalizedElevData = normalize(elevData, elevDataMin, elevDataMax, 0, maxNormalizedHeight);

		verticesSize = 3 * elevData.size();
		vertices = new float[verticesSize];
		int vertexIndex = 0;
		float mulipliedRow, multipliedCol;
		
		//for (int row = 0; row < nrows; row++) {
		for (int row = nrows-1; row >= 0; row--) {
			for (int col = 0; col < ncols; col++) {
				mulipliedRow = row * multiplierOfColsAndRows;
				multipliedCol = col * multiplierOfColsAndRows;

				vertices[vertexIndex] = multipliedCol;
				//vertices[vertexIndex] = col;
				
				//vertices[vertexIndex + 1] = elevData[(row * ncols) + col];
				vertices[vertexIndex + 1] = normalizedElevData[(row * ncols) + col]; 

				vertices[vertexIndex + 2] = mulipliedRow;
				//vertices[vertexIndex + 2] = row;
				
				#if defined DEBUGGINGFILE
				{
					cout << "Vertex nr " << vertexIndex << " = ";
					cout << "(" << vertices[vertexIndex] << ", ";
					cout << vertices[vertexIndex+1] << ", ";
					cout << vertices[vertexIndex+2] << ")" << endl;
				}
				#endif

				vertexIndex = vertexIndex + 3;
			}
		}
	}
	else {
		cout << "WARRNING: Cannot open !" << path << endl;
	}

	return vertices;
}