#include "Camera.h"
#include <iostream>
#include <Windows.h>
#include <math.h>
#include "Utility.h"
#include "Sphere.h"
#include "Light.h"
#include "PointLight.h"

// Default camera construtor
Camera::Camera() {
	
}

// Create a camera at a specified position
Camera::Camera(Vector position) {
	m_position = position;
}

// Create a camera at a specified position looking in a specified direction
Camera::Camera(Vector position, Vector forward) {
	m_position = position;
	m_forward = forward;
}

// Create a camera at a specified position with a given FOV
Camera::Camera(Vector position, float fov) {
	m_position = position;
	m_fov = fov;
}

// Create a camera at a specified position with a given FOV looking in a specified direction
Camera::Camera(Vector position, Vector forward, float fov) {
	m_position = position;
	m_forward = forward;
	m_fov = fov;
}

// Destructor
Camera::~Camera() {
	if(m_img) delete m_img;
}

// Converts a pixel on the screen to Normalized Device Coordinates
Vector Camera::pixelToNDC(float pixel_x, float pixel_y) {
	float pixel_ndc_x = (pixel_x + 0.5f) / static_cast<float>(m_x_res);
	float pixel_ndc_y = (pixel_y + 0.5f) / static_cast<float>(m_y_res);

	return Vector(pixel_ndc_x, pixel_ndc_y);
}

// Assign the camera a new image buffer with specifed x resolutions and y resolutions - will return false if a buffer is already applied
bool Camera::assignImage(int xres, int yres) {
	if (m_img) return false;

	m_img = new PPMImage(xres, yres);
	m_x_res = m_img->getXRes();
	m_y_res = m_img->getYRes();
	return true;
}

// Assign the camera an already allocated image buffer - will return false if a buffer is already applied
bool Camera::assignImage(PPMImage* img) {
	if (m_img) return false;
	
	m_img = img;
	m_x_res = m_img->getXRes();
	m_y_res = m_img->getYRes();
	return true;
}

// Setter: sets the camera's position to a new point in space
void Camera::setPosition(Vector new_position) {
	m_position = new_position;
}

// Setter: sets the direction the camera is facing (aka the "forward" Vector)
void Camera::setForward(Vector new_forward) {
	m_forward = new_forward;
}

// Setter: sets the camera's "up" vector
void Camera::setUp(Vector new_up) {
	m_up = new_up;
}

// Setter: sets the FOV to a new value
void Camera::setFOV(float new_fov) {
	m_fov = new_fov;
}

// Setter: changes the resolution to new x and y parameters
// WARNING: This is dangerous and can lead to mismatches between the camera and the image buffer
void Camera::setResolution(int xres, int yres) {
	m_x_res = xres;
	m_y_res = yres;
	if (m_img) {
		m_img->setXRes(m_x_res);
		m_img->setYRes(m_y_res);
	}
}

// Setter: changes the x-axis resolution to a new value
// WARNING: This is dangerous and can lead to mismatches between the camera and the image buffer
void Camera::setXResolution(int xres) {
	m_x_res = xres;
	if (m_img) m_img->setXRes(m_x_res);
}

// Setter: changes the y-axis resolution to a new value
// WARNING: This is dangerous and can lead to mismatches between the camera and the image buffer
void Camera::setYResolution(int yres) {
	m_y_res = yres;
	if (m_img) m_img->setXRes(m_y_res);
}

// Getter: returns the camera's position in space
Vector Camera::getPosition() {
	return m_position;
}

// Getter: returns the direction the camera is facing (aka the "forward" direction)
Vector Camera::getForward() {
	return m_forward;
}

// Getter: returns the "up" Vector for the camera (is defaulted to (0,1,0) upon initialization)
Vector Camera::getUp() {
	return m_up;
}

// Getter: returns the camera's FOV
float Camera::getFOV() {
	return m_fov;
}

// Getter: returns the camera's image buffer
PPMImage* Camera::getImage() {
	return m_img;
}

// Getter: returns the camera's x-axis resolution
int Camera::getXResolution() {
	return m_x_res;
}

// Getter: returns the camera's y-axis resolution.
int Camera::getYResolution() {
	return m_y_res;
}

float Camera::getAspectRatio() {
	return static_cast<float>(m_x_res) / static_cast<float>(m_y_res);
}

Matrix44f Camera::getTranslationMatrix() {
	return Matrix44f(1.0f, 0.0f, 0.0f, m_position.getX(),
		0.0f, 1.0f, 0.0f, m_position.getY(),
		0.0f, 0.0f, 1.0f, m_position.getZ(),
		0.0f, 0.0f, 0.0f, 1.0f);
}

