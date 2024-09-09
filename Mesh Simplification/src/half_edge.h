#pragma once
#include <memory>
#include <vector>
#include "vertex.h"

using namespace std;

class CHalfEdge
{
public:
	CHalfEdge(const vector<long> &faces_indices, const vector<vertex> &vertices);
	~CHalfEdge();

	long twin(long i_he);
	long tail_vertex(long i_he);
	long tip_vertex(long i_he);
	bool is_boundary_half_edge(long i_he);
	void update_tip(long i_he, long i_tip);
	void update_twin(long i_he, long i_he_twin);
	long next(long i_he);
	vector<long>vertex_one_ring_half_edges_from_half_edge(long i_he, bool around_tail = true);
	vector<long> vertex_one_ring_vertices_from_half_edge(long i_he);
	bool is_collapse_valid(const vector<vertex>& vertices, long i_he, vertex v_opt, bool verbose);
	bool is_boundary_vertex_from_half_edge(const long & i_he);
	void face_from_half_edge_data(std::vector<long>& faces_indices);

	// Method to determine the number of half-edges
	size_t size();
private:
	vector<long> twins;
	vector<long> tips;

	// New attributes
	vector<long> tails;
	//vector<long> nexts;
	vector<long> adjacent_faces;
	vector<bool> is_boundary_halfedge;
};
