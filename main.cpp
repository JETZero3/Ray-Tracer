#include "SceneObject.h"
#include "RgbImage.h"
#include <stdlib.h>
#include <iostream>

#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include "glm/glm/glm.hpp"
#include "glm/glm/gtx/transform.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"	

#include <iostream>
#include <string>

#define IMAGE_WIDTH 400
#define IMAGE_HEIGHT 400
#define NUM_OBJECTS 8
#define NUM_LIGHTS 3
#define MAX_DEPTH 5
#define NO_OBJECT 0
#define OBJECT_HIT 1
#define LIGHT_HIT 2

using namespace std;

//Colors
glm::vec3 
	black(0.0f, 0.0f, 0.0f), 
	red(1.0f, 0.0f, 0.0f), 
	green(0.0f, 1.0f, 0.0f), 
	white(1.0f, 1.0f, 1.0f), 
	blue(0.0f, 0.0f, 1.0f), 
	grey(0.329f, 0.329f, 0.329f), 
	gold(1.0f, 0.843f, 0.0f), 
	sand(0.953125f, 0.643f, 0.376f),  
	pink(1.0f , 0.43f, 0.706f), 
	orange(1.0f, 0.647f, 0.0f),
	sky(0.529f, 0.808f, 0.9216f), 
	grass(0.172549f, 0.69f, 0.215686f), 
	purple(78/255, 0, 142/255);

//Plane normal
glm::vec3 
	plane_normal(1.0f, 0.0f, 0.0f);

//Triangle coordinates and normals
glm::vec3
	t1_v1(0.6f, -0.5f, 1.0f), t1_v2(0.2, -0.3f, 0.5f), t1_v3(0.9, 0.0f, 2.2f), 
	t2_v1(1.5f, 1.0f, 0.0f), t2_v2(1.0f, -1.0f, 0.0f), t2_v3(1.0f, 0.0f, 0.2f),
	t3_v1(0.4f, 0.5f, 0.0f), t3_v2(0.6f, 0.8f, 0.8f), t3_v3(0.9f, 0.0f, -0.6f);
glm::vec3
	t1_a = t1_v2 - t1_v1, t1_b = t1_v3 - t1_v1, 
	t2_a = t2_v2 - t2_v1, t2_b = t2_v3 - t2_v1, 
	t3_a = t3_v2 - t3_v1, t3_b = t3_v3 - t3_v1;
glm::vec3
	t1_n = glm::cross(t1_a, t1_b),
	t2_n = glm::cross(t2_a, t2_b), 
	t3_n = glm::cross(t3_a, t3_b);

//Min-max coordinates for image plane
float min_pt_x = 0.0f, min_pt_y = -1.0f, min_pt_z = 3.0f;
float max_pt_x = 2.0f, max_pt_y = 1.0f, max_pt_z = min_pt_z;

//Sets of object coefficients
glm::vec3
	all_ones(1.0f, 1.0f, 1.0f);

RgbImage * myImage;

//Getting the reflected ray
glm::vec3 reflect(Ray r, glm::vec3 normalVector)
{
	glm::vec3 l = glm::normalize(-1.0f*(r.slope));
	return ((2 * glm::dot(normalVector, l) * normalVector) - l);
}

//The recursive tracing function
glm::vec3 trace (SceneObject* object, Ray ray, Light* light, float lowDistance, SceneObject* objects[NUM_OBJECTS], int depth, glm::vec3 cameraPosition)
{
	glm::vec3 cLocal, cReflected;	//The colours for local and reflected
	///*
	glm::vec3 objectColor = glm::vec3(object->getRed(), object->getGreen(), object->getBlue());
	glm::vec3 intersection = ray.point + (lowDistance - 0.05f)*glm::normalize(ray.slope);
	glm::vec3 direction = glm::normalize(reflect(ray, glm::normalize(object->getNormal())));
	Ray r(intersection, direction);
	float lowestDistance = -1.0f, currentDistance;
	int objectState = NO_OBJECT, arrayState = -1;

	//If a certain depth has been reached, return the background colour (sky blue)
	if (depth > MAX_DEPTH)
		return sky;

	//Check for intersections with objects
	for (int i = 0; i < NUM_OBJECTS; i++)
	{
		currentDistance = objects[i]->intersect(r);
		if (currentDistance != -1.0f)
		{
			if (lowestDistance == -1.0f || currentDistance < lowestDistance){
				objectState = OBJECT_HIT; arrayState = i; lowestDistance = currentDistance;
			}
		}
	}
	//Check for intersections with lights
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		currentDistance = light[i].intersect(r);
		if (currentDistance != -1.0f)
		{
			if (lowestDistance == -1.0f || currentDistance < lowestDistance){
				objectState = LIGHT_HIT; arrayState = i; lowestDistance = currentDistance;
			}
		}
	}
	//If an intersection has been found
	if (lowestDistance != -1.0f)
	{
		//If the closest intersection was an object, ray trace again, then combine the results
		if (objectState == OBJECT_HIT){
			cLocal = (1.0f - object->getRef()) * (object->shader(ray, cameraPosition, light, lowDistance, objects));
			cReflected =  object->getRef() * (trace(objects[arrayState], r, light, lowestDistance, objects, depth+1, cameraPosition));
			glm::vec3 returnValue = (cLocal + cReflected);
			if (returnValue.x < 0.0f) returnValue.x = 0.0f; if (returnValue.x > 1.0f) returnValue.x = 1.0f;
			if (returnValue.y < 0.0f) returnValue.y = 0.0f; if (returnValue.y > 1.0f) returnValue.y = 1.0f;
			if (returnValue.z < 0.0f) returnValue.z = 0.0f; if (returnValue.z > 1.0f) returnValue.z = 1.0f;
			return returnValue;
		}
		//If the closest intersection was a light, return the white light
		if (objectState == LIGHT_HIT)
			return (1 - object->getRef())*object->shader(ray, cameraPosition, light, lowDistance, objects) + object->getRef() * white;
	}
	//If there weren't any intersections, return the background colour
	return (1 - object->getRef())*(object->shader(ray, cameraPosition, light, lowDistance, objects)) + object->getRef() * sky;
}

