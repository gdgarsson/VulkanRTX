#pragma once
#include "Vector.h"
#include "Matrix22f.h"
class Matrix33f
{
private:
	Matrix33f mult33(const Matrix33f& m1);

	// Helper functions
	Matrix33f scaleMat33ByFloat(const float& f);

	// Gets the minor matrix for the inversion process
	Matrix22f getMatrixMinor(int row, int col);
public:

	float m[3][3];

	Matrix33f();
	Matrix33f(float v00, float v01, float v02,
		float v10, float v11, float v12,
		float v20, float v21, float v22);


	Matrix33f operator*(const Matrix33f& m1);
	Matrix33f operator*(const float& f);

	Matrix33f getCofactorMatrix();
	Matrix33f getAdjointMatrix();
	Matrix33f getInverse();
	float getDeterminant();
};

