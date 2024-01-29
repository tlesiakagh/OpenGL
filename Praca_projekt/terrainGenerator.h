#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <random>

#include "globalVariables.h"

//#define DEBUGGINGDIAMONDSQUARE
//#define LISTINGDIAMONDSQUARE
//#define DEBUGRAND


// RNG functions
//#define MT
#define RAND

/*
	#if defined DEBUGGINGDIAMONDSQUARE
	{

	}
	#endif
*/

using namespace std;

int maxRandomValue;
int squareCounter, diamondCounter;
bool isRandomRangeSymmetrical = false;
bool sameValueInCorners = false;
float elevData[800000];

//-----------------------------------DiamondSquare - Nag³ówki-----------------------------------
float sample(int x, int y);
void setSample(int x, int y, float value);
void sampleSquare(int x, int y, int stepSize, float value);
void sampleDiamond(int x, int y, int stepSize, float value);
void DiamondSquare(int nrows, int ncols, int stepSize, float scale);

//--------------------------------Funkcje pomocniczne - Nag³ówki--------------------------------
void setMaxRandomValue(int value);
void setIfRandomRangeSymmetrical(bool randomRangeSymmetrical);
float randomFromRange(int maxValue, int option);
void initGenerator(int startingStepSize);
void prepareElevationDataWithDiamondSquare(int startingStepSize);
float findMinMax(int flag);
void normalizeDiamondSquare(float oldMin, float oldMax, int newMin, int newMax);
void showElevDataAsMatrix();
void fillVerticesArray();

//-----------------------------Funkcje pomocniczne - Cia³o funkcji------------------------------
void initGenerator(int startingStepSize) {
	//setIfRandomRangeSymmetrical(isRandomRangeSymmetrical);
	//setMaxRandomValue(maxRandomValue);

	//srand(time(NULL));
	float randomValue = 0.0;
	// <<-- W poni¿szym for siê wywala
	//string trash;
	for (int y = 0; y < nrows; y++) {
		for (int x = 0; x < ncols; x++) {
			#if defined DEBUGGINGDIAMONDSQUARE
			{
				//cout << "(" << x << ", " << y << ")" << endl;
			}
			#endif
			setSample(x, y, 0);
		}
	}
	//cout << "size: " << nrows * ncols << endl;
	//cin >> trash;

	//randomValue = randomFromRange(maxRandomValue/2, 0);
	randomValue = randomFromRange(maxRandomValue, 0);
	for (int y = 0; y < nrows; y += startingStepSize) {
		for (int x = 0; x < ncols; x += startingStepSize) {
			if (sameValueInCorners) {
				setSample(x, y, randomValue);
			}
			else {
				//setSample(x, y, randomFromRange(maxRandomValue/2, 0));
				setSample(x, y, randomFromRange(maxRandomValue, 0));
			}
		}
	}
}

void prepareElevationDataWithDiamondSquare(int startingStepSize) {
	int currentStepSize = startingStepSize;
	int step = 1;
	float scale = 1.0;
	int roughness = maxRandomValue;
	// W niektórych implementacjach wykorzystuj¹ roughness zamiast scale
	// W przypadku roughness liczbê dodawan¹ do œredniej z 4 punktów losuj¹ z przedzia³u (-roughness, roughness), zamiast RANDOM * scale
	string trash;
	while (currentStepSize > 1)
	{
		// cout << "-----------------------" << step << "-----------------------" << endl;
		//DiamondSquare(nrows, ncols, currentStepSize, scale);
		DiamondSquare(nrows, ncols, currentStepSize, roughness);

		currentStepSize /= 2;
		//scale /= 2.0;
		roughness /= 2;
		if (roughness == 0) {
			roughness = 1;
		}
		setMaxRandomValue(roughness);
		//cout << "maxRandomValue: " << maxRandomValue << ", at step " << step << endl;
		//showValuesAsMatrix(nrows, ncols);

		//cout << roughness << endl;
		step++;
		#if defined DEBUGRAND
		{
			cout << "Input sth to move on...";
			cin >> trash;
		}
		#endif
	}

	//normalizeDiamondSquare(findMinMax(0), findMinMax(1), 0, 30);
	fillVerticesArray();
}

void setMaxRandomValue(int value) {
	maxRandomValue = value;
}

void setIfRandomRangeSymmetrical(bool randomRangeSymmetrical) {
	isRandomRangeSymmetrical = randomRangeSymmetrical;
}