int main()
{
	string strFile; int reflectState = 0;
	cout<<"Please enter the filename (without .bmp) that you want to save the image as: ";
	cin >> strFile;
	cout<<"Press 1 for a 0% reflective plane. Press 2 for a 50% reflective plane: ";
	cin >> reflectState;
	while (reflectState != 1 && reflectState != 2)
	{
		cout<<"Try again. Press 1 for a 0% reflective plane. Press 2 for a 50% reflective plane: ";
		cin >> reflectState;
	}
	cout<<"Cool beans. Creating image..."<<endl;
	strFile.append(".bmp");

	//Where the objects get created
	SceneObject* objects[8];														
	Light l1(0.3f, glm::vec3(10.0f, 10.0f, 0.0f), 2.0f);
	Light l2(0.2f, glm::vec3(0.2f, 0.0f, 5.0f), 2.0f);
	Light l3(0.3f, glm::vec3(10.0f, 0.0f, 10.0f), 2.0f);
	Light light[NUM_LIGHTS] = {l1, l2, l3};
	objects[0] = new Sphere(0.2f, glm::vec3(0.2f, 0.2f, 0.0f), red, all_ones, 0.8f);
	objects[1] = new Sphere(0.1f, glm::vec3(1.0f, 1.0f, 1.0f), red, all_ones, 0.4f);
	objects[2] = new Sphere(0.7f, glm::vec3(0.5f, 0.5f, -1.0f), blue, all_ones, 0.1f);
	objects[3] = new Sphere(0.6f, glm::vec3(0.8f, -0.5f, 0.0f), gold, all_ones, 0.0f);
	if (reflectState == 1)
		objects[4] = new Plane(0.0f, plane_normal, grass, all_ones, 0.0f);
	else
		objects[4] = new Plane(0.0f, plane_normal, grass, all_ones, 0.5f);
	objects[5] = new Triangle(t1_v1, t1_v2, t1_v3, t1_n, pink, all_ones, 0.2f);
	objects[6] = new Triangle(t2_v1, t2_v2, t2_v3, t2_n, sand, all_ones, 0.3f);
	objects[7] = new Triangle(t3_v1, t3_v2, t3_v3, t3_n, grey, all_ones, 0.0f);

	//Setting up the pixel screen and the camera
	int width = IMAGE_WIDTH, height = IMAGE_HEIGHT;
	glm::vec3 cameraPos(1.0f, 0.0f, 10.0f);
	glm::vec3 imageMinPt(min_pt_x, min_pt_y, min_pt_z), 
		imageMaxPt(max_pt_x, max_pt_y, max_pt_z);
	float xSize = (max_pt_x - min_pt_x) / width, 
		ySize = (max_pt_y - min_pt_y) / height;

	int depth = 0;
	myImage = new RgbImage(width, height);
	float lowDistance = -1.0f, currentDistance;
	int state = -1;

	//Checking for intersections starts here
	for (int u = 0; u < width; u++){
		for (int v = 0; v < height; v++){
			glm::vec3 pixelPos(min_pt_x + (xSize * u), min_pt_y + (ySize * v), min_pt_z);
			glm::vec3 slope = pixelPos - cameraPos;
			slope = glm::normalize(slope);
			Ray r(cameraPos, slope);
			for (int i = 0; i < NUM_OBJECTS; i++){
				currentDistance = objects[i]->intersect(r);		//Intersection checking
				if (currentDistance != -1.0f){
					if (lowDistance == -1.0f || currentDistance < lowDistance){
						lowDistance = currentDistance;
						state = i;
					}
				}
			}
			if (state != -1){	//If there was an intersection
				glm::vec3 shades = trace(objects[state], r, light, lowDistance, objects, depth, cameraPos);	//Ray tracing begins
				myImage->SetRgbPixelf(u, v, shades.x, shades.y, shades.z);
				state = -1;
				lowDistance = -1.0f;
			}
			else				//If no intersection, colour the pixel sky blue
				myImage->SetRgbPixelf(u, v, sky.x, sky.y, sky.z);
		}
	}
	//Create the bmp file
	myImage->WriteBmpFile(strFile.c_str());
	return 0;
}