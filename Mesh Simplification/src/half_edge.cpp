#include <algorithm>
#include <vector>
#include "half_edge.h"
#include <map>
#include <iostream>
#include <unordered_set>

using namespace std;

CHalfEdge::CHalfEdge(const vector<long> & faces_indices, const vector<vertex> &vertices)
{
	
	// initialize the half edge data structure
	std::map<std::pair<long, long>, size_t> index_from_halfedges;

	// For each triangle face, iterate over three edges and create three halfedges
	for (int face = 0; face < faces_indices.size() / 3; ++face)
	{
		// getting all halfedges from the mesh:
		int v0 = faces_indices[3 * face];
		int v1 = faces_indices[3 * face + 1];
		int v2 = faces_indices[3 * face + 2];

		// New Halfedges: v0v1, v1v2, v2v0
		size_t prev_halfedges_len = tips.size();
		// mapping vertices to halfedges indices. The number of tips counts how many
		// halfedges were already created
		// creating halfedge index for the vertices pair v0v1
		index_from_halfedges.insert({ std::pair<long,long>(v0, v1), tips.size() });
		tails.push_back(v0);
		tips.push_back(v1);
		// creating the next of each halfedge. The indices are the number 
		// of halfedges already created until the moment + a relative index (that ranges from 0 to 3,
		// since the triangle has 3 vertices)
		// getting the next of the first halfedge of the current triangle face. 1%3 is equal to 1,
		// so, 1 is summed, since this is the first halfedge of the triangle
		nexts.push_back(static_cast<long>(prev_halfedges_len) + (1 % 3));
		// getting the face that the current halfedge 'points' to
		adjacent_faces.push_back(face);

		// creating halfedge index for the vertices pair v1v2
		index_from_halfedges.insert({ std::pair<long, long>(v1, v2), tips.size() });
		tails.push_back(v1);
		tips.push_back(v2);
		// getting the next of the second halfedge of the current triangle face. 2%3 is equal to 2,
		// so, 2 is summed, since this is the second halfedge of the triangle	
		nexts.push_back(static_cast<long>(prev_halfedges_len) + (2 % 3));
		// getting the face that the current halfedge 'points' to
		adjacent_faces.push_back(face);

		// creating halfedge index for the vertices pair v2v0. Note that we are returning
		// to the first vertex of the triangle face:
		index_from_halfedges.insert({ std::pair<long, long>(v2, v0), tips.size() });
		tails.push_back(v2);
		tips.push_back(v0);
		// getting the next of the first halfedge of the current triangle face. 3%3 is equal to 0,
		// so, 0 is summed, since this is the starting halfedge of the triangle
		nexts.push_back(static_cast<long>(prev_halfedges_len) + (3 % 3));
		// getting the face that the current halfedge 'points' to
		adjacent_faces.push_back(face);
	}
	
	twins.resize(tips.size(), -1);
	is_boundary_halfedge.resize(tips.size(), false);
	for (const auto& [halfedge, index]: index_from_halfedges)
	{
		// Current halfedge with tail vA and tip vB and index X...
		// Search for halfedge with tail vB and tip vA and index Y
		// -> if this halfedge exists, they are twins!
		// -> if this halfedge doesn't exist, then the current halfedge is a boundary edge
		int vA = halfedge.first;
		int vB = halfedge.second;
		auto it = index_from_halfedges.find(std::pair<long, long>(vA, vB));
		if (it != index_from_halfedges.end())
		{
			// the halfedge is not on the boundary,
			// and a twin was found
			// twins[vAvB = index and twins[index = vAvB
			twins[it->second] = static_cast<long>(index);
			twins[index] = static_cast<long>(it->second);
		}
		else
		{
			// no twin was found, implying the halfedge is in the boundary of the mesh
			is_boundary_halfedge[index] = true;
		}
	}

	// Get the current number of halfedges -- this represents all halfedges created so far,
	// which doesn't include the twins of border halfedges
	// The next loop creates the twins of border halfedges and place their attributes at the end of
	// the vectors that define the halfedges (tips, tails, nexts etc)
	long num_interior_halfedges = static_cast<long>(tips.size());
	for (long i = 0; i < num_interior_halfedges; ++i)
	{
		if (is_boundary_halfedge[i])
		{
			// Current (border) halfedge has tail vA and tip vB
			// Create twin halfedge which has tail vB and tip vA
			// A new halfedge was just created, so it is necesary to put a new entry in the the vectors
			// that implicitly represent the halfedges
			int vA = tails[i];
			int vB = tips[i];

			// The current number of halfedges can be used as an index for the new halfedge
			size_t new_halfedge_index = twins.size();
			// Insert the index into the map created before, this will be helpful to find
			// the next halfedge of a boundary halfedge
			index_from_halfedges.insert({ std::pair<long, long>(vB, vA), new_halfedge_index });
			tails.push_back(vB); // tip of halfedge i is the tail of the twin halfedge
			tips.push_back(vA); // tail of the halfedge i is the tip of the twin halfedge
			// Border halfedge doesn't have adjacent triangle counter-clockwise, so put a placeholder value
			adjacent_faces.push_back(-1); 

			// According to this documentation: https://www.graphics.rwth-aachen.de/media/openmesh_static/Documentations/OpenMesh-6.2-Documentation/a00032.html#nav_bound
			// a halfedge is only a boundary if it is not adjacent to a face
			// this means that current halfedge is not really on the boundary, but its twin is, 
			// so mark current halfedge as not boundary and mark new halfedge as boundary
			is_boundary_halfedge[i] = false;
			is_boundary_halfedge.push_back(true); // put twin halfedge, which is on boundary
			
			// twins[new halfedge] = i
			// twins[i] = new halfedge index
			twins[i] = static_cast<long>(new_halfedge_index);
			twins.push_back(i); 			 
			
			// Delay computation of the next halfedges because these halfedges may not exist yet
			// So a placeholder value -1 means that the next halfedge wasn't computed yet
			nexts.push_back(-1); 
		}
	}

	// Now that all the boundary halfedges were created,
	// compute their nexts halfedges
	for (int i = num_interior_halfedges; i < tips.size(); ++i)
	{
		if (is_boundary_halfedge[i])
		{
			// the next of a boundary halfedge is equal to
			// the twin of next of the next of its twin...
			// that is, next[i] = twin[next[next[twin[i]];
			// Easier to draw than to write:
			/*
			
			      A
			    /   \ 
			  /       \  
			B --------> C
			next of BC should be CA, which is the same as computing:
			1- twin of BC == CB
			2- next of the twin of BC == next of CB == BA 
			3- next of the next of the twin of BC == next of BA == AC
			twin of the next of the next of the twin of BC = CA
			which is the answer...
			*/
			nexts[i] = twins[nexts[nexts[twins[i]]]];
		}
	}
}