// untested, DO NOT USE
Matrix44f Camera::lookAt(Vector eye, Vector at, Vector up) {
	Utility util;

	Vector cZAxis = at - eye; // view dir vector
	cZAxis.normalize3D();

	Vector cXAxis = util.cross3D(cZAxis, up); // perpendicular vector 
	cXAxis.normalize3D();

	Vector cYAxis = util.cross3D(cXAxis, cZAxis); // the new "up" vector

	//cYAxis.normalize3D();

	cZAxis *= -1.0f;

	Matrix44f viewMatrix = Matrix44f(cXAxis.getX(), cXAxis.getY(), cXAxis.getZ(), -util.dot3D(cXAxis, eye),
		cYAxis.getX(), cYAxis.getY(), cYAxis.getZ(), -util.dot3D(cYAxis, eye),
		cZAxis.getX(), cZAxis.getY(), cZAxis.getZ(), -util.dot3D(cZAxis, eye),
		0.0f, 0.0f, 0.0f, 1.0f);

	return viewMatrix;
}

Matrix44f Camera::cameraToWorld() {
	return lookAt(m_position, m_forward, m_up).getInverse();
}

inline Vector Camera::castRay(Ray r, std::vector<Object*>* objects, std::vector<Light*>* lights) {
	Utility util;
	IntersectInfo IsectInfo;
	Vector final_color = Vector();
	float draw_t = t_far;

	Vector view_dir = r.getDirection(); // making a copy so I don't mess anything major up just in case
	view_dir.normalize3D();
	
	// For each object
	for (int i = 0; i < objects->size(); i++) {
		// Determine if ray cast from camera intersects object i
		if (objects->at(i)->intersects(r, IsectInfo)) {
			IntersectInfo LightIsectInfo; // Create struct containing ray intersect info from the lighting
			Vector obj_color = Vector(); // Create the color vector for object i

			switch (objects->at(i)->getShadeType()) {
			case kPhong: {
				// For each light
				for (int l = 0; l < lights->size(); l++) {
					Ray light_ray = Ray(lights->at(l)->getPosition()); // Create Light Ray
					Vector light_dir = IsectInfo.PHit - light_ray.getOrigin(); // Set the direction of the light ray
					light_dir.normalize3D(); // Normalize the direction
					light_ray.setDirection(light_dir); // Apply to light ray

					Vector light_color = lights->at(l)->getColor(); // Create a temporary color vector containing the light's value
					//std::cout << "Light color: " << lights->at(i)->getColor().toString() << std::endl;
					Vector specular = Vector();
					Vector diffuse = Vector();
					Vector reflect = util.getReflectionDirection(IsectInfo.NHit, light_dir);
					reflect.normalize3D();

					// if the light intersects the object
					if (objects->at(i)->intersects(Ray(lights->at(l)->getPosition(), IsectInfo.PHit - lights->at(l)->getPosition()), LightIsectInfo)) {
						// Apply specular and diffuse lighting
						diffuse = util.diffuse(IsectInfo.NHit, light_ray.getDirection(), lights->at(l)->getColor(), lights->at(l)->getIntensity());
						specular = util.specular(IsectInfo.NHit, light_ray.getDirection(), reflect,
							lights->at(l)->getIntensity(), objects->at(i)->getShininess(), r.getDirection());

						// Add lightign color to the object
						light_color = (diffuse * objects->at(i)->getDiffuseValue()) + (specular * objects->at(i)->getSpecularValue());
						obj_color = obj_color + light_color;

						//std::cout << "Light " << l << " Color: " << temp_color.toString() << std::endl;
						//std::cout << "New Color:" << final_color.toString() << std::endl;
					}
				}

				break;
			}
			case kReflect: {
				// If the object is reflective

				Vector reflectDir = util.getReflectionDirection(IsectInfo.NHit, r.getDirection()); // get the reflection direction at the normal of the intersect
				Ray reflectRay = Ray(IsectInfo.PHit + IsectInfo.NHit, reflectDir); // create a reflection ray
				obj_color += util.castReflectRay(reflectRay, lights, objects, 0) * 0.8f; // Set the object's color to be the color gathered from reflection

				break;
			}

			default: {
				break;

			}
			}

			//std::cout << "Object Color:" << obj_color.toString() << std::endl;
			// If the current object is closer to the camera than the previous object
			if (IsectInfo.t < draw_t) {
				final_color = obj_color; // set the pixel color to the object's color
				draw_t = IsectInfo.t; // set the new draw distance threshold to the t value from the IsectInfo
			}


		}
	}

	//std::cout << "Final Color:" << final_color.toString() << std::endl;

	return final_color;
}

