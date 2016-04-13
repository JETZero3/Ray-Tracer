#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include <stdlib.h>
#include <iostream>

#include "glm/glm/glm.hpp"
#include "glm/glm/gtx/transform.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"

#include <math.h>

#define NUM_OBJECTS 8

#define NUM_LIGHTS 3
//Ray class to simplify the code
class Ray{	
public:
	glm::vec3 point;
	glm::vec3 slope;
	Ray(glm::vec3 p, glm::vec3 s){
		point = p;
		slope = s;
	}
};	

//Light class
class Light{
public:
	float intensity;
	float radius;
	glm::vec3 position;
	Light(float i, glm::vec3 p, float r)
	{
		intensity = i;
		position = p;
		radius = r;
	}
	float intersect(Ray r)
	{
		float d = glm::distance(position, r.point);
		if (d == radius)
			return -1.0f;
		glm::vec3 pointToCenter = position - r.point;
		float tp = glm::dot(pointToCenter, r.slope);
		float r2 = pow(radius, 2);
		float p2c_length = sqrt(pow(pointToCenter.x, 2) + pow(pointToCenter.y, 2) + pow(pointToCenter.z, 2));
		if (tp < 0.0f)
		{
			return -1.0f;
		}
		float e2 = pow(p2c_length, 2) - pow(tp, 2);
		if (e2 > r2)
		{
			return -1.0f;
		}
		float t_prime = sqrt(r2 - e2);
		if (d > radius)
		{
			float returnValue = tp - t_prime;
			return returnValue;
		}
		float returnValue = tp - t_prime;
		return returnValue;
	}
};  

//Object classes
class SceneObject{
public:
	virtual float intersect(Ray r) = 0;
	virtual float getRed() = 0;
	virtual float getGreen() = 0;
	virtual float getBlue() = 0;
	virtual glm::vec3 getNormal() = 0;
	virtual float getRef() = 0;
	virtual glm::vec3 shader(Ray ray, glm::vec3 view, Light* light, float lowDistance, SceneObject* object[NUM_OBJECTS]) = 0;
};

class Sphere : public SceneObject{
private:
	float radius;
	glm::vec3 center;
	glm::vec3 rgb;
	glm::vec3 normal;
	float reflective;
	float diffuse, specular, ambient;
public:
	float intersect(Ray r) {
		float d = glm::distance(center, r.point);
		if (d == radius)
			return -1.0f;
		glm::vec3 pointToCenter = center - r.point;
		float tp = glm::dot(pointToCenter, r.slope);
		float r2 = pow(radius, 2);
		float p2c_length = sqrt(pow(pointToCenter.x, 2) + pow(pointToCenter.y, 2) + pow(pointToCenter.z, 2));
		if (tp < 0.0f)
		{
			return -1.0f;
		}
		float e2 = pow(p2c_length, 2) - pow(tp, 2);
		if (e2 > r2)
		{
			return -1.0f;
		}
		float t_prime = sqrt(r2 - e2);
		if (d > radius)
		{
			float returnValue = tp - t_prime;
			glm::vec3 point = r.point + (returnValue - 0.01f)*r.slope;
			normal = glm::normalize(point - center);
			return returnValue;
		}
		float returnValue = tp - t_prime;
		glm::vec3 point = r.point + (returnValue - 0.01f)*r.slope;
		normal = glm::normalize(point - center);
		return returnValue;
	}
	Sphere(float r, glm::vec3 c, glm::vec3 color, glm::vec3 coefficients, float ref){
		radius = r; center = c; rgb = color; 
		diffuse = coefficients.x; specular = coefficients.y; ambient = coefficients.z; reflective = ref;
	}
	float getRed(){return rgb.x;}
	float getGreen(){return rgb.y;}
	float getBlue(){return rgb.z;}
	float getRef(){return reflective;}
	glm::vec3 getNormal() {return normal;}
	glm::vec3 shader(Ray ray, glm::vec3 view, Light* light, float lowDistance, SceneObject* object[NUM_OBJECTS]){
		glm::vec3 point = ray.point + (lowDistance - 0.01f)*ray.slope;
		normal = glm::normalize(point - center);
		float d = 0, s = 0, a = 0;
		float lowest = -1.0f;
		for (int i = 0; i < NUM_LIGHTS; i++)
		{
			if (light[i].intensity != 0.0f)
			{
				glm::vec3 shadowDirection = glm::normalize(light[i].position - point);
				Ray shadowRay(point, shadowDirection);
				for (int j = 0; j < NUM_OBJECTS; j++)
				{
					float e = object[j]->intersect(shadowRay);
					if (e != -1.0f){
						if (lowest == -1.0f || e < lowest){
							lowest = e;
						}
					}
				}
				float distance_light_point = glm::distance(light[i].position, point);
				if (lowest == -1.0f || distance_light_point < lowest)
				{
					glm::vec3 l = glm::normalize(light[i].position - point);
					glm::vec3 r = glm::normalize((2*glm::dot(normal, l) * normal) - l);
					glm::vec3 v = glm::normalize(view - point);
					d = d + ((diffuse * light[i].intensity) * glm::dot(normal, l));
					s = s + (specular * light[i].intensity * glm::dot(r, v));
				}
				lowest = -1.0f;
				a = a + (ambient * light[i].intensity);
			}
		}
		if (d < 0) d = 0; if (d > 1) d = 1;
		if (s < 0) s = 0; if (s > 1) s = 1;
		if (a < 0) a = 0; if (a > 1) a = 1;
		float final_intensity = d + s + a;
		float final_red = rgb.x * final_intensity;
		float final_green = rgb.y * final_intensity;
		float final_blue = rgb.z * final_intensity;
		if (final_red < 0.0f) final_red = 0.0f; if (final_red > 1.0f) final_red = 1.0f;
		if (final_green < 0.0f) final_green = 0.0f; if (final_green > 1.0f) final_green = 1.0f;
		if (final_blue < 0.0f) final_blue = 0.0f; if (final_blue > 1.0f) final_blue = 1.0f;
		glm::vec3 final = glm::vec3(final_red, final_green, final_blue);
		return final;
	}
};

