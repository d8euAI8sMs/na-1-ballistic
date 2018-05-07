#pragma once

/**
 * Defines vector and matrix arithmetics.
 */

#include "kScience/kScience.h"

kScienceNSHeader()
	
typedef struct {
	double x, y, z;

	const double& operator[](int i) const
	{
		ASSERT(i == 0 || i == 1 || i == 2);
		return i == 0 ? x : i == 1 ? y : z;
	}
	double& operator[](int i)
	{
		ASSERT(i == 0 || i == 1 || i == 2);
		return i == 0 ? x : i == 1 ? y : z;
	}
} v3;

inline v3 operator+(const v3 &first, const v3 &second)
{
	return{ first.x + second.x, first.y + second.y, first.z + second.z};
}

inline v3 operator-(const v3 &first, const v3 &second)
{
	return{ first.x - second.x, first.y - second.y, first.z - second.z };
}

inline v3 operator-(const v3 &first)
{
	return{ -first.x, -first.y, -first.z };
}

inline v3 operator*(const v3 &first, double n)
{
	return{ n * first.x, n * first.y, n * first.z };
}

inline v3 operator*(double n, const v3 &first)
{
	return first * n;
}

// Vector product
inline v3 operator^(const v3 &first, const v3 &second)
{
	return{
		first.y * second.z - first.z * second.y,
		first.z * second.x - first.x * second.z,
		first.x * second.y - first.y * second.x
	};
}

// Scalar product
inline double operator*(const v3 &first, const v3 &second)
{
	return first.x * second.x + first.y * second.y + first.z * second.z;
}

inline v3 operator/(const v3 &first, double n)
{
	return{ first.x / n, first.y / n, first.z / n };
}

inline double norm(const v3 &first)
{
	return sqrt(first.x * first.x + first.y * first.y + first.z * first.z);
}

// Performs transformation from (r, theta, phi) to (x, y, z)
// where r >= 0, theta in [-pi/2, pi/2], phi in [-pi, pi]
inline v3 spherical2rect(const v3 &polar, bool deg = false)
{
	double m = (deg ? (acos(0) * 2) / 180 : 1);
	return{
		polar.x * cos(m * polar.y) * cos(m * polar.z),
		polar.x * cos(m * polar.y) * sin(m * polar.z),
		polar.x * sin(m * polar.y)
	};
}

// Performs transformation to (r, theta, phi) from (x, y, z)
inline v3 rect2spherical(const v3 &rect, bool deg = false)
{
	double m = (deg ? 180 / (acos(0) * 2) : 1);
	double r = norm(rect);
	return{
		// >= 0
		r,
		// in [-pi/2, pi/2] as r >= 0 that gives atan2(y <=> 0, x >= 0)
		m * atan2(rect.z, sqrt(rect.x * rect.x + rect.y * rect.y)),
		// in [-pi, pi] as that gives atan2(y <=> 0, x <=> 0)
		m * atan2(rect.y, rect.x)
	};
}





typedef struct {
	v3 c1, c2, c3; // Rows

	const v3& operator[](int i) const
	{
		ASSERT(i == 0 || i == 1 || i == 2);
		return i == 0 ? c1 : i == 1 ? c2 : c3;
	}

	v3& operator[](int i)
	{
		ASSERT(i == 0 || i == 1 || i == 2);
		return i == 0 ? c1 : i == 1 ? c2 : c3;
	}
} m3;

// Matrix * Vector
inline v3 operator*(const m3 &first, const v3 &second)
{
	return 
	{
		first.c1.x * second.x + first.c1.y * second.y + first.c1.z * second.z,
		first.c2.x * second.x + first.c2.y * second.y + first.c2.z * second.z,
		first.c3.x * second.x + first.c3.y * second.y + first.c3.z * second.z
	};
}

// Matrix */ Number
inline m3 operator*(const m3 &first, double a)
{
	return{ first.c1 * a, first.c2 * a, first.c3 * a };
}

inline m3 operator*(double a, const m3 &first)
{
	return first * a;
}

inline m3 operator/(const m3 &first, double a)
{
	return{ first.c1 / a, first.c2 / a, first.c3 / a };
}

// Transpose
inline m3 operator~(const m3 &first)
{
	return
	{
		{ first.c1.x, first.c2.x, first.c3.x },
		{ first.c1.y, first.c2.y, first.c3.y },
		{ first.c1.z, first.c2.z, first.c3.z },
	};
}

