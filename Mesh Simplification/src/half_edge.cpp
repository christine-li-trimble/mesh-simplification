#include <algorithm>
#include <vector>
#include "half_edge.h"
#include <map>

using namespace std;

CHalfEdge::CHalfEdge(const vector<long> & faces_indices, const vector<vertex> &vertices)
{
	//TODO: implement this method
	// initialize the half edge data structure
	std::map<std::pair<long, long>, int> index_from_halfedges;

	// For each triangle face, iterate over three edges and create three halfedges
	for (int face = 0; face < faces_indices.size() / 3; ++face)
	{
		// getting all halfedges from the mesh:
		int v0 = faces_indices[3 * face];
		int v1 = faces_indices[3 * face + 1];
		int v2 = faces_indices[3 * face + 2];

		// New Halfedges: v0v1, v1v2, v2v0
		int prev_halfedges_len = tips.size();
		// mapping vertices to halfedges indices. The number of tips counts how many
		// halfedges were already created
		// creating halfedge index for the vertices pair v0v1
		index_from_halfedges.insert(std::pair<long, long>(v0, v1), tips.size());
		tails.push_back(v0);
		tips.push_back(v1);
		// creating the next of each halfedge. The indices are the number 
		// of halfedges already created until the moment + a relative index (that ranges form 0 to 3,
		// since the triangle has 3 vertices)
		// getting the next of the first halfedge of the current triangle face. 1%3 is equal to 1,
		// so, 1 is summed, since this is the first halfedge of the triangle
		nexts.push_back(prev_halfedges_len + (1 % 3));
		// getting the face that the current halfedge 'points' to
		adjacent_faces.push_back(face);

		// creating halfedge index for the vertices pair v1v2
		index_from_halfedges.insert(std::pair<long, long>(v1, v2), tips.size());
		tails.push_back(v1);
		tips.push_back(v2);
		// getting the next of the second halfedge of the current triangle face. 2%3 is equal to 2,
		// so, 2 is summed, since this is the second halfedge of the triangle	
		nexts.push_back(prev_halfedges_len + (2 % 3));
		// getting the face that the current halfedge 'points' to
		adjacent_faces.push_back(face);

		// creating halfedge index for the vertices pair v2v0. Note that we are returning
		// to the first vertex of the triangle face:
		index_from_halfedges.insert(std::pair<long, long>(v2, v0), tips.size());
		tails.push_back(v2);
		tips.push_back(v0);
		// getting the next of the first halfedge of the current triangle face. 3%3 is equal to 0,
		// so, 0 is summed, since this is the starting halfedge of the triangle
		nexts.push_back(prev_halfedges_len + (3 % 3));
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
			twins[it->second] = index;
			twins[index] = it->second;
		}
		else
		{
			// boundary:
			// tail vB, 
			// tip vA
			// next == flipped version of twin.next.next 
			// 
			// twin = current halfedge
			// twins[index] = ??;
			// twins[??] = index;
			// adjacent_faces.push_back(-1);
			tails.push_back(vB);
			tips.push_back(vA);
			//nexts.push_back(twins[nexts[nexts[indexed]]]); // next == "flipped" (twin) version of twin.next.next
			adjacent_faces.push_back(-1); // No counter-clockwise face at this boundary halfedge
			is_boundary_halfedge[index] = true;

			// TODO... finish case of boundary halfedge...
		}
	}


}

CHalfEdge::~CHalfEdge()
{
	//TODO: implement this method
	// do we need to implement this? doesnt std::vector free its own memory automatically in c++ 20?
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

// return the head_vertex of the current half edge
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
