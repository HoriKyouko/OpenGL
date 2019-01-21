// OpenGL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "OpenGL.h"

#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <string>

#define PI  3.14159265358979323846

using namespace std;

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
};
struct Camera {
	glm::vec3 viewPoint;
	glm::vec3 atPoint;
	glm::vec3 up;
	int fov;
	int aspectRatio;
};

void setupCameraCoordinateSystem(Camera&, glm::vec3&, glm::vec3&, glm::vec3&);
void settingUpValues(Camera&, glm::vec3&, glm::vec3&, glm::vec3&, int&, int&, int&, float&, float&, float&, float&);
void buildRayBundle(Ray&, glm::vec3&, glm::vec3&, glm::vec3&, int, int, int, float, float, float*);
void createOutputFile(int , float*);
Camera setupCamera();

int main()
{
	Camera cam = setupCamera();
	Ray trace;
	glm::vec3 u, v, w, CV;
	float pixelWidth, pixelHeight, width, height, *output;
	int rows, cols, sizeOfArray;
	// Setting our Ray's origin to be the eyePoint or viewPoint.
	trace.origin = cam.viewPoint;
	// Setting up camera space.
	setupCameraCoordinateSystem(cam, w, u, v);
	// Setting up variables we need.
	settingUpValues(cam, CV, v, u, rows, cols, sizeOfArray, height, width, pixelWidth, pixelHeight);
	// Creates our output Array.
	output = new float[sizeOfArray];
	// Fills our output Array with the Ray Bundles we want.
	buildRayBundle(trace, CV, u, v, rows, cols, 0, pixelWidth, pixelHeight, output);
	// Creates our .json file with the values from our output array.
	createOutputFile(sizeOfArray, output);
	delete[] output;
	return 0;
}

float dot(glm::vec3 a, glm::vec3 b) {
	return (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
}

float length(glm::vec3 a) {
	return sqrt((a.x * a.x) + (a.y * a.y) + (a.z*a.z));
}

glm::vec3 cross(glm::vec3 a, glm::vec3 b) {
	return glm::vec3((a.y*b.z - a.z*b.y), (a.z*b.x - a.x*b.z), (a.x*b.y - a.y*b.x));
}

glm::vec3 normalize(glm::vec3 a) {
	float len = length(a);
	return glm::vec3((a.x / len), (a.y / len), (a.z / len));
}

void insertIntoArray(float *output, int &count, Ray &trace)
{
	// Puts in the origin first.
	output[count] = trace.origin.x;
	output[count + 1] = trace.origin.y;
	output[count + 2] = trace.origin.z;
	// Puts in the direction second.
	output[count + 3] = trace.direction.x;
	output[count + 4] = trace.direction.y;
	output[count + 5] = trace.direction.z;
	count += 6;
}

void buildRayBundle(Ray &trace, glm::vec3 &CV, glm::vec3 &u, glm::vec3 &v, int rows, int cols, int count, float pixelWidth, float pixelHeight,  float *output)
{
	// looping through the rows and cols
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			float tempW = (float)(i + .5)*pixelWidth;
			float tempH = (float)(j + .5)*pixelHeight;
			trace.direction = (CV + tempW * u - tempH * v);

			// Remove the next line comment to get the professors output.
			//trace.direction /= 2;
			insertIntoArray(output, count, trace);

			// normalize it here, but we don't have to do in this homework assignment.
		}
	}
}

void settingUpValues(Camera &cam, glm::vec3 &CV, glm::vec3 &v, glm::vec3 &u, int &rows, int &cols, int &sizeOfArray, float &height, float &width, float &pixelWidth, float &pixelHeight)
{
	// Sets our rows and cols size.
	rows = cols = 32;
	// Sets our height and width based on the camera fov and aspect ratio
	height = 2 * tan((cam.fov * (float)PI / 180) / 2);
	width = height * cam.aspectRatio;
	// sets our pixelWidth and pixelHeight based on the width and height divided by the number of cols and rows respectively.
	pixelWidth = width / cols;
	pixelHeight = height / rows;
	// Finds our Corner Vector based on the camera Coordinate System and the height and width.
	CV = (v * (height / 2)) - (u * (width / 2));
	// Sets our array size based on how many outputs we'll need per Ray.
	sizeOfArray = 6 * cols * rows;
}

void setupCameraCoordinateSystem(Camera &cam, glm::vec3 &w, glm::vec3 &u, glm::vec3 &v)
{
	// Creates our u,v,w axis based on where the camera is looking.
	w = (cam.viewPoint - cam.atPoint) / length(cam.viewPoint - cam.atPoint);
	u = cross(cam.up, w) / length(cross(cam.up, w));
	v = cross(w, u);
}

void createOutputFile(int sizeOfArray, float *output)
{
	string myString = "{\"rayBundles\":[[";
	ostringstream os;
	for (int i = 0; i < sizeOfArray; i++) {
		os << output[i];
		if (i != sizeOfArray - 1)
			os << ",";
		else
			os << "]]}";
	}
	myString.append(os.str());
	cout << "Printing out the .json file for you..." << endl;
	ofstream out("raybundle.json");
	out << myString;
	out.close();
}

Camera setupCamera() {
	Camera cam;
	cam.viewPoint = glm::vec3(1.0f, 0.0f, 0.0f);
	cam.atPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	cam.up = glm::vec3(0.0f, 1.0f, 0.0f);
	cam.fov = 45;
	cam.aspectRatio = 1;
	return cam;
}