float randomFromRange(int maxValue, int option) {
	random_device rd;   // non-deterministic generator
	mt19937 gen(rd());  // to seed mersenne twister.
	uniform_real_distribution<float> distNonSymmetric(0, maxValue); // distribute results between 1 and 6 inclusive.
	uniform_real_distribution<float> distSymmetric(-maxValue, maxValue); // distribute results between 1 and 6 inclusive.

	/*
	Symetryczny przedzia³ od :
	  - option=0, 0<->maxValue
	  - option=1, -maxValue<->maxValue
	*/

	float randomValue = 0;
	#if defined DEBUGGINGDIAMONDSQUARE
	{
		cout << "maxRandomValue: " << maxRandomValue << endl;
	}
	#endif
	switch (option) {
	case 0:
		#if defined MT
		{
			randomValue = distNonSymmetric(gen);
		}
		#endif	
		
		#if defined RAND
		{
			randomValue = maxValue * ((float)rand() / (RAND_MAX));
		}
		#endif
		break;
	case 1:
		#if defined MT
		{
			randomValue = distSymmetric(gen);
		}
		#endif

		#if defined RAND
		{
			randomValue = (2 * maxValue * ((float)rand() / (RAND_MAX))) - maxValue;
		}
		#endif
		break;
	default:
		cout << "There is not such range option!" << endl;
		exit;
		break;
	}
	#if defined DEBUGRAND
	{
		cout << "randomValueFromDiamondSquare: " << randomValue  << ", maxValue " << maxValue << endl;
	}
	#endif
	return randomValue;
}

float findMinMax(int flag) {
	float min = sample(0, 0);
	float max = sample(0, 0);
	float returnValue = 0;

	for (int y = 0; y < nrows; y++) {
		for (int x = 0; x < ncols; x++) {
			if (flag == 0) {
				if (sample(x, y) < min) {
					min = sample(x, y);
				}
			}
			else {
				if (sample(x, y) > max) {
					max = sample(x, y);
				}
			}
		}
	}

	if (flag == 0) {
		#if defined DEBUGGINGDIAMONDSQUARE
		{
			cout << "min: " << min << endl;
		}
		#endif
		returnValue = min;
	}
	else {
		#if defined DEBUGGINGDIAMONDSQUARE
		{
			cout << "max: " << max << endl;
		}
		#endif
		returnValue = max;
	}

	return returnValue;
}

void normalizeDiamondSquare(float oldMin, float oldMax, int newMin, int newMax) {
	float normalizedValue = 0;

	for (int y = 0; y < nrows; y++) {
		for (int x = 0; x < ncols; x++) {
			normalizedValue = (((sample(x, y) - oldMin) / (oldMax - oldMin)) * (newMax - (newMin))) + (newMin);
			//cout << "normalizedValue = " << normalizedValue << endl;
			//cout << "Set normalized value ";
			setSample(x, y, normalizedValue);
		}
	}
}

void fillVerticesArray() {
	int vertexIndex = 0;

	//for (int row = 0; row < nrows; row++) {
	for (int row = nrows - 1; row >= 0; row--) {
		for (int col = 0; col < ncols; col++) {
			vertices[vertexIndex] = col;
			vertices[vertexIndex + 1] = elevData[(row * ncols) + col];
			vertices[vertexIndex + 2] = row;

			#if defined DEBUGGINGDIAMONDSQUARE
			{
				cout << vertices[vertexIndex] << ", ";
				cout << vertices[vertexIndex + 1] << ", ";
				cout << vertices[vertexIndex + 2] << endl;
			}
			#endif
			vertexIndex = vertexIndex + 3;
		}
	}
}

void showElevDataAsMatrix() {
	for (int y = 0; y < nrows; y++) {
		for (int x = 0; x < ncols; x++) {
			cout << sample(x, y) << " ";
		}
		cout << endl;
	}
}

//--------------------------------DiamondSquare - Cia³o funkcji---------------------------------
void DiamondSquare(int nrows, int ncols, int stepSize, float scale)
{
	int hs = stepSize / 2;
	int xPlusHs, yPlusHs;
	//cout << "Entering DiamondSquare" << endl;
	for (int y = hs; y < nrows; y += stepSize)
	{
		for (int x = hs; x < ncols; x += stepSize)
		{
			squareCounter++;
			if (isRandomRangeSymmetrical) {
				sampleSquare(x, y, stepSize, randomFromRange(maxRandomValue, 1) /** scale*/);
			}
			else {
				sampleSquare(x, y, stepSize, randomFromRange(maxRandomValue, 0) /** scale*/);
			}	
		}
	}

	for (int y = 0; y < nrows; y += stepSize)
	{
		for (int x = 0; x < ncols; x += stepSize)
		{
			xPlusHs = x + hs;
			yPlusHs = y + hs;
			//cout << "pos -> (" << xPlusHs << ", " << y << ")" << endl;

			diamondCounter+=2;
			if (isRandomRangeSymmetrical) {
				sampleDiamond(xPlusHs, y, stepSize, randomFromRange(maxRandomValue, 1) /** scale*/);
				sampleDiamond(x, yPlusHs, stepSize, randomFromRange(maxRandomValue, 1) /** scale*/);
			}
			else {
				sampleDiamond(xPlusHs, y, stepSize, randomFromRange(maxRandomValue, 0) /** scale*/);
				sampleDiamond(x, yPlusHs, stepSize, randomFromRange(maxRandomValue, 0) /** scale*/);
			}
		}
	}
	//cout << "Exiting DiamondSquare" << endl;
}