CHalfEdge::~CHalfEdge()
{
	//TODO: implement this method
	// do we need to implement this? doesnt std::vector free its own memory automatically in c++ 20?
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
	vertex = tails[i_he];
	return vertex;
}

// return the head_vertex (tip) of the current half edge
long CHalfEdge::tip_vertex(long i_he)
{
	long vertex=0;
	//TODO: implement this method
	vertex = tips[i_he];
	return vertex;
}

// return whether the current edge is a boundary half edge
bool CHalfEdge::is_boundary_half_edge(long i_he)
{
	bool is_boundary = false;
	//TODO: implement this method
	is_boundary = is_boundary_halfedge[i_he];
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
	twins[i_he] = i_twin;
}

long CHalfEdge::next(long i_he)
{
	long value=0;
	//TODO: implement this method
	value = nexts[i_he];
	return value;
}

vector<long> CHalfEdge::vertex_one_ring_half_edges_from_half_edge(long i_he, bool around_tail)
{
	vector<long> vertices_one_ring;
	
	long he_start = i_he;


	if (around_tail) {
		// rotating around the tail of the half edge
		vertices_one_ring.push_back(he_start);
		while (true) {
			// getting the counter-clockwise half-edge around the tail vertex
			he_start = twin(next(next(he_start)));
			if (he_start == i_he) {
				// getting back to the starting vertex
				break;
			}
			if (he_start == -1) {
				// hitted a boundary, start clockwise traversal
				he_start = i_he;
				while (true) {
					if (twin(he_start) == -1) {
						break;
					}
					he_start = next(twin(he_start));
					vertices_one_ring.insert(vertices_one_ring.begin(), he_start);
				}
				break;
			}
			vertices_one_ring.push_back(he_start);
		}
	}
	else {
		// rotating around the tip of the half edge
		vertices_one_ring.push_back(i_he);
		while (true) {
			if (twin(i_he) == -1) {
				// hitted a boundary, start clockwise traversal
				i_he = next(i_he);
				while (true) {
					i_he = twin(next(i_he));
					if (i_he == -1) {
						// boundary is reached
						return vertices_one_ring;
					}
					vertices_one_ring.insert(vertices_one_ring.begin(), i_he);
				}
			}

			// getting the counter-clockwise half-edge around the tip vertex
			i_he = next(next(twin(i_he)));
			if (i_he == he_start) {
				// completed the ring around the tip vertex
				return vertices_one_ring;
			}
			vertices_one_ring.push_back(i_he);
		}
	}

	return vertices_one_ring;
}


