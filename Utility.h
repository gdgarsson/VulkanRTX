#pragma once
#include <vector>
#include "Vector.h"
#include "Matrix33f.h"
#include "Matrix44f.h"
#include "Ray.h"
#include "Camera.h"
#include "Light.h"
#include "Object.h"
#include "PPMImage.h"

struct ReflectInfo {
	Vector a; // initial vector
	Vector b; // reflection vector
};

class Utility
{
public:
	int max_depth = 5;

	/*Vector normalize2D(Vector v1, Vector v2); // Normalize two 2D vectors
	Vector normalize3D(Vector v1, Vector v2); // Normalize two 3D vectors
	Vector normalize4D(Vector v1, Vector v2); // Normalize two 4D vectors*/
	
	float dot2D(Vector v1, Vector v2); // Dot product of two 2D vectors
	float dot3D(Vector v1, Vector v2); // Dot product of two 3D vectors
	float dot4D(Vector v1, Vector v2); // Dot product of two 4D vectors
	
	Vector cross3D(Vector v1, Vector v2); // Cross product of two 3D vectors

	// Converts Radians to Degrees
	float toDegrees(float rad);
	// Converts Degrees to Radians
	float toRadians(float deg); 

	float distance2D(Vector p1, Vector p2);
	float distance3D(Vector p1, Vector p2);

	float clamp(float value, float min, float max); // clamp value between min and max

	bool solveQuadratic(const float& a, const float& b, const float& c, float &x0, float &x1);

	// functions for inverting matrices of sizes 3x3 and 4x4
	Matrix33f invertMat33f(Matrix33f m);
	Matrix44f invertMat44f(Matrix44f m);

	// Reflect Ray cast function. Depth determines how many times the function has been recursively called. Returns the color vector.
	Vector castReflectRay(Ray ray, std::vector<Light*>* lights, std::vector<Object*>* objects, int depth);

	Vector blendColors(Vector color1, Vector color2, bool useA = false);

	// Returns the diffuse color using the light direction vector and the normal of the collision point
	Vector diffuse(Vector surface_normal_hit, Vector light_direction, Vector light_color, float light_intensity);
	// Returns the specular highlight color using the normal of the collision point, the direction of the light, and viewing direction
	Vector specular(Vector surface_normal_hit, Vector light_direction, Vector reflect_direction, float light_intensity, float shininess, Vector view_direction);

	Vector getReflectionDirection(Vector surface_normal_hit, Vector light_direction);
	Vector getRefractionDirection(Vector incidence_vector, Vector surface_normal_hit, float ior);

	void setProjectionMatrix(float& angleOfView, float& near, float& far, Matrix44f& M);

	bool computePixelCoordinates(Vector& pWorld, Matrix44f& cameraToWorld, float& canvasWidth, float& canvasHeight,
		int& imageWidth, int& imageHeight, Vector& pRaster);
};

