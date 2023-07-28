#include "Matrix33f.h"
#include <math.h>


Matrix33f::Matrix33f() {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			m[i][j] = 0.0f;
		}
	}
}

Matrix33f::Matrix33f(float v00, float v01, float v02,
					 float v10, float v11, float v12,
					 float v20, float v21, float v22) {
	m[0][0] = v00;
	m[0][1] = v01;
	m[0][2] = v02;

	m[1][0] = v10;
	m[1][1] = v11;
	m[1][2] = v12;
	
	m[2][0] = v20;
	m[2][1] = v21;
	m[2][2] = v22;
}

Matrix33f Matrix33f::operator*(const Matrix33f& m1)
{
	return Matrix33f(mult33(m1));
}

Matrix33f Matrix33f::operator*(const float& f)
{
	return Matrix33f(scaleMat33ByFloat(f));
}

Matrix33f Matrix33f::mult33(const Matrix33f& m1) {
	Matrix33f result = Matrix33f();

	for (int i = 0; i < 3; i++) { // Loop through this matrix's rows
		for (int j = 0; j < 3; j++) { // Loop through the other matrix's rows
			for (int k = 0; k < 3; k++) { // Loop through this matrix's columns
				result.m[i][j] += m[i][k] * m1.m[j][k];
			}
		}
	}

	return result;
}

Matrix33f Matrix33f::scaleMat33ByFloat(const float& f)
{
	Matrix33f result = Matrix33f();
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.m[i][j] = f * m[i][j];
		}
	}

	return result;
}

float Matrix33f::getDeterminant() {
	float det = 0;
	for (int i = 0; i < 3; i++) {
		if (i % 2 == 0) {
			det += m[0][i] * getMatrixMinor(0, i).getDeterminant();
		}
		else {
			det -= m[0][i] * getMatrixMinor(0, i).getDeterminant();
		}
	}
	return det;
}

Matrix22f Matrix33f::getMatrixMinor(int row, int col) {
	Matrix22f result = Matrix22f();

	int minorRow = 0, minorCol = 0; // row and column indices for the minor matrix
	for (int i = 0; i < 3; i++) { // iterate over the Mat33's rows
		if (i != row) { // if i != the excluded slot's row
			// begin copying into that column
			for (int j = 0; j < 3; j++) { // iterate over the Mat33's columns
				if (j != col) { // if j != the excluded slot's column
					result.m[minorRow][minorCol] = m[i][j]; // copy contents into minor matrix
					minorCol++; // increment minor column index
					if (minorCol >= 2) { // if minor column index exceeds minor matrix # columns
						minorCol = 0; // reset minor column index to zero
						minorRow++; // increment minor row index
					}
				}
			}
		}
	}

	return result;
		
}

Matrix33f Matrix33f::getCofactorMatrix()
{
	Matrix33f result = Matrix33f();

	result.m[0][0] = m[0][0];
	result.m[0][1] = -m[0][1];
	result.m[0][2] = m[0][2];

	result.m[1][0] = -m[1][0];
	result.m[1][1] = m[1][1];
	result.m[1][2] = -m[1][2];

	result.m[2][0] = m[2][0];
	result.m[2][1] = -m[2][1];
	result.m[2][2] = m[2][2];

	return result;
}

Matrix33f Matrix33f::getAdjointMatrix() {
	Matrix33f result = Matrix33f();

	result.m[0][0] = m[0][0];
	result.m[0][1] = m[1][0];
	result.m[0][2] = m[2][0];

	result.m[1][0] = m[0][1];
	result.m[1][1] = m[1][1];
	result.m[1][2] = m[2][1];

	result.m[2][0] = m[0][2];
	result.m[2][1] = m[1][2];
	result.m[2][2] = m[2][2];

	return result;
}

Matrix33f Matrix33f::getInverse() {
	Matrix33f inverse = Matrix33f();

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			inverse.m[i][j] = getMatrixMinor(i, j).getDeterminant();
		}
	}

	inverse = inverse.getCofactorMatrix();
	inverse = inverse.getAdjointMatrix();
	inverse = inverse * inverse.getDeterminant();

	return inverse;
}