void sampleSquare(int x, int y, int stepSize, float value)
{
	//cout << "Entering sampleSquare at step " << squareCounter << endl;

	//cout << "(" << x << ", " << y << ")" << endl;
	//cout << ncols << endl;
	int hs = stepSize / 2;

	// a     b
	//
	//    x
	//
	// c     d

	float a = sample(x - hs, y - hs);
	float b = sample(x + hs, y - hs);
	float c = sample(x - hs, y + hs);
	float d = sample(x + hs, y + hs);

	//cout << "scale: " << value << endl;
	//cout << "meanValue: " << ((a + b + c + d) / 4.0) << endl;
	//cout << "SquareValue: " << ((a + b + c + d) / 4.0) + value << endl;
	//cout << "(" << x << ", " << y << "), " << "(" << x - hs << ", " << y - hs << "): " << a << endl;
	//cout << "(" << x << ", " << y << "), " << "(" << x - hs << ", " << y + hs << "): " << b << endl;
	//cout << "(" << x << ", " << y << "), " << "(" << x + hs << ", " << y + hs << "): " << d << endl;
	//cout << "(" << x << ", " << y << "), " << "(" << x + hs << ", " << y - hs << "): " << c << endl;

	setSample(x, y, ((a + b + c + d) / 4.0) + value);

	//cout << "Exiting sampleSquare at step " << squareCounter << endl;
}

void sampleDiamond(int x, int y, int stepSize, float value)
{
	//cout << "Entering sampleDiamond at step " << diamondCounter << endl;
	int hs = stepSize / 2;

	//   c
	//
	//a  x  b
	//
	//   d

	float a = sample(x - hs, y);
	float b = sample(x + hs, y);
	float c = sample(x, y - hs);
	float d = sample(x, y + hs);
	//cout << "(" << x << ", " << y << "), " << "(" << x - hs << ", " << y - hs << "): " << a << endl;
	//cout << "(" << x << ", " << y << "), " << "(" << x - hs << ", " << y + hs << "): " << b << endl;
	//cout << "(" << x << ", " << y << "), " << "(" << x + hs << ", " << y + hs << "): " << d << endl;
	//cout << "(" << x << ", " << y << "), " << "(" << x + hs << ", " << y - hs << "): " << c << endl;
	//cout << "DiamondValue: " << ((a + b + c + d) / 4.0) + value << endl;

	setSample(x, y, ((a + b + c + d) / 4.0) + value);
	//cout << "Exiting sampleDiamond at step " << diamondCounter << endl;
}

float sample(int x, int y)
{
	float sampleValue = 0;
	//cout << "(" << x << ", " << y << ") ";
	
	if ((x >= 0 && x <= ncols - 1) && (y >= 0 && y <= nrows - 1)) {
		sampleValue = elevData[(y * ncols) + x];
		//cout << "In range!" << endl;
	}
	else {
		//sampleValue = elevData[(y * ncols) + x];
		//cout << "sampleValue: " << sampleValue << endl;
		//cout << "Not in range!" << endl;
	}
	//cout << "After sample if" << endl;
	return sampleValue;

	//cout << "function -> sample, " << x << ", " << y << "; pos: (" << ((x % ncols) < 0 ? ncols + x : x % ncols) << ", " << ((y % nrows) < 0 ? nrows + y : y % nrows) << ")" << endl;
	//return elevData[((x % ncols) < 0 ? ncols + x : x % ncols) + ((y % nrows) < 0 ? nrows + y : y % nrows) * ncols];
}

void setSample(int x, int y, float value)
{
	if ((x >= 0 && x <= ncols - 1) && (y >= 0 && y <= nrows - 1)) {
		//cout << "Setting data at pos: (" << x << ", " << y << ")" << endl;
		//cout << "Setting data at pos: " << (y * ncols) + x << endl;
		elevData[(y * ncols) + x] = value;
		//cout << "elevData: " << elevData[(y * ncols) + x] << endl;
	}
	
	//cout << "function -> setSample, " << x << ", " << y << "; pos: (" << ((x % ncols) < 0 ? ncols + x : x % ncols) << ", " << ((y % nrows) < 0 ? nrows + y : y % nrows) << ")" << endl;
	//elevData[((x % (ncols)) < 0 ? ncols + x : x % ncols) + ((y % (nrows)) < 0 ? nrows + y : y % nrows) * ncols] = value;
}