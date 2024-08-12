#pragma once
struct vertex
{
	double x, y, z;
	bool operator==(const vertex& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}
};