// Casts one ray per pixel and populates each pixel via ray-tracing and intersects (Matrix version)
void Camera::castRaysMV(std::vector<Object*>* objects, std::vector<Light*>* lights) {
	Utility util;
	//Matrix44f viewMatrix = lookAt(m_position, m_forward, m_up);
	Matrix44f viewMatrix = Matrix44f();

	Matrix44f projMatrix = Matrix44f();
	util.setProjectionMatrix(m_fov, t_near, t_far, projMatrix);

	float aspectRatio = static_cast<float>(m_x_res) / static_cast<float>(m_y_res);
	float scale = tan(util.toRadians(m_fov * 0.5f));

	float yFov = (static_cast<float>(m_y_res) / static_cast<float>(m_x_res)) * static_cast<float>(m_fov); // the y-angle FOV of the camera

	// Convert camera coordinates to world coordinates
	Vector orig = viewMatrix.multVecMatrix(m_position);

	Ray ray = Ray(orig); // Create new ray
	Vector rayDir = Vector(); // create temporary directional vector
	
	rayDir.setZ(-1.0f); // Set Z direction to look down -1.0

	// For each pixel on the y-axis
	for (int yPixel = 0; yPixel < m_y_res; yPixel++) {

		// set the ray's direction to the y-axis tangent value
		float y = (1 - 2 * (yPixel + 0.5) / static_cast<float>(m_y_res)) * scale;
		rayDir.setY(y);

		// For each pixel on the x-axis
		for (int xPixel = 0; xPixel < m_x_res; xPixel++) {
			// set the ray's direction to the y-axis tangent value
			float x = (2 * (xPixel + 0.5) / static_cast<float>(m_x_res) - 1) * aspectRatio* scale;
			rayDir.setX(x);

			rayDir = viewMatrix.multDirMatrix(rayDir);

			// Normalize the ray's direction
			rayDir.normalize3D();

			ray.setDirection(rayDir);
			
			Vector view_dir = rayDir; // making a copy so I don't mess anything major up just in case
			view_dir.normalize3D();

			// set the current draw distance value equal to the maximum draw distance
			float draw_t = t_far;

			// Create the final pixel color vector
			Vector final_color = Vector();

			// Create struct containing ray intersect info from the camera
			IntersectInfo IsectInfo;
			IsectInfo.t = 0.0f;

			//std::cout << "Ray sent from camera in heading (" << ray.getDirection().getX() << ", " << ray.getDirection().getY() << ", " << ray.getDirection().getZ() << ")" << std::endl;

			final_color = castRay(ray, objects, lights);

			// set pixel color
			m_img->setPixelColor(xPixel, m_y_res - (yPixel + 1), final_color.getX(), final_color.getY(), final_color.getZ());						
			
		}
	}
}

// Casts one ray per pixel and populates each pixel via ray-tracing and intersects
void Camera::castRays(std::vector<Object*>* objects, std::vector<Light*>* lights) {
	Utility util;

	Matrix44f viewMatrix = lookAt(m_position, m_forward, m_up);
	std::cout << viewMatrix.toString() << std::endl;

	float aspectRatio = static_cast<float>(m_x_res) / static_cast<float>(m_y_res);
	float scale = tan(util.toRadians(m_fov * 0.5f));

	float angleStep = m_fov / static_cast<float>(m_x_res); // Determine # steps across x-axis. Is XFOV / XRes

	float yFov = (static_cast<float>(m_y_res) / static_cast<float>(m_x_res)) * static_cast<float>(m_fov); // the y-angle FOV of the camera
	float yAngleStep = yFov / static_cast<float>(m_y_res); // Determine # steps across y-axis. Is YFOV / YRes

	Ray ray = Ray(m_position); // Create new ray
	Vector rayDir = Vector(); // create temporary directional vector


	rayDir.setZ(-1.0f); // Set Z direction to look down -1.0

	// For each pixel on the y-axis
	for (int yPixel = 0; yPixel < m_y_res; yPixel++) {

		// set the ray's direction to the y-axis tangent value
		float yTan = tan(util.toRadians((-yFov / 2.0f) + (yPixel * yAngleStep)));
		rayDir.setY(yTan);

		// For each pixel on the x-axis
		for (int xPixel = 0; xPixel < m_x_res; xPixel++) {
			// set the ray's direction to the y-axis tangent value
			float xTan = tan(util.toRadians((-m_fov / 2.0f) + (xPixel * angleStep)));
			xTan *= aspectRatio;
			rayDir.setX(xTan);

			rayDir = viewMatrix.multDirMatrix(rayDir);

			// Normalize the ray's direction
			rayDir.normalize3D();

			ray.setDirection(rayDir);

			// set the current draw distance value equal to the maximum draw distance
			float draw_t = t_far;

			// Create the final pixel color vector
			Vector final_color = Vector();

			// Create struct containing ray intersect info from the camera
			IntersectInfo IsectInfo;
			IsectInfo.t = 0.0f;

			//std::cout << "Ray sent from camera in heading (" << ray.getDirection().getX() << ", " << ray.getDirection().getY() << ", " << ray.getDirection().getZ() << ")" << std::endl;

			final_color = castRay(ray, objects, lights);

			// set pixel color
			m_img->setPixelColor(xPixel, m_y_res - (yPixel + 1), final_color.getX(), final_color.getY(), final_color.getZ());

		}
	}
}