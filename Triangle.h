#pragma once
#include "Plane.h"
class Triangle :
    public Plane
{
protected:

    Vector m_v0;
    Vector m_v1;
    Vector m_v2;

    float kEpsilon = 0.0f;
    bool m_single_sided = false; // determines if this triangle can only be viewed from one side

public:
    Triangle();
    Triangle(Vector v0, Vector v1, Vector v2);
    Triangle(Vector plane, Vector v0, Vector v1, Vector v2);

    void setVertex0(Vector v0);
    void setVertex1(Vector v1);
    void setVertex2(Vector v2);
    void setSingleSidedness(bool isSingleSided);

    Vector getVertex0();
    Vector getVertex1();
    Vector getVertex2();

    bool getSingleSidedness();

    Vector getNormal();

    // Get the edge between vertices 0 and 1
    Vector getEdge0();

    // Get the edge between vertices 0 and 2
    Vector getEdge1();

    // Get the edge between vertices 1 and 2
    Vector getEdge2();

    // Get the distance between vertices 0 and 1
    float getDistV0V1();

    // Get the distance from vertices 0 and 2
    float getDistV0V2();

    // Get the distance from vertices 1 and 2
    float getDistV1V2();

    // Returns true if the point passed in is on the triangle
    bool pointInTriangle(Vector P);

    // Intersection test
    bool intersectsTriangle(Vector& n, Vector& p0, Vector& l0, Vector& l, IntersectInfo& i);
};