class Triangle : public SceneObject{
private:
	glm::vec3 vert1, vert2, vert3, normal;
	glm::vec3 rgb;
	float diffuse, specular, ambient;
	float reflective;
public:
	float intersect(Ray r) { //https://courses.cs.washington.edu/courses/csep557/10au/lectures/triangle_intersection.pdf
		float slopeDotNormal = glm::dot(r.slope, normal);
		if (slopeDotNormal == 0.0f)
			return -1.0f;
		glm::vec3 pointToV1 = r.point - vert1;
		float normalDotP2V1 = glm::dot(pointToV1, normal);
		float t = -(normalDotP2V1)/(slopeDotNormal);
		if (t < 0)
			return -1.0f;
		glm::vec3 P = r.point + t*r.slope;
		glm::vec3 B_minus_A = vert2 - vert1, C_minus_B = vert3 - vert2, A_minus_C = vert1 - vert3, 
				P_minus_A = P - vert1, P_minus_B = P - vert2, P_minus_C = P - vert3;
		if (glm::dot(normal, glm::cross(B_minus_A, P_minus_A)) < 0  ||
			glm::dot(normal, glm::cross(C_minus_B, P_minus_B)) < 0	||
			glm::dot(normal, glm::cross(A_minus_C, P_minus_C)) < 0)
			return -1.0f;
		float distance = glm::distance(P, r.point);
		return distance;
	}
	Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 n, glm::vec3 color, glm::vec3 coefficients, float ref){
		vert1 = v1; vert2 = v2; vert3 = v3; normal = glm::normalize(n); rgb = color;
		diffuse = coefficients.x; specular = coefficients.y; ambient = coefficients.z; reflective = ref;
	}
	float getRed(){return rgb.x;}
	float getGreen(){return rgb.y;}
	float getBlue(){return rgb.z;}
	float getRef(){return reflective;}
	glm::vec3 getNormal() {return normal;}
	glm::vec3 shader(Ray ray, glm::vec3 view, Light* light, float lowDistance, SceneObject* object[NUM_OBJECTS]){
		glm::vec3 point = ray.point + (lowDistance-0.01f)*ray.slope;
		float d = 0, s = 0, a = 0;
		float lowest = -1.0f;
		for (int i = 0; i < NUM_LIGHTS; i++)
		{
			if (light[i].intensity != 0.0f)
			{
				glm::vec3 shadowDirection = glm::normalize(light[i].position - point);
				Ray shadowRay(point, shadowDirection);
				for (int j = 0; j < NUM_OBJECTS; j++)
				{
					float e = object[j]->intersect(shadowRay);
					if (e != -1.0f){
						if (lowest == -1.0f || e < lowest){
							lowest = e;
						}
					}
				}
				float distance_light_point = glm::distance(light[i].position, point);
				if (lowest == -1.0f || distance_light_point < lowest)
				{
					glm::vec3 l = glm::normalize(light[i].position - point);
					glm::vec3 r = glm::normalize((2*glm::dot(normal, l) * normal) - l);
					glm::vec3 v = glm::normalize(view - point);
					d = d + ((diffuse * light[i].intensity) * glm::dot(normal, l));
					s = s + (specular * light[i].intensity * glm::dot(r, v));
				}
				lowest = -1.0f;
				a = a + (ambient * light[i].intensity);
			}
		}
		if (d < 0) d = 0; if (d > 1) d = 1;
		if (s < 0) s = 0; if (s > 1) s = 1;
		if (a < 0) a = 0; if (a > 1) a = 1;
		float final_intensity = d + s + a;
		float final_red = rgb.x * final_intensity;
		float final_green = rgb.y * final_intensity;
		float final_blue = rgb.z * final_intensity;
		if (final_red < 0.0f) final_red = 0.0f; if (final_red > 1.0f) final_red = 1.0f;
		if (final_green < 0.0f) final_green = 0.0f; if (final_green > 1.0f) final_green = 1.0f;
		if (final_blue < 0.0f) final_blue = 0.0f; if (final_blue > 1.0f) final_blue = 1.0f;
		glm::vec3 final = glm::vec3(final_red, final_green, final_blue);
		return final;
	}
};

