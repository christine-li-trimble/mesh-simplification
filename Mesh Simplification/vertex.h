#pragma once
#include <cmath>
struct vertex
{
	double x, y, z;

	// constructors
	vertex() : x(0), y(0), z(0) {}
	vertex(double x, double y, double z) : x(x), y(y), z(z) {}
	vertex(const double arr[3]) : x(arr[0]), y(arr[1]), z(arr[2]) {} // for Tina's implementation

	bool operator==(const vertex& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	// Operator overloading for subtraction
	vertex operator-(const vertex& v) const {
		return vertex(x - v.x, y - v.y, z - v.z);
	}

	// Operator overloading for addition
	vertex operator+(const vertex& v) const {
		return vertex(x + v.x, y + v.y, z + v.z);
	}

	// Operator overloading for scalar multiplication
	vertex operator*(double scalar) const {
		return vertex(x * scalar, y * scalar, z * scalar);
	}

	// Dot product
	double dot(const vertex& v) const {
		return x * v.x + y * v.y + z * v.z;
	}

	// Cross product
	vertex cross(const vertex& v) const {
		return vertex(
			y * v.z - z * v.y,
			z * v.x - x * v.z,
			x * v.y - y * v.x
		);
	}

	// Normalize the vector
	vertex normalize() const {
		double magnitude = std::sqrt(x * x + y * y + z * z);
		if (magnitude > 0) {
			return vertex(x / magnitude, y / magnitude, z / magnitude);
		}
		return vertex(0, 0, 0);  // Return a zero vector if magnitude is zero
	}
};