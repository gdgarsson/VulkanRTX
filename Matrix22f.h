#pragma once
class Matrix22f
{

private:
	Matrix22f mult22(const Matrix22f& m1);

	// Helper functions
	Matrix22f prepareForInvert();
	Matrix22f scaleMat22ByFloat(const float& f);
public:

	float m[2][2];

	Matrix22f();
	Matrix22f(float v00, float v01,
		float v10, float v11);


	Matrix22f operator*(const Matrix22f& m1);
	Matrix22f operator*(const float& f);

	Matrix22f getInverse();
	Matrix22f getCofactorMatrix();
	Matrix22f getAdjointMatrix();
	float getDeterminant();
};

