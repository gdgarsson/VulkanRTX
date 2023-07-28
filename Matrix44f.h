#pragma once
#include "Vector.h"
#include "Matrix33f.h"
class Matrix44f
{
private:
	Matrix44f mult44(const Matrix44f& m1);
	Matrix44f sub44(const Matrix44f& m1);

	// Helper functions
	Matrix44f scaleMat44ByFloat(const float& f);

	// Gets the minor matrix for the inversion process
	Matrix33f getMatrixMinor(int row, int col);
public:

	float m[4][4];

	// Initialize an Identity matrix
	Matrix44f();

	Matrix44f(float v00, float v01, float v02, float v03,
		float v10, float v11, float v12, float v13,
		float v20, float v21, float v22, float v23,
		float v30, float v31, float v32, float v33);

	Matrix44f operator*(const Matrix44f& m1);
	Matrix44f operator*(const float& f);
	Matrix44f operator-(const Matrix44f& m1);

	float getDeterminant();

	// Returns the inverse of this matrix
	Matrix44f getInverse();
	
	// Returns the cofactor of this matrix
	Matrix44f getCofactorMatrix();

	// Returns the adjoint version of this matrix
	Matrix44f getAdjointMatrix();

	// Returns the transpose of this 4x4 matrix
	Matrix44f getTranspose() const;
	
	// Used for point-matrix multiplication
	Vector multVecMatrix(Vector v);

	// Used for vector-matrix multiplication
	Vector multDirMatrix(Vector v);

	Matrix44f ortho(float left, float right, float bottom, float top, float near, float far);
	Matrix44f perspective(float fovY, float aspectRatio, float near, float far);

	std::string toString();
};

