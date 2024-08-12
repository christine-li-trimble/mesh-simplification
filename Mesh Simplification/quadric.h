#pragma once
#include "half_edge.h"
#include <vector>

using namespace std;

// TODO: implement the quadric error metric

class CQuadricData
{
public:
	CQuadricData(float value)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m_quadric[i][j] = value;
			}
		}
	}

	~CQuadricData(); // TODO: implement the destructor

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

	void fill(float input);

	
private:
	float m_quadric[4][4] ;
};

class CQuadric
{
public:
	CQuadric(const vector<long> & faces_indices, const vector<vertex> &vertices); // TODO: implement the constructor
	~CQuadric(); // TODO: implement the destructor


	std::vector<CQuadricData> vertex_quadric(CHalfEdge half_edge, float boundary_quadric_weight, double boundary_quadric_regularization);
	void optimal_location_and_cost(CQuadricData Qeij_, vertex& v_opt, float& cost);


private:
	std::vector<CQuadricData> m_Qv;
	std::vector<CQuadricData> m_Qf ;


};