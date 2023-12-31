#include "Utility.h"
#include <math.h>
#include <utility>
#include <iostream>
#include <algorithm>

#define PI 3.14159265358979323846

float Utility::distance2D(Vector p1, Vector p2) {
	float x2 = powf((p2.getX() - p1.getX()), 2.0f);
	float y2 = powf((p2.getY() - p1.getY()), 2.0f);

	return sqrtf(x2 + y2);
}

float Utility::distance3D(Vector p1, Vector p2) {
	float x2 = powf((p2.getX() - p1.getX()), 2.0f);
	float y2 = powf((p2.getY() - p1.getY()), 2.0f);
	float z2 = powf((p2.getZ() - p1.getZ()), 2.0f);

	return sqrtf(x2 + y2 + z2);
}

// Returns the 2D dot product of two vectors
float Utility::dot2D(Vector v1, Vector v2) {
	return (v1.getX() * v2.getX()) + (v1.getY() * v2.getY());
}

// Returns the 3D dot product of two vectors
float Utility::dot3D(Vector v1, Vector v2) {
	return (v1.getX() * v2.getX()) + (v1.getY() * v2.getY()) + (v1.getZ() * v2.getZ());
}

// Returns the 4D dot product of two vectors
float Utility::dot4D(Vector v1, Vector v2) {
	return (v1.getX() * v2.getX()) + (v1.getY() * v2.getY()) + (v1.getZ() * v2.getZ()) + (v1.getA() * v2.getA());
}

// Returns the 3D cross product of two vectors
Vector Utility::cross3D(Vector v1, Vector v2) {
	float new_x = (v1.getY() * v2.getZ()) - (v1.getZ() * v2.getY());
	float new_y = -1 * ((v1.getX() * v2.getZ()) - (v1.getZ() * v2.getX()));
	float new_z = (v1.getX() * v2.getY()) - (v1.getY() * v2.getX());

	return Vector(new_x, new_y, new_z);
}

// Converts Radians to Degrees
float Utility::toDegrees(float rad) {
	return (rad * 180.0f) / PI;
}

// Converts Degrees to Radians
float Utility::toRadians(float deg) {
	return (deg * PI) / 180.0f;
}

// Clamps a float between a specified minimum and maximums
float Utility::clamp(float value, float min, float max) {
	if (value < min) {
		value = min;
	}
	
	if (value > max) {
		value = max;
	}
	
	return value;

}

// Attempt to solve a quadratic with given a, b, and c values
bool Utility::solveQuadratic(const float& a, const float& b, const float& c, float& x0, float& x1) {
	float discr = (b * b) - (4 * a * c);

	if (discr < 0) return false;
	else if (discr == 0) x0 = x1 = (-0.5f * b) / a;
	else { // discr > 0
		float q = (b > 0) ?
			-0.5f * (b + sqrtf(discr)) : // if b > 0 
			-0.5f * (b - sqrtf(discr)); // if b <= 0
		x0 = q / a;
		x1 = c / q;
	}
	if (x0 > x1) std::swap(x0, x1);

	return true;
}

// Reflection ray cast function - used with reflection items
Vector Utility::castReflectRay(Ray ray, std::vector<Light*>* lights, std::vector<Object*>* objects, int depth) {
	Vector view_dir = ray.getDirection();
	view_dir.normalize3D();
	Vector final_color = Vector();
	IntersectInfo IsectInfo;
	IsectInfo.t = 0;

	// if the ray has its maximum # of times, return black
	if (depth > ray.getMaxDepth()) return final_color;

	for (int i = 0; i < objects->size(); i++) {
		if (objects->at(i)->intersects(ray, IsectInfo)) {
			switch (objects->at(i)->getShadeType()) {
			case kPhong: {
				IntersectInfo LightIsectInfo;

				for (int l = 0; l < lights->size(); l++) {
					Ray light_ray = Ray(lights->at(l)->getPosition());
					Vector light_dir = IsectInfo.PHit - light_ray.getOrigin();
					light_dir.normalize3D();
					light_ray.setDirection(light_dir);
					Vector temp_color = lights->at(l)->getColor();
					//std::cout << "Light color: " << lights->at(i)->getColor().toString() << std::endl;
					Vector spec = Vector();
					Vector diff = Vector();
					Vector reflect = getReflectionDirection(IsectInfo.NHit, light_dir);
					reflect.normalize3D();

					if (objects->at(i)->intersects(Ray(lights->at(l)->getPosition(), IsectInfo.PHit - lights->at(l)->getPosition()), LightIsectInfo)) {
						diff = diffuse(IsectInfo.NHit, light_ray.getDirection(), lights->at(l)->getColor(), lights->at(l)->getIntensity());
						spec = specular(IsectInfo.NHit, light_ray.getDirection(), reflect,
							lights->at(l)->getIntensity(), objects->at(i)->getShininess(), view_dir);

						temp_color = (diff * objects->at(i)->getDiffuseValue()) + (spec * objects->at(i)->getSpecularValue());
						final_color += temp_color;

						//std::cout << "Light " << l << " Color: " << temp_color.toString() << std::endl;
						//std::cout << "New Color:" << final_color.toString() << std::endl;
					}
				}
				break;
			}
			case kReflect: {
				Vector reflectDir = getReflectionDirection(IsectInfo.NHit, ray.getDirection());
				Ray reflectRay = Ray(IsectInfo.PHit + IsectInfo.NHit, reflectDir);
				reflectRay.setMaxDepth(ray.getMaxDepth()); // make sure the new ray's max depth is the same as the starter!
				final_color += castReflectRay(reflectRay, lights, objects, depth + 1) * 0.8f;
				break;
			}
			default: {
				break;
			}
			}
		}
	}
	
	return final_color;
}