// Invert
inline m3 operator!(const m3 &first)
{
	// 0. Inverse matrix is given as A^-1 = adj(A) / det(A)
	// where adj(A) is adjugate matrix:
	// adj(A){j,i} = { [(-1)^(i+j)] * M{i,j} },
	// M{i,j} is an (i,j) minor.
	m3 inv; double det = 0;
	// 1. Calculate adj(A){j,i} and accumulate det(A)
	for (int i = 0; i < 3; i++) // rows
	{
		for (int j = 0; j < 3; j++) // cols
		{
			// 1.1 Calculate M{i,j} of matrix m3\{i,j} -- 2x2 minor
			// Using a trick
			double det2[2] = { 1, 1 }; int d = 0;
			for (int k1 = 0; k1 < 3; k1++) // rows
			{
				if (k1 == i) continue;
				for (int k2 = 0; k2 < 3; k2++) // cols
				{
					if (k2 == j) continue;
					det2[d % 2] *= first[k1][k2];
					d++;
				}
				d = 1;
			}
			// 1.2 Calculate (-1)^(i+j)*M{i,j}
			inv[j][i] = (((i + j) % 2) == 0 ? 1 : -1)
				* (det2[0] - det2[1]);
			// 1.3 Accumulate det(m3) -- 1st row expansion
			if (i == 0)
			{
				det += (((i + j) % 2) == 0 ? 1 : -1)
					* first[i][j] * (det2[0] - det2[1]);
			}
		}
	}
	// 2. Return result
	return inv / det;
}

// Matrix * Matrix
inline m3 operator*(const m3 &first, const m3 &second)
{
	m3 t = ~second;
	return
	{
		{ first.c1 * t.c1, first.c1 * t.c2, first.c1 * t.c3 },
		{ first.c2 * t.c1, first.c2 * t.c2, first.c2 * t.c3 },
		{ first.c3 * t.c1, first.c3 * t.c2, first.c3 * t.c3 }
	};
}

inline m3 identity()
{
	return{
			{ 1, 0, 0 },
			{ 0, 1, 0 },
			{ 0, 0, 1 }
	};
}

inline m3 rotate_x(double angle)
{
	return{
			{ 1, 0, 0 },
			{ 0, cos(angle), -sin(angle) },
			{ 0, sin(angle), cos(angle) }
	};
}

inline m3 rotate_y(double angle)
{
	return{
			{ cos(angle), 0, sin(angle) },
			{ 0, 1, 0 },
			{ -sin(angle), 0, cos(angle) }
	};
}

inline m3 rotate_z(double angle)
{
	return{
			{ cos(angle), -sin(angle), 0 },
			{ sin(angle), cos(angle), 0 },
			{ 0, 0, 1 }
	};
}

// Perform the given transformation in the specified base
// Suppose that the base consists of rows,
// each is a base vector.
inline m3 transform_in(const m3 &base, const m3 &transform)
{
	m3 base_t = ~base;
	return base_t * transform * !base_t;
}

// Calculate transformation to align the given axis
// with the direction. Or, in other words, calculate
// rotation matrix to rotate the axis to the direction
// in orthonormal base.
inline m3 align_axis(const v3 &axis, const v3 &direction)
{
	v3 ax_n = axis / norm(axis); // normalized
	v3 d_n = direction / norm(direction); // normalized
	// normal vector -- the rotation axis
	v3 n_n = (direction ^ axis) / norm(direction ^ axis); // normalized

	// cosine of the angle between the axis and the direction
	double alpha = ax_n * d_n;

	// normal vector to two others
	v3 v_n = d_n - alpha * ax_n;
	v_n = v_n / norm(v_n); // normalized

	m3 base = m3{ ax_n, v_n, n_n };
	
	return transform_in(base, rotate_z(acos(alpha)));
}

// Calculate transformation to align the given axes
// with the given directions. Or, in other words, calculate
// transformation matrix to rotate two axes to the given two directions.
// The initial base is base0. Suppose that the base consists of rows,
// each is a base vector.
inline m3 align_axes(const m3 &base0, int axis1, int axis2,
	const v3 &direction1, const v3 direction2)
{
	// Rotate the axis1 to the direction1 first
	m3 align_m1 = align_axis(base0[axis1], direction1);
	// Go into the new base
	m3 aligned_base = ~(align_m1 * (~base0));
	
	// Calculate rotation
	double align_2_rot_angle =
		// If triple product is positive
		(((aligned_base[axis2] ^ direction2) * aligned_base[axis1]) > 0 ? 1 : -1) *
		acos(aligned_base[axis2] * direction2 / norm(direction2));

	return transform_in(aligned_base, rotate_z(align_2_rot_angle)) * align_m1;
}

kScienceNSFooter()