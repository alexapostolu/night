#include "Matrix.h"

namespace ice
{
	Matrix::Matrix()
		: matrix{ {0} } {}

	// column major format (openGL)
	Matrix::Matrix(float a, float b, float c, float d, float e, float f, float g, float h, float i)
	{
		matrix[0] = a;
		matrix[3] = b;
		matrix[6] = c;
		matrix[1] = d;
		matrix[4] = e;
		matrix[7] = f;
		matrix[2] = g;
		matrix[5] = h;
		matrix[8] = i;
	}

	Matrix Matrix::operator+(float value) const
	{
		return Matrix(matrix[0] + value, matrix[1] + value, matrix[2] + value,
					  matrix[3] + value, matrix[4] + value, matrix[5] + value,
					  matrix[6] + value, matrix[7] + value, matrix[8] + value);
	}

	Matrix Matrix::operator-(float value) const
	{
		return Matrix(matrix[0] - value, matrix[1] - value, matrix[2] - value,
				      matrix[3] - value, matrix[4] - value, matrix[5] - value,
					  matrix[6] - value, matrix[7] - value, matrix[8] - value);
	}

	Matrix Matrix::operator*(float value) const
	{
		return Matrix(matrix[0] * value, matrix[1] * value, matrix[2] * value,
					  matrix[3] * value, matrix[4] * value, matrix[5] * value,
					  matrix[6] * value, matrix[7] * value, matrix[8] * value);
	}

	Matrix Matrix::operator/(float value) const
	{
		return Matrix(matrix[0] / value, matrix[1] / value, matrix[2] / value,
					  matrix[3] / value, matrix[4] / value, matrix[5] / value,
					  matrix[6] / value, matrix[7] / value, matrix[8] / value);
	}

	Matrix Matrix::operator+(const Matrix& mtx) const
	{
		return Matrix(matrix[0] + mtx.matrix[0], matrix[1] + mtx.matrix[1],
					  matrix[2] + mtx.matrix[2], matrix[3] + mtx.matrix[3],
					  matrix[4] + mtx.matrix[4], matrix[5] + mtx.matrix[5],
					  matrix[6] + mtx.matrix[6], matrix[7] + mtx.matrix[7],
					  matrix[8] + mtx.matrix[8]);
	}

	Matrix Matrix::operator-(const Matrix& mtx) const
	{
		return Matrix(matrix[0] - mtx.matrix[0], matrix[1] - mtx.matrix[1],
					  matrix[2] - mtx.matrix[2], matrix[3] - mtx.matrix[3],
					  matrix[4] - mtx.matrix[4], matrix[5] - mtx.matrix[5],
					  matrix[6] - mtx.matrix[6], matrix[7] - mtx.matrix[7],
					  matrix[8] - mtx.matrix[8]);
	}

	Matrix Matrix::operator*(const Matrix& mtx) const
	{
		return Matrix(
			(matrix[0] * mtx.matrix[0]) + (matrix[3] * mtx.matrix[1]) + (matrix[6] * mtx.matrix[2]), 
			(matrix[0] * mtx.matrix[3]) + (matrix[3] * mtx.matrix[4]) + (matrix[6] * mtx.matrix[5]),
			(matrix[0] * mtx.matrix[6]) + (matrix[3] * mtx.matrix[7]) + (matrix[6] * mtx.matrix[8]),

			(matrix[1] * mtx.matrix[0]) + (matrix[4] * mtx.matrix[1]) + (matrix[7] * mtx.matrix[2]),
			(matrix[1] * mtx.matrix[3]) + (matrix[4] * mtx.matrix[4]) + (matrix[7] * mtx.matrix[5]),
			(matrix[1] * mtx.matrix[6]) + (matrix[4] * mtx.matrix[7]) + (matrix[7] * mtx.matrix[8]),

			(matrix[2] * mtx.matrix[0]) + (matrix[5] * mtx.matrix[1]) + (matrix[8] * mtx.matrix[2]),
			(matrix[2] * mtx.matrix[3]) + (matrix[5] * mtx.matrix[4]) + (matrix[8] * mtx.matrix[5]),
			(matrix[2] * mtx.matrix[6]) + (matrix[5] * mtx.matrix[7]) + (matrix[8] * mtx.matrix[8]));
	}

	Matrix Matrix::operator/(const Matrix& mtx) const
	{
		return Matrix(matrix[0] / mtx.matrix[0], matrix[1] / mtx.matrix[1],
					  matrix[2] / mtx.matrix[2], matrix[3] / mtx.matrix[3],
					  matrix[4] / mtx.matrix[4], matrix[5] / mtx.matrix[5],
					  matrix[6] / mtx.matrix[6], matrix[7] / mtx.matrix[7],
					  matrix[8] / mtx.matrix[8]);
	}

	void Matrix::operator+=(float value)
	{
		matrix[0] += value;
		matrix[3] += value;
		matrix[6] += value;
		matrix[1] += value;
		matrix[4] += value;
		matrix[7] += value;
		matrix[2] += value;
		matrix[5] += value;
		matrix[8] += value;
	}

