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
struct Sphere {
	glm::vec3 center;
	float radius;
};

void setupCameraCoordinateSystem(Camera&, glm::vec3&, glm::vec3&, glm::vec3&);
void settingUpValues(Camera&, glm::vec3&, glm::vec3&, glm::vec3&, glm::vec3&, int&, int&, float&, float&, float&, float&);
void buildRayBundle(Sphere[], Ray&, glm::vec3&, glm::vec3&, glm::vec3&, int, int, float, float);
Camera setupCamera();

int main()
{
	Camera cam = setupCamera();
	Ray trace;
	Sphere spheres[26] = {
		{glm::vec3(2.06, -2.26, 0.12), 2},
		{glm::vec3(-0.35, 1.18, -0.41), 2},
		{glm::vec3(2.54, -0.98, 0.73), 2},
		{glm::vec3(2.48, -1.21, 2.26), 2},
		{glm::vec3(1.78, -2.56, 2.44), 2},
		{glm::vec3(2.17, -3.31, 1.16), 2},
		{glm::vec3(1.72, 0.19, 0.32), 2},
		{glm::vec3(2.61, -2.63, -1.2), 2},
		{glm::vec3(2.28, 1.49, 0.33), 2},
		{glm::vec3(0.36, 0.08, -0.06), 2},
		{glm::vec3(1.52, 2.61, -0.03), 2},
		{glm::vec3(0.18, 2.42, -0.41), 2},
		{glm::vec3(3.58, -0.82, 0.43), 1},
		{glm::vec3(3.5, -1.27, 2.66), 1},
		{glm::vec3(1.95, -0.42, 2.81), 1},
		{glm::vec3(2.11, -3.08, 3.34), 1},
		{glm::vec3(0.69, -2.42, 2.48), 1},
		{glm::vec3(3.19, -3.69, 1.24), 1},
		{glm::vec3(1.48, -4.14, 0.96), 1},
		{glm::vec3(3.69, -2.8, -1.15), 1},
		{glm::vec3(2.42, -1.82, -1.91), 1},
		{glm::vec3(2.13, -3.53, -1.58), 1},
		{glm::vec3(3.26, 1.63, 0.62), 1},
		{glm::vec3(-0.1, -0.82, -0.07), 1},
		{glm::vec3(1.93, 3.54, -0.02), 1},
		{glm::vec3(-0.4, 3.22, -0.68), 1} };

	// Create Array of Struct Spheres with the spheres manually inputted.
	/*Sphere spheres[3] = {	{glm::vec3(-4, 0, 0), 1},
							{glm::vec3(-5.06, 1.06, 0), .5},
							{glm::vec3(-5.06, -1.06, 0), .5}};*/
	glm::vec3 u, v, w, CV;
	float pixelWidth, pixelHeight, width, height;
	int rows, cols;
	// Setting our Ray's origin to be the eyePoint or viewPoint.
	trace.origin = cam.viewPoint;
	// Setting up camera space.
	setupCameraCoordinateSystem(cam, w, u, v);
	// Setting up variables we need.
	settingUpValues(cam, CV, w, v, u, rows, cols, height, width, pixelWidth, pixelHeight);
	// Fills our output Array with the Ray Bundles we want.
	buildRayBundle(spheres, trace, CV, u, v, rows, cols, pixelWidth, pixelHeight);
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

float negativeQuadFormula(float A, float B, float C) {
	return (-B - sqrt((B*B) - 4 * A*C)) / (2 * A);
}

float positiveQuadFormula(float A, float B, float C) {
	return (-B + sqrt((B*B) - 4 * A*C)) / (2 * A);
}

void buildRayBundle(Sphere spheres[], Ray &trace, glm::vec3 &CV, glm::vec3 &u, glm::vec3 &v, int rows, int cols, float pixelWidth, float pixelHeight)
{
	string myString = "{\"rayBundles\":[[";
	ostringstream os;
	ostringstream os2;

	bool first = true;
	// looping through the rows and cols
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			float tempW = (float)(i + .5)*pixelWidth;
			float tempH = (float)(j + .5)*pixelHeight;
			trace.direction = (CV + tempW * u - tempH * v);			
			
			for (int k = 0; k < 26; k++) {
				// equation for ray-sphere interstection is:
				// t^2 * (d o d) + t * 2 * (O - C) o d + ((O - C) o (O - C) - R^2) = 0
				// find t using quadratic equation:
				// t = (-B +- sqrt(B^2 - 4 * A * D)) / 2 * A
				// A = d o d
				// B = 2(O o d)
				// C = (O o O - R^2)
				float A = dot(trace.direction, trace.direction);
				float B = 2 * (dot(trace.origin - spheres[k].center, trace.direction));
				float C = dot(trace.origin - spheres[k].center, trace.origin - spheres[k].center) - (spheres[k].radius * spheres[k].radius);

				if ((pow(B, 2) - 4 * A * C) <= 0)
					continue;

				float t1 = negativeQuadFormula(A, B, C);
				float t2 = positiveQuadFormula(A, B, C);
				// Intersection Rays.
				glm::vec3 P1, P2;
				if (abs(t1) < abs(t2)) {
					P1 = trace.origin + (t1 * trace.direction);
					P2 = trace.origin + (t2 * trace.direction);
				}
				else {
					P1 = trace.origin + (t2 * trace.direction);
					P2 = trace.origin + (t1 * trace.direction);
				}
				// Output the intersection points to our outputstream.
				if (first) {
					os2 << P1.x << "," << P1.y << "," << P1.z << "," << P2.x << "," << P2.y << "," << P2.z;
					first = false;
				}
				else {
					os2 << "," << P1.x << "," << P1.y << "," << P1.z << "," << P2.x << "," << P2.y << "," << P2.z;
				}
			}
			// P = o + t*d Primary Rays
			glm::vec3 P = trace.origin + trace.direction;
			// Output the origin and direction points to our outputstream.
			if(i+1 == rows && j+1 == cols)
				os << trace.origin.x << "," << trace.origin.y << "," << trace.origin.z << "," << P.x << "," << P.y << "," << P.z;
			else
				os << trace.origin.x << "," << trace.origin.y << "," << trace.origin.z << "," << P.x << "," << P.y << "," << P.z << ",";

			// normalize it here, but we don't have to do in this homework assignment.
		}
	}
	os << "],[";
	os2 << "]]}";
	myString.append(os.str());
	myString.append(os2.str());
	cout << "Printing out the .json file for you..." << endl;
	ofstream out("raybundleS.json");
	out << myString;
	out.close();
}

