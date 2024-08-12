#include "quadric.h"


void CQuadricData::fill(float input)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m_quadric[i][j] = input;
		}
	}
}

CQuadricData::~CQuadricData()
{
	//TODO: implement the constructor		
}

CQuadric::CQuadric(const vector<long> &faces_indices, const vector<vertex>& vertices)
{
	//TODO: implement the constructor		

}

CQuadric::~CQuadric()
{
	//TODO: implement the constructor		

}

std::vector<CQuadricData> CQuadric::vertex_quadric(CHalfEdge half_edge, float boundary_quadric_weight, double boundary_quadric_regularization)
{

	//TODO: implement the quadric error metric and calculate m_Qv vector for vertices		
	return m_Qv;
}

void CQuadric::optimal_location_and_cost(CQuadricData Qeij_, vertex& v_opt, float& cost)
{
	//TODO:	implement this function
	//calculate v_opt and cost
}