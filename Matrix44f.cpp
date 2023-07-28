#include "Matrix44f.h"
#include "Utility.h"
#include <iostream>

#define PI 3.14159265358979323846

Matrix44f::Matrix44f() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (i == j) {
				m[i][j] = 1.0f;
			}
			else {
				m[i][j] = 0.0f;
			}
		}
	}
}

Matrix44f::Matrix44f(float v00, float v01, float v02, float v03,
	float v10, float v11, float v12, float v13,
	float v20, float v21, float v22, float v23,
	float v30, float v31, float v32, float v33) {
	m[0][0] = v00;
	m[0][1] = v01;
	m[0][2] = v02;
	m[0][3] = v03;

	m[1][0] = v10;
	m[1][1] = v11;
	m[1][2] = v12;
	m[1][3] = v13;
	
	m[2][0] = v20;
	m[2][1] = v21;
	m[2][2] = v22;
	m[2][3] = v23;

	m[3][0] = v30;
	m[3][1] = v31;
	m[3][2] = v32;
	m[3][3] = v33;
}

Matrix44f Matrix44f::scaleMat44ByFloat(const float& f) {
	Matrix44f result = Matrix44f();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = f * m[i][j];
		}
	}

	return result;
}

Matrix33f Matrix44f::getMatrixMinor(int row, int col) {
	Matrix33f result = Matrix33f();

	int minorRow = 0, minorCol = 0; // row and column indices for the minor matrix
	for (int i = 0; i < 4; i++) { // iterate over the Mat33's rows
		if (i != row) { // if i != the excluded slot's row
			// begin copying into that column
			for (int j = 0; j < 4; j++) { // iterate over the Mat33's columns
				if (j != col) { // if j != the excluded slot's column
					result.m[minorRow][minorCol] = m[i][j]; // copy contents into minor matrix
					minorCol++; // increment minor column index
					if (minorCol >= 3) { // if minor column index exceeds minor matrix # columns
						minorCol = 0; // reset minor column index to zero
						minorRow++; // increment minor row index
					}
				}
			}
		}
	}

	return result;
}

Matrix44f Matrix44f::operator*(const Matrix44f& m1)
{
	return Matrix44f(mult44(m1));
}

Matrix44f Matrix44f::operator*(const float& f) {
	return Matrix44f(scaleMat44ByFloat(f));
}

Matrix44f Matrix44f::operator-(const Matrix44f& m1)
{
	return Matrix44f(sub44(m1));
}

Matrix44f Matrix44f::sub44(const Matrix44f& m1) {
	Matrix44f result = Matrix44f();

	for (int i = 0; i < 4; i++) { // Loop through this matrix's rows
		for (int j = 0; j < 4; j++) { // Loop through the other matrix's rows
			result.m[i][j] += m[i][j] - m1.m[i][j];
		}
	}

	return result;
}

Matrix44f Matrix44f::mult44(const Matrix44f& m1) {
	Matrix44f result = Matrix44f();

	for (int i = 0; i < 4; i++) { // Loop through this matrix's rows
		for (int j = 0; j < 4; j++) { // Loop through the other matrix's rows
			for (int k = 0; k < 4; k++) { // Loop through this matrix's columns
				result.m[i][j] += m[i][k] * m1.m[j][k];
			}
		}
	}

	return result;
}

float Matrix44f::getDeterminant() {
	float det = 0;
	for (int i = 0; i < 4; i++) {
		if (i % 2 == 0) {
			det += m[0][i] * getMatrixMinor(0, i).getDeterminant();
		}
		else {
			det -= m[0][i] * getMatrixMinor(0, i).getDeterminant();
		}
	}
	return det;
}

Matrix44f Matrix44f::getCofactorMatrix() {
	Matrix44f result = Matrix44f();

	result.m[0][0] = m[0][0];
	result.m[0][1] = -m[0][1];
	result.m[0][2] = m[0][2];
	result.m[0][3] = -m[0][3];

	result.m[1][0] = -m[1][0];
	result.m[1][1] = m[1][1];
	result.m[1][2] = -m[1][2];
	result.m[1][3] = m[1][3];

	result.m[2][0] = m[2][0];
	result.m[2][1] = -m[2][1];
	result.m[2][2] = m[2][2];
	result.m[2][3] = -m[2][3];

	result.m[3][0] = -m[3][0];
	result.m[3][1] = m[3][1];
	result.m[3][2] = -m[3][2];
	result.m[3][3] = m[3][3];

	return result;
}