void settingUpValues(Camera &cam, glm::vec3 &CV, glm::vec3 &w, glm::vec3 &v, glm::vec3 &u, int &rows, int &cols, float &height, float &width, float &pixelWidth, float &pixelHeight)
{
	// Sets our rows and cols size.
	rows = cols = 128;
	// Sets our height and width based on the camera fov and aspect ratio
	height = 2 * tan((cam.fov * (float)PI / 180) / 2);
	width = height * cam.aspectRatio;
	// sets our pixelWidth and pixelHeight based on the width and height divided by the number of cols and rows respectively.
	pixelWidth = width / cols;
	pixelHeight = height / rows;
	// Finds our Corner Vector based on the camera Coordinate System and the height and width.
	CV = -w + (v * (height / 2)) - (u * (width / 2));
}

void setupCameraCoordinateSystem(Camera &cam, glm::vec3 &w, glm::vec3 &u, glm::vec3 &v)
{
	// Creates our u,v,w axis based on where the camera is looking.
	w = (cam.viewPoint - cam.atPoint) / length(cam.viewPoint - cam.atPoint);
	u = cross(cam.up, w) / length(cross(cam.up, w));
	v = cross(w, u);
}

Camera setupCamera() {
	Camera cam;
	cam.viewPoint = glm::vec3(1.6450000256299973, -0.2999999523162842, 8.336749156440431);
	cam.atPoint = glm::vec3(1.6450000256299973, -0.2999999523162842, 0.7149999737739563);
	cam.up = glm::vec3(0.0f, 1.0f, 0.0f);
	cam.fov = 90;
	cam.aspectRatio = 1;
	return cam;
}