bool CHalfEdge::is_boundary_vertex_from_half_edge(long i_he)
{
	bool is_boundary_vertex = false;
	//TODO: implement this method
	// According to https://github.com/odedstein/sgi-introduction-course/blob/main/007_boundary/007_boundary.md
	// "A vertex is a boundary vertex if it is contained in a boundary edge. Otherwise, it is an interior vertex."
	// Does the same applies to the halfedge? If so, I think it is...
	is_boundary_vertex = is_boundary_half_edge(i_he);
	return is_boundary_vertex;
}

void CHalfEdge::face_from_half_edge_data(std::vector<long> &faces_indices)
	// for each halfedge, retrieves the vertices of the face that the current halfedge belongs to
{
	//TODO: implement this method
	// update face_indices vector that passed in as reference according to the halfedge data structure

	// Get the number of halfedges; any of the vectors that implicitly represent the halfedges could be used
	size_t num_halfedges = tips.size();
	for (size_t i = 0; i < num_halfedges; ++i)
	{
		int current_face = adjacent_faces[i];
		if (current_face == -1)
		{
			// Skip boundary halfedges, no adjacent face
			continue;
		}
		
		if (3 * (current_face + 1) > faces_indices.size())
		{
			faces_indices.resize(3 * (current_face + 1));
		}
		// getting the vertices of each face by "walking" on the face through v0, v1 and finally ending up in v2
		int v0 = tips[i];
		int v1 = tips[nexts[i]];
		int v2 = tips[nexts[nexts[i]]];

		// faces_indices is a vector that stores vertices related to the faces.
		// The first 3 positions determine the first face; the next 3 positions determine another face; and so on
		// a face is composed by vertices Vn, Vn+1 and Vn+2
		// the current_face variable is only an offset to traverse the vector in every 3 to 3 positions
		faces_indices[3 * current_face] = v0;
		faces_indices[3 * current_face + 1] = v1;
		faces_indices[3 * current_face + 2] = v2;
	}
}

vector<long> CHalfEdge::vertex_one_ring_vertices_from_half_edge( long i_he) {

	vector<long> one_ring_he = vertex_one_ring_half_edges_from_half_edge(i_he, true);
	vector<long> one_ring_v;

	for (long half_edge : one_ring_he) {
		// getting the tip vertex of each half-edge
		long tip_v = tip_vertex(half_edge);
		one_ring_v.push_back(tip_v);

		// checking if the vertex is on the boundary 
		if (twin(next(next(half_edge))) == -1) {
			tip_v = tip_vertex(next(half_edge));
			one_ring_v.push_back(tip_v);
		}
	}

	// removing duplicated vertices if necessary
	sort(one_ring_v.begin(), one_ring_v.end());
	one_ring_v.erase(unique(one_ring_v.begin(), one_ring_v.end()), one_ring_v.end());

	return one_ring_v;
}


bool CHalfEdge::is_collapse_valid(const vector<vertex>& vertices, long i_he, vertex v_opt, bool verbose)
{
	bool is_valid = true;
	// Condition 1:
	// Boundary constraint: If v1 and v2 are boundary vertices, then the edge (v1, v2) has to be a boundary edge
	bool is_i_boundary_vertex = is_boundary_vertex_from_half_edge(i_he);
	bool is_j_boundary_vertex = is_boundary_vertex_from_half_edge(next(i_he));
	bool is_he_boundary_half_edge = is_boundary_half_edge(i_he);

	if (is_i_boundary_vertex && is_j_boundary_vertex && !is_he_boundary_half_edge) {
		
		cout << "Decimation violates the boundary topological constraint, skip it" << endl;
		
		return false;
	}

	// Condition 2:
	// Consider the intersection of the one-ring neighbourhoods of v1 and v2 (v1 and v2 are an edge)
	// the intersection of the one-rings neighbourhoods of both v1 and v2 contains exactly two vertices in the case of
	// a non - boundary edge and exactly one vertex in the case of a boundary edge

	vector<long> one_ring_v_i = vertex_one_ring_vertices_from_half_edge(i_he);
	vector<long> one_ring_v_j = vertex_one_ring_vertices_from_half_edge(next(i_he));

	std::unordered_set<long> intersected_v;
	// getting the intersection of vertices between the one-ring neighbourhood of
	// both vi and vj vertex (vi and vj form an edge)
	for (long v : one_ring_v_i) {
		if (find(one_ring_v_j.begin(), one_ring_v_j.end(), v) != one_ring_v_j.end()) {
			intersected_v.insert(v);
		}
	}
	size_t num_intersected_v = intersected_v.size();

	if (!is_he_boundary_half_edge && num_intersected_v != 2) {
		if (verbose) {
			cout << "Decimation violates internal link condition, skip it" << std::endl;
		}
		return false;
	}

	if (is_he_boundary_half_edge && num_intersected_v != 1) {
		if (verbose) {
			cout << "Decimation violates boundary link condition, skip it" << endl;
		}
		return false;
	}
	// TODO: implement conditions 3 and 4
	return is_valid;
}
