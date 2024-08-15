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
			is_boundary_halfedge[index] = true;
		}
	}

	// Get the current number of halfedges -- this represents all halfedges created so far,
	// which doesn't include the twins of border halfedges
	// The next loop creates the twins of border halfedges and place their attributes at the end of
	// the vectors that define the halfedges (tips, tails, nexts etc)
	int num_interior_halfedges = tips.size();
	for (int i = 0; i < num_interior_halfedges; ++i)
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
			int new_halfedge_index = twins.size();
			// Insert the index into the map created before, this will be helpful to find
			// the next halfedge of a boundary halfedge
			index_from_halfedges.insert(std::pair<long, long>(vB, vA), new_halfedge_index);
			tails.push_back(vB); // tip of halfedge i is the tail of the twin halfedge
			tips.push_back(vA); // tail of the halfedge i is the tip of the twin halfedge
			// Border halfedge doesn't have adjacent triangle counter-clockwise, so put a placeholder value
			adjacent_faces.push_back(-1); 

			// According to this documentation: https://www.graphics.rwth-aachen.de/media/openmesh_static/Documentations/OpenMesh-6.2-Documentation/a00032.html#nav_bound
			// a halfedge is only a boundary if it is not adjacent to a face
			// this means that current halfedge is not really on the boundary, but its twin is
			// so mark current halfedge as not boundary and mark new halfedge as boundary
			is_boundary_halfedge[i] = false;
			is_boundary_halfedge.push_back(true); // put twin halfedge, which is on boundary
			
			// twins[new halfedge] = i
			// twins[i] = new halfedge index
			twins[i] = new_halfedge_index;
			twins.push_back(index); 			 
			
			// Delay computation of the next halfedges because these halfedges may not exist yet
			// So a placeholder value -1 means that the next halfedge wasn't computed yet
			nexts.push_back(-1); 
		}
	}

	// Now thah all the boundary halfedges were created,
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
			twin of BC = CB
			next of the twin of BC = BA
			next of the next of the twin of BC = AC
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
	// According to https://github.com/odedstein/sgi-introduction-course/blob/main/007_boundary/007_boundary.md
	// "A vertex is a boundary vertex if it is contained in a boundary edge. Otherwise, it is an interior vertex."
	// Does the same applies to the halfedge? If so, I think it is...
	is_boundary_vertex = is_boundary_half_edge(i_he);
	return is_boundary_vertex;
}

void CHalfEdge::face_from_half_edge_data(std::vector<long> &faces_indices)
{
	//TODO: implement this method
	// update face_indices vector that passed in as reference according to the halfedge data structure

	// Get the number of halfedges; any of the vectors that implicitly represent the halfedges could be used
	int num_halfedges = tips.size();
	for (int i = 0; i < num_halfedges; ++i)
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

		int v0 = tips[i];
		int v1 = tips[nexts[i]];
		int v2 = tips[nexts[nexts[i]]];
		faces_indices[3 * current_face] = v0;
		faces_indices[3 * current_face + 1] = v1;
		faces_indices[3 * current_face + 2] = v2;
	}
}

bool CHalfEdge::is_collapse_valid(const vector<vertex>& vertices, long i_he, vertex v_opt)
{
	bool is_valid = true;
	//TODO:	implement this function
	return is_valid;
}