class Plane : public SceneObject{
private:
	glm::vec3 normal;
	glm::vec3 rgb;
	float plane_d;
	float diffuse, specular, ambient;
	float reflective;
public:
	float intersect(Ray r) { //https://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld017.htm
		float slopeDotNormal = glm::dot(r.slope, normal); //(n dot d)
		if (slopeDotNormal == 0)
			return -1.0f;
		glm::vec3 A(0.0f, 0.0f, plane_d);
		A = r.point - A;
		float t = glm::dot(A, normal); //(n dot (O-A))
		t = -(t/slopeDotNormal);
		if (t < 0)
			return -1.0f;
		glm::vec3 P = t*r.slope;
		P = P + r.point;
		float distance = glm::distance(P, r.point);
		return distance;
	}
	Plane(float d, glm::vec3 n, glm::vec3 color, glm::vec3 coefficients, float ref){
		plane_d = d; normal = glm::normalize(n); rgb = color;
		diffuse = coefficients.x; specular = coefficients.y; ambient = coefficients.z; reflective = ref;
	}
	float getRed(){return rgb.x;}
	float getGreen(){return rgb.y;}
	float getBlue(){return rgb.z;}
	float getRef(){return reflective;}
	glm::vec3 getNormal() {return normal;}
	glm::vec3 shader(Ray ray, glm::vec3 view, Light* light, float lowDistance, SceneObject* object[NUM_OBJECTS]){
		glm::vec3 point = ray.point + (lowDistance-0.01f)*ray.slope;
		float d = 0, s = 0, a = 0;
		float lowest = -1.0f;
		for (int i = 0; i < NUM_LIGHTS; i++)
		{
			if (light[i].intensity != 0.0f)
			{
				glm::vec3 shadowDirection = glm::normalize(light[i].position - point);
				Ray shadowRay(point, shadowDirection);
				for (int j = 0; j < NUM_OBJECTS; j++)
				{
					float e = object[j]->intersect(shadowRay);
					if (e != -1.0f){
						if (lowest == -1.0f || e < lowest){
							lowest = e;
						}
					}
				}
				float distance_light_point = glm::distance(light[i].position, point);
				if (lowest == -1.0f || distance_light_point < lowest)
				{
					glm::vec3 l = glm::normalize(light[i].position - point);
					glm::vec3 r = glm::normalize((2*glm::dot(normal, l) * normal) - l);
					glm::vec3 v = glm::normalize(view - point);
					d = d + ((diffuse * light[i].intensity) * glm::dot(normal, l));
					s = s + (specular * light[i].intensity * glm::dot(r, v));
				}
				lowest = -1.0f;
				a = a + (ambient * light[i].intensity);
			}
		}
		if (d < 0) d = 0; if (d > 1) d = 1;
		if (s < 0) s = 0; if (s > 1) s = 1;
		if (a < 0) a = 0; if (a > 1) a = 1;
		float final_intensity = d + s + a;
		float final_red = rgb.x * final_intensity;
		float final_green = rgb.y * final_intensity;
		float final_blue = rgb.z * final_intensity;
		if (final_red < 0.0f) final_red = 0.0f; if (final_red > 1.0f) final_red = 1.0f;
		if (final_green < 0.0f) final_green = 0.0f; if (final_green > 1.0f) final_green = 1.0f;
		if (final_blue < 0.0f) final_blue = 0.0f; if (final_blue > 1.0f) final_blue = 1.0f;
		glm::vec3 final = glm::vec3(final_red, final_green, final_blue);
		return final;
	}
};