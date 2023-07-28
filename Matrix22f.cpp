#include "Matrix22f.h"
#include <math.h>


Matrix22f::Matrix22f() {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			m[i][j] = 0.0f;
		}
	}
}

Matrix22f::Matrix22f(float v00, float v01,
	float v10, float v11) {
	m[0][0] = v00;
	m[0][1] = v01;

	m[1][0] = v10;
	m[1][1] = v11;
}

Matrix22f Matrix22f::operator*(const Matrix22f& m1)
{
	return Matrix22f(mult22(m1));
}

Matrix22f Matrix22f::operator*(const float& f)
{
	return Matrix22f(scaleMat22ByFloat(f));
}

Matrix22f Matrix22f::mult22(const Matrix22f& m1) {
	Matrix22f result = Matrix22f();

	for (int i = 0; i < 2; i++) { // Loop through this matrix's rows
		for (int j = 0; j < 2; j++) { // Loop through the other matrix's rows
			for (int k = 0; k < 2; k++) { // Loop through this matrix's columns
				result.m[i][j] += m[i][k] * m1.m[j][k];
			}
		}
	}

	return result;
}

Matrix22f Matrix22f::getInverse() {
	float determinant = getDeterminant();
	if (determinant == 0.0f) return Matrix22f(); // return empty matrix if unable to be inversed

	Matrix22f result = prepareForInvert();
	result = result * (1 / determinant);

	return result;
}

float Matrix22f::getDeterminant() {
	return (m[0][0] * m[1][1]) - (m[0][1] * m[1][0]);
}

Matrix22f Matrix22f::getCofactorMatrix() {
	Matrix22f result = Matrix22f();

	result.m[0][0] = m[0][0];
	result.m[0][1] = -m[0][1];

	result.m[1][0] = -m[1][0];
	result.m[1][1] = m[1][1];

	return result;
}

Matrix22f Matrix22f::getAdjointMatrix() {
	Matrix22f result = Matrix22f();

	result.m[0][0] = m[0][0];
	result.m[0][1] = m[1][0];
	
	result.m[1][0] = m[0][1];
	result.m[1][1] = m[1][1];

	return result;
}

Matrix22f Matrix22f::prepareForInvert() {
	Matrix22f cofactor = getCofactorMatrix();
	Matrix22f result = Matrix22f();

	// Note; this process is NOT creating an Adjoint Matrix!!!!
	result.m[0][0] = cofactor.m[1][1];
	result.m[0][1] = cofactor.m[0][1];

	result.m[1][0] = cofactor.m[1][0];
	result.m[1][1] = cofactor.m[0][0];

	return result;
}

Matrix22f Matrix22f::scaleMat22ByFloat(const float& f)
{
	Matrix22f result = Matrix22f();
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			result.m[i][j] = f * m[i][j];
		}
	}

	return result;
}
