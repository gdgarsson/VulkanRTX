#include "Triangle.h"
#include "Utility.h"

Triangle::Triangle() {
	m_plane = Vector();
}

Triangle::Triangle(Vector v0, Vector v1, Vector v2) {
	m_plane = Vector();
	m_v0 = v0;
	m_v1 = v1;
	m_v2 = v2;
}

Triangle::Triangle(Vector plane, Vector v0, Vector v1, Vector v2) {
	m_plane = plane;
	m_v0 = v0;
	m_v1 = v1;
	m_v2 = v2;
}

void Triangle::setVertex0(Vector v0) {
	m_v0 = v0;
}

void Triangle::setVertex1(Vector v1) {
	m_v1 = v1;
}

void Triangle::setVertex2(Vector v2) {
	m_v2 = v2;
}

void Triangle::setSingleSidedness(bool isSingleSided) {
	m_single_sided = isSingleSided;
}

Vector Triangle::getVertex0() {
	return m_v0;
}

Vector Triangle::getVertex1() {
	return m_v1;
}

Vector Triangle::getVertex2() {
	return m_v2;
}

bool Triangle::getSingleSidedness() {
	return m_single_sided;
}

Vector Triangle::getNormal() {
	Utility u;

	Vector A = getEdge0();
	Vector B = getEdge1();

	Vector C = u.cross3D(A, B);
	C.normalize3D();

	return C;
}

// Get the edge between points 0 and 1
Vector Triangle::getEdge0() {
	Vector e1 = m_v1 - m_v0;
	return e1;
}

// Get the edge between points 0 and 2
Vector Triangle::getEdge1() {
	Vector e1 = m_v2 - m_v0;
	return e1;
}

// Get the edge between points 1 and 2
Vector Triangle::getEdge2() {
	Vector e1 = m_v2 - m_v1;
	return e1;
}

// Get the distance from point 0 to point 1
float Triangle::getDistV0V1() {
	Utility u;
	return u.distance3D(m_v0, m_v1);
}

// Get the distance from point 0 to point 2
float Triangle::getDistV0V2() {
	Utility u;
	return u.distance3D(m_v0, m_v2);
}

// Get the distance from point 1 to point 2
float Triangle::getDistV1V2() {
	Utility u;
	return u.distance3D(m_v1, m_v2);
}


bool Triangle::pointInTriangle(Vector P) {
	Utility u;
	
	Vector e0 = m_v1 - m_v0;
	Vector e1 = m_v2 - m_v1;
	Vector e2 = m_v0 - m_v2;

	Vector C0 = P - m_v0;
	Vector C1 = P - m_v1;
	Vector C2 = P - m_v2;

	if (u.dot3D(getNormal(), u.cross3D(e0, C0)) > 0 &&
		u.dot3D(getNormal(), u.cross3D(e1, C1)) > 0 &&
		u.dot3D(getNormal(), u.cross3D(e2, C2)) > 0) {
		return true; // P is inside the triangle
	}

	return false; // P is not inside the triangle
}

// Intersection test for the triangle
//   n is the plane normal - you can get this from the triangle with getNormal()
//   p is the plane's point relative to the world origin
//   l0 is the ray origin
//   l is the ray direction
//   i is where information gathered from the Intersection test is stored
bool Triangle::intersectsTriangle(Vector& n, Vector& p0, Vector& l0, Vector& l, IntersectInfo& i) {
	Utility u;

	float area2 = getNormal().pythagorean3D();
	
	float NdotRayDir = u.dot3D(n, l);
	if (fabs(NdotRayDir) < kEpsilon) { // the absolute value of the normal and ray dir dot product is less than 0
		return false; // ray and plane are parallel, will never intersect. Return false
	}

	// compute D and t
	float D = u.dot3D(n, m_v0);
	float t = -(u.dot3D(n, l0) + D) / NdotRayDir;

	if (t < 0.0f) return false; // if t < 0, triangle is behind the ray, return false

	// compute intersection point
	Vector Phit = l0 + (l * t);

	// determine if Phit is inside the triangle, return false if not
	if (!pointInTriangle(Phit)) {
		return false;
	}

	// determine if single-sided
	if (u.dot3D(l, n) && m_single_sided) {
		return false;
	}

	// if all tests pass, ray intersects triangle, return true
	i.PHit = Phit;
	i.NHit = getNormal();
	return true;
}