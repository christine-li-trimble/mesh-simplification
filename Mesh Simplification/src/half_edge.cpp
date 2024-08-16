#include <algorithm>
#include <vector>
#include "half_edge.h"

using namespace std;

CHalfEdge::CHalfEdge(const vector<long> & faces_indices, const vector<vertex> &vertices)
{
	//TODO: implement this method
	// initialize the half edge data structure
}

CHalfEdge::~CHalfEdge()
{
	//TODO: implement this method
}

size_t CHalfEdge::size()
{
	return twins.size();
}

// return the twin half edge of the current half edge
long CHalfEdge::twin(long i_he)
{
	// TODO: check this method
	long value = twins[i_he];
	return value;
}

// return the tail_vertex of the current half edge
long CHalfEdge::tail_vertex(long i_he)
{
	long vertex=0;
	//TODO: implement this method
	return vertex;
}

// return the head_vertex of the current half edge
long CHalfEdge::tip_vertex(long i_he)
{
	long vertex=0;
	//TODO: implement this method
	return vertex;
}

// return whether the current edge is a boundary half edge
bool CHalfEdge::is_boundary_half_edge(long i_he)
{
	bool is_boundary = false;
	//TODO: implement this method
	return is_boundary;
}

void CHalfEdge::update_tip(long i_he, long i_tip)
{
	// TODO: check this method
	tips[i_he] = i_tip;
}

void CHalfEdge::update_twin(long i_he, long i_twin)
{
	//TODO: implement this method
}

long CHalfEdge::next(long i_he)
{
	long value=0;
	//TODO: implement this method
	return value;
}

vector<long> CHalfEdge::vertex_one_ring_half_edges_from_half_edge(long i_he, bool around_tail)
{
	vector<long> vertices;
	//TODO: implement this method
	return vertices;
}

bool CHalfEdge::is_boundary_vertex_from_half_edge(long i_he)
{
	bool is_boundary_vertex = false;
	//TODO: implement this method
	return is_boundary_vertex;
}

void CHalfEdge::face_from_half_edge_data(std::vector<long> &faces_indices)
{
	//TODO: implement this method
	// update face_indices vector that passed in as reference according to the halfedge data structure
}

bool CHalfEdge::is_collapse_valid(const vector<vertex>& vertices, long i_he, vertex v_opt)
{
	bool is_valid = true;
	//TODO:	implement this function
	return is_valid;
}