Matrix44f Matrix44f::getAdjointMatrix() {
	Matrix44f result = Matrix44f();

	result.m[0][0] = m[0][0];
	result.m[0][1] = m[1][0];
	result.m[0][2] = m[2][0];
	result.m[0][3] = m[3][0];

	result.m[1][0] = m[0][1];
	result.m[1][1] = m[1][1];
	result.m[1][2] = m[2][1];
	result.m[1][3] = m[3][1];

	result.m[2][0] = m[0][2];
	result.m[2][1] = m[1][2];
	result.m[2][2] = m[2][2];
	result.m[2][3] = m[3][2];
	
	result.m[3][0] = m[0][3];
	result.m[3][1] = m[1][3];
	result.m[3][2] = m[2][3];
	result.m[3][3] = m[3][3];

	return result;
}

Matrix44f Matrix44f::getInverse() {
	Matrix44f inverse = Matrix44f();

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

Matrix44f Matrix44f::getTranspose() const {
	Matrix44f result = Matrix44f();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = m[j][i];
		}
	}

	return result;
}

Vector Matrix44f::multVecMatrix(Vector v) {
	Vector result = Vector();
	
	result.setX(v.getX() * m[0][0] + v.getY() * m[1][0] + v.getZ() * m[2][0] + m[3][0]);
	result.setY(v.getX() * m[0][1] + v.getY() * m[1][1] + v.getZ() * m[2][1] + m[3][1]);
	result.setZ(v.getX() * m[0][2] + v.getY() * m[1][2] + v.getZ() * m[2][2] + m[3][2]);
	
	float w = v.getX() * m[0][3] + v.getY() * m[1][3] + v.getZ() * m[2][3] + m[3][3];
	// Normalize if the 4th row of the matrix is not [0, 0, 0, 0] or [0, 0, 0, 1]
	if (w != 0.0f && w != 1.0f) {
		result.setX(result.getX() / w);
		result.setY(result.getY() / w);
		result.setZ(result.getZ() / w);
	}

	return result;
}

Vector Matrix44f::multDirMatrix(Vector v) {
	Vector result = Vector();

	result.setX(v.getX() * m[0][0] + v.getY() * m[1][0] + v.getZ() * m[2][0]);
	result.setY(v.getX() * m[0][1] + v.getY() * m[1][1] + v.getZ() * m[2][1]);
	result.setZ(v.getX() * m[0][2] + v.getY() * m[1][2] + v.getZ() * m[2][2]);

	return result;
}

Matrix44f Matrix44f::ortho(float left, float right, float bottom, float top, float near, float far)
{
	if (left == right) { std::cerr << "ortho(): left and right are equal\n"; return Matrix44f(); }
	if (bottom == top) { std::cerr << "ortho(): bottom and top are equal\n"; return Matrix44f(); }
	if (near == far) { std::cerr << "ortho(): near and far are equal\n"; return Matrix44f(); }

	Matrix44f result = Matrix44f();

	float w = right - left;
	float h = top - bottom;
	float d = far - near;

	result.m[0][0] = 2.0 / w;
	result.m[1][1] = 2.0 / h;
	result.m[2][2] = -2.0 / d;
	result.m[0][3] = -(left + right) / w;
	result.m[1][3] = -(top + bottom) / h;
	result.m[2][3] = -(near + far) / d;

	return result;
}

Matrix44f Matrix44f::perspective(float fovY, float aspectRatio, float near, float far) {
	Matrix44f result = Matrix44f();
	Utility u;

	float f = 1.0 / tan(u.toRadians(fovY) / 2);
	float d = far - near;

	
	result.m[0][0] = f / aspectRatio;
	result.m[1][1] = f;
	result.m[2][2] = -(near + far) / d;
	result.m[2][3] = -2 * near * far / d;
	result.m[3][2] = -1;
	result.m[3][3] = 0.0;

	return result;
}

std::string Matrix44f::toString() {
	std::string s = "";
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			s += std::to_string(m[i][j]);
			s += " ";
		}
		s += "\n";
	}

	return s;
}