	void Matrix::operator-=(float value)
	{
		matrix[0] -= value;
		matrix[3] -= value;
		matrix[6] -= value;
		matrix[1] -= value;
		matrix[4] -= value;
		matrix[7] -= value;
		matrix[2] -= value;
		matrix[5] -= value;
		matrix[8] -= value;
	}

	void Matrix::operator*=(float value)
	{
		matrix[0] *= value;
		matrix[3] *= value;
		matrix[6] *= value;
		matrix[1] *= value;
		matrix[4] *= value;
		matrix[7] *= value;
		matrix[2] *= value;
		matrix[5] *= value;
		matrix[8] *= value;
	}

	void Matrix::operator/=(float value)
	{
		matrix[0] /= value;
		matrix[3] /= value;
		matrix[6] /= value;
		matrix[1] /= value;
		matrix[4] /= value;
		matrix[7] /= value;
		matrix[2] /= value;
		matrix[5] /= value;
		matrix[8] /= value;
	}

	void Matrix::operator+=(const Matrix& mtx)
	{
		matrix[0] += mtx.matrix[0];
		matrix[3] += mtx.matrix[0];
		matrix[6] += mtx.matrix[0];
		matrix[1] += mtx.matrix[0];
		matrix[4] += mtx.matrix[0];
		matrix[7] += mtx.matrix[0];
		matrix[2] += mtx.matrix[0];
		matrix[5] += mtx.matrix[0];
		matrix[8] += mtx.matrix[0];
	}

	void Matrix::operator-=(const Matrix& mtx)
	{
		matrix[0] -= mtx.matrix[0];
		matrix[3] -= mtx.matrix[0];
		matrix[6] -= mtx.matrix[0];
		matrix[1] -= mtx.matrix[0];
		matrix[4] -= mtx.matrix[0];
		matrix[7] -= mtx.matrix[0];
		matrix[2] -= mtx.matrix[0];
		matrix[5] -= mtx.matrix[0];
		matrix[8] -= mtx.matrix[0];
	}

	void Matrix::operator*=(const Matrix& mtx)
	{
		// '*=' will cause a stack overflow (infinite recursive function)
		// since '*=' is this function
		*this = *this * mtx;
	}

	void Matrix::operator/=(const Matrix& mtx)
	{
		matrix[0] /= mtx.matrix[0];
		matrix[3] /= mtx.matrix[0];
		matrix[6] /= mtx.matrix[0];
		matrix[1] /= mtx.matrix[0];
		matrix[4] /= mtx.matrix[0];
		matrix[7] /= mtx.matrix[0];
		matrix[2] /= mtx.matrix[0];
		matrix[5] /= mtx.matrix[0];
		matrix[8] /= mtx.matrix[0];
	}

	Vector3D Matrix::operator*(const Vector3D& vec) const
	{
		return Vector3D((matrix[0] * vec.x) + (matrix[3] * vec.y) + (matrix[6] * vec.z),
						(matrix[1] * vec.x) + (matrix[4] * vec.y) + (matrix[7] * vec.z),
						(matrix[2] * vec.x) + (matrix[5] * vec.y) + (matrix[8] * vec.z));
	}

	void Matrix::identity()
	{
		*this *= 0;

		matrix[0] = 1;
		matrix[4] = 1;
		matrix[8] = 1;
	}

	void Matrix::inverse() 
	{
		float t1 = matrix[0] * matrix[4];
		float t2 = matrix[0] * matrix[7];
		float t3 = matrix[3] * matrix[1];
		float t4 = matrix[6] * matrix[1];
		float t5 = matrix[3] * matrix[2];
		float t6 = matrix[6] * matrix[2];

		// determinant
		float det = (t1 * matrix[8]) - (t2 * matrix[5]) - (t3 * matrix[8]) +
			(t4 * matrix[5]) + (t5 * matrix[7]) - (t6 * matrix[4]);

		if (det == 0)
			return;

		float invd = 1 / det;

		matrix[0] = ((matrix[4] * matrix[8]) - (matrix[7] * matrix[5])) * invd;
		matrix[1] = -((matrix[1] * matrix[8]) - (matrix[7] * matrix[2])) * invd;
		matrix[2] = ((matrix[1] * matrix[5]) - (matrix[4] * matrix[2])) * invd;
		matrix[3] = -((matrix[3] * matrix[8]) - (matrix[6] * matrix[5])) * invd;
		matrix[4] = ((matrix[0] * matrix[8]) - t6) * invd;
		matrix[5] = -((matrix[0] * matrix[5]) - t5) * invd;
		matrix[6] = ((matrix[3] * matrix[7]) - (matrix[6] * matrix[4])) * invd;
		matrix[7] = -(t2 - t4) * invd;
		matrix[8] = (t1 - t3) * invd;
	}

	Matrix Matrix::transpose() const
	{
		return Matrix(matrix[0], matrix[3], matrix[6], matrix[1], matrix[4], matrix[7],
			matrix[2], matrix[5], matrix[8]);
	}
}