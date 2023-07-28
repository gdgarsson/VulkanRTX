#pragma once
#include <vector>
#include "Object.h"
#include "Vector.h"
#include "Matrix44f.h"
#include "PPMImage.h"
#include "Ray.h"
#include "Light.h"

class Camera
{
private:
	Vector m_position = Vector(0.0f, 0.0f, 4.0f); // Position of the camera
	Vector m_forward = Vector(0.0f, 0.0f, -1.0f); // Forward direction of the camera
	Vector m_up = Vector(0.0f, 1.0f, 0.0f); // Defines the "up" vector for the camera (defaults to (0, 1, 0))
	float m_fov = 45.0f; // Field of view

	int m_x_res; // X-resolution of the camera
	int m_y_res; // Y-resolution of the camera

	float t_near = 0.0f; // minimum distance needed to draw.
	float t_far = 1000.0f; // Maxmimum draw distance.

	PPMImage* m_img; // Image buffer pointer

	Vector pixelToNDC(float pixel_x, float pixel_y); // Converts a pixel on the screen to Normalized Device Coordinates


public:
	// Constructors
	Camera();
	Camera(Vector position);
	Camera(Vector position, Vector forward);
	Camera(Vector position, float fov);
	Camera(Vector position, Vector forward, float fov);
	
	// Destructor
	~Camera();

	// Setters
	bool assignImage(int xres, int yres);
	bool assignImage(PPMImage* img);

	void setPosition(Vector position);
	void setForward(Vector forward);
	void setUp(Vector up);
	void setFOV(float new_fov);
	void setResolution(int xres, int yres);
	void setXResolution(int xres);
	void setYResolution(int yres);

	// Getters
	Vector getPosition();
	Vector getForward();
	Vector getUp();
	float getFOV();
	int getXResolution();
	int getYResolution();
	float getAspectRatio();
	Matrix44f getTranslationMatrix();

	PPMImage* getImage();

	Matrix44f lookAt(Vector eye, Vector at, Vector up);
	Matrix44f cameraToWorld();

	// Raycast function
	void castRays(std::vector<Object*> *objects, std::vector<Light*>* lights);
	void castRaysMV(std::vector<Object*>* objects, std::vector<Light*>* lights);

	inline Vector castRay(Ray r, std::vector<Object*>* objects, std::vector<Light*>* lights);
};

