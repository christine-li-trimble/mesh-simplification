#pragma once
#include "half_edge.h"
#include <vector>
#include <numeric>

using namespace std;

class CQuadricData
{
public:
	CQuadricData(double value)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m_quadric[i][j] = value;
			}
		}
	}

	~CQuadricData(); // TODO: implement the destructor?

	// overload the + operator
	friend CQuadricData operator +(const CQuadricData& q1, const CQuadricData& q2)
	{
		CQuadricData temp(0.) ;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp.m_quadric[i][j] = q1.m_quadric[i][j] + q2.m_quadric[i][j] ;
			}
		}
		return temp ;
	}

	// Overload the * operator for scalar multiplication
	friend CQuadricData operator *(const CQuadricData& q, double scalar)
	{
		CQuadricData temp(0.0f);
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp.m_quadric[i][j] = q.m_quadric[i][j] * scalar;
			}
		}
		return temp;
	}

	// Overload the * operator for scalar multiplication (commutative version)
	friend CQuadricData operator *(double scalar, const CQuadricData& q)
	{
		return q * scalar;
	}

	void fill(double input);
	void set(double input, int i, int j);
	double get(int i, int j);

	
private:
	double m_quadric[4][4] ;
};

class CQuadric
{
public:
	CQuadric(const vector<long> & faces_indices, const vector<vertex> &vertices);
	~CQuadric(); // TODO: implement the destructor



	std::vector<CQuadricData> vertex_quadric(CHalfEdge half_edge, float boundary_quadric_weight, double boundary_quadric_regularization);
	void optimal_location_and_cost(CQuadricData Qeij_, vertex& v_opt, double& cost);


private:
	// For Tina's implementation
	bool solve_linear_system(const double A[3][3], const vertex& b, vertex& v_opt);
	bool inverse(const double A[3][3], double invA[3][3]);
	double determinant(const double A[3][3]);

	std::vector<CQuadricData> m_Qv;
	std::vector<CQuadricData> m_Qf ;
	std::vector<long> F;
	std::vector<vertex> V;
};