// Blend two colors and return the result
Vector Utility::blendColors(Vector color1, Vector color2, bool useA) {

	Vector temp_c1 = color1;
	//temp_c1.normalize3D();
	Vector temp_c2 = color2;
	//temp_c2.normalize3D();
	//std::cout << "Blending Colors: " << temp_c1.toString() << " and " << temp_c2.toString() << std::endl;
	
	Vector newColor = Vector((temp_c1.getX() + temp_c2.getX()) / 2.0f,
		(temp_c1.getY() + temp_c2.getY()) / 2.0f,
		(temp_c1.getZ() + temp_c2.getZ()) / 2.0f);
	if (useA) {
		float a = color1.getA() + color2.getA() * (1.0f - color1.getA());
		newColor.setA(a);
	}
	//std::cout << "New Color: " << newColor.toString() << std::endl;

	return newColor;
}

// Get the reflection direciton from a given normal and light direction
Vector Utility::getReflectionDirection(Vector surface_normal_hit, Vector light_direction) {
	Vector R = ((surface_normal_hit * dot3D(surface_normal_hit, light_direction)) * 2.0f) - light_direction;
	//R.normalize3D();
	//std::cout << R.toString() << std::endl;
	return R;
}

// Returns the diffuse color gathered from the surface normal, light direction, light color, and light intensity.
Vector Utility::diffuse(Vector surface_normal_hit, Vector light_direction, Vector light_color, float light_intensity) {
	float lambert_cos = dot3D(surface_normal_hit, -light_direction);
	if (lambert_cos < 0.0f) lambert_cos = 0.0f;
	if (lambert_cos > 1.0f) lambert_cos = 1.0f;
	//std::cout << "Lambert Cos: " << lambert_cos << std::endl;
	float diffuse = lambert_cos * light_intensity;
	return Vector(light_color.getX() * diffuse, light_color.getY() * diffuse, light_color.getZ() * diffuse);
}

// Returns the specular color gathered from the surface normal, light direction, reflection direction, light intensity, shininess, and view direction.
Vector Utility::specular(Vector surface_normal_hit, Vector light_direction, Vector reflect_direction, float light_intensity, float shininess, Vector view_direction) {
	float specular = dot3D(view_direction, reflect_direction);
	specular = clamp(specular, 0.0f, 1.0f);
	//std::cout << specular << std::endl;
	float spec_with_shiny = std::powf(specular, shininess);
	spec_with_shiny *= light_intensity;
	if(spec_with_shiny < 0.0f) std::cout << spec_with_shiny << std::endl;
	return Vector(spec_with_shiny, spec_with_shiny, spec_with_shiny);
	//return Vector(specular, specular, specular);
}

// Returns the refraction direction from the given incidence vector and the normal of the surface that was hit
Vector Utility::getRefractionDirection(Vector incidence_vector, Vector surface_normal_hit, float refract_index) {
	
	float cosi = clamp(dot3D(incidence_vector, surface_normal_hit), -1, 1);
	float etai = 1;
	float etat = refract_index;
	Vector n = surface_normal_hit;
	if (cosi < 0) { // not inside surface
		cosi = -cosi;
	}
	else { // inside surface
		std::swap(etai, etat);
		n = -surface_normal_hit;
	}

	float eta = etai / etat;

	float k = 1 - eta * eta * (1 - cosi * cosi);
	if (k < 0) {
		return Vector();
	}
	else {
		Vector return_val = (incidence_vector * eta) + n * (eta * cosi - sqrtf(k));
		return return_val;
	}
}

void Utility::setProjectionMatrix(float& fov, float& near, float& far, Matrix44f& M)
{
	// set the basic projection matrix
	float scale = 1 / tan(fov * 0.5 * PI / 180);
	M.m[0][0] = scale; // scale the x coordinates of the projected point 
	M.m[1][1] = scale; // scale the y coordinates of the projected point 
	M.m[2][2] = -far / (far - near); // used to remap z to [0,1] 
	M.m[3][2] = -far * near / (far - near); // used to remap z [0,1] 
	M.m[2][3] = -1; // set w = -z 
	M.m[3][3] = 0;
}

bool Utility::computePixelCoordinates(
	Vector& pWorld,
	Matrix44f& cameraToWorld,
	float& canvasWidth,
	float& canvasHeight,
	int& imageWidth,
	int& imageHeight,
	Vector& pRaster)
{
	// First transform the 3D point from world space to camera space. 
	// It is of course inefficient to compute the inverse of the cameraToWorld
	// matrix in this function. It should be done outside the function, only once
	// and the worldToCamera should be passed to the function instead. 
	// We are only compute the inverse of this matrix in this function ...
	Vector pCamera;
	Matrix44f worldToCamera = cameraToWorld.getInverse();
	pCamera = worldToCamera.multVecMatrix(pWorld);
	// Coordinates of the point on the canvas. Use perspective projection.
	Vector pScreen;
	pScreen.setX(pCamera.getX() / -pCamera.getZ());
	pScreen.setY(pCamera.getY() / -pCamera.getZ());
	// If the x- or y-coordinate absolute value is greater than the canvas width 
	// or height respectively, the point is not visible
	if (std::abs(pScreen.getX()) > canvasWidth || std::abs(pScreen.getY()) > canvasHeight)
		return false;
	// Normalize. Coordinates will be in the range [0,1]
	Vector pNDC;
	pNDC.setX((pScreen.getX() + canvasWidth / 2) / canvasWidth);
	pNDC.setY((pScreen.getY() + canvasHeight / 2) / canvasHeight);
	// Finally convert to pixel coordinates. Don't forget to invert the y coordinate
	pRaster.setX(std::floor(pNDC.getX() * imageWidth));
	pRaster.setY(std::floor((1 - pNDC.getY()) * imageHeight));

	return true;
}