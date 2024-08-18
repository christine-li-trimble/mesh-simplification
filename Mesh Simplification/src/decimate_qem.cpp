#include <SketchUpAPI/sketchup.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>
#include "decimate_qem.h"
#include "half_edge.h"
#include "quadric.h"
#include <cassert>
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
#include <set>
#include <limits>

using namespace std;


// ========================================
// global constants QEM
// ========================================

static constexpr float INF_COST = numeric_limits<float>::max();
static constexpr long GHOST_HALF_EDGE = numeric_limits<long>::max() - 5;
static constexpr long INVALID_HALF_EDGE = numeric_limits<long>::max() - 1;
static constexpr long INVALID_VERTEX_INDEX = numeric_limits<long>::max() - 2;
static constexpr long INVALID_EDGE = numeric_limits<long>::max() - 4;
static constexpr vertex GHOST_VERTEX_LOCATION = { 0,0,0 };

struct edge_to_collapse
{
	float cost;
	long cur_collapse;
	long i_he;
	long i_vertex;
	long j_vertex;
	vertex v_opt;
};


void remove_unreferenced(vector<long> &faces_indices, vector<vertex> &vertices)
{
	// remove unferenced indices from vertices list
	// Step 1: Mark all vertices that are referenced by a face
	vector<bool> is_referenced(vertices.size(), false);

	// Iterate through faces_indices and mark referenced vertices
	for (long idx : faces_indices)
	{
		if (idx >= 0 && idx < vertices.size())  // Ensure the index is valid
		{
			is_referenced[idx] = true;
		}
	}

	// Step 2: Create a mapping from old vertex indices to new vertex indices
	vector<long> old_to_new(vertices.size(), -1);
	vector<vertex> new_vertices;

	long new_index = 0;

	for (long i = 0; i < vertices.size(); ++i)
	{
		if (is_referenced[i])
		{
			old_to_new[i] = new_index++;  // Map old index to new index
			new_vertices.push_back(vertices[i]);  // Add the referenced vertex to new_vertices
		}
	}

	// Replace old vertices with the new compacted vertices
	vertices = std::move(new_vertices);

	// Step 3: Update faces_indices to use the new vertex indices
	for (long& idx : faces_indices)
	{
		if (idx >= 0 && idx < old_to_new.size())  // Ensure the index is valid
		{
			idx = old_to_new[idx];
		}
	}
}

/// <summary>
/// Main Decimation loop using QEM
/// </summary>
/// <param name="faces_indices">face vetex indices, size of total face * 3</param>
/// <param name="vertices">vertices </param>
/// <param name="num_targe_vertices"> target vertex number for decimation</param>
void decimate_qem(vector<long> faces_indices, vector<vertex> vertices,
	long num_targe_vertices, 
	int print_every_iterations, 
	float boundary_quadric_weight,
	double boundary_quadric_regularization)
{


	// basic geometry quantities
	long nHe = static_cast<long>( faces_indices.size()) ;
	long nV = static_cast<long>(vertices.size());


	// ========================================
	// build half edge data structure
	// ========================================
	//twins
	//tips
	// twins, tips = build_implicit_half_edges(F, return_v2he=False)
	CHalfEdge half_edge(faces_indices, vertices);

	// ========================================
	// calculate vertex Quadric 
	// ========================================
	CQuadric quadric(faces_indices, vertices);
	vector<CQuadricData> Qv = quadric.vertex_quadric(half_edge, boundary_quadric_weight, boundary_quadric_regularization);

	// ========================================
	// Initialization
	// ========================================
	// decimation parameter
	long total_collapse = nV - num_targe_vertices;
	long cur_collapse = 0;

	// compute initial edge quadrics
	unique_ptr<bool[]> he_processed = make_unique<bool[]>(nHe);
	unique_ptr<long[]> he2e = make_unique<long[]>(nHe);
	for (long i = 0; i < nHe; i++)
	{
		he_processed[i] = false;
		he2e[i] = -1;
	}

	long edge_Index = 0;
	vector<struct edge_to_collapse> edges_heap;

	for (long i = 0; i < nHe; i++)
	{
		if (he_processed[i])
		{
			long i_twin = half_edge.twin(i); // get the twin of the half edge
			he2e[i] = he2e[i_twin];  // set the edge index of the twin
		}
		else
		{
			long i_vertex = half_edge.tail_vertex(i);
			long j_vertex = half_edge.tip_vertex(i);
			CQuadricData Qi = Qv[i_vertex];
			CQuadricData Qj = Qv[j_vertex];
			CQuadricData Qeij = Qi + Qj;

			float cost;
			vertex v_opt;

			// find optimal location and cost
			quadric.optimal_location_and_cost(Qeij, v_opt, cost);
			edges_heap.push_back({ cost, cur_collapse, i, i_vertex, j_vertex, v_opt });

			// add processed half edge
			he_processed[i] = true;
			if (!half_edge.is_boundary_half_edge(i))
			{
				he_processed[half_edge.twin(i)] = true;
			}

			// construct he2e
			he2e[i] = edge_Index;
			edge_Index++;
		}
	}

	// build a minheap for the collapsed edges
	make_heap (edges_heap.begin(), edges_heap.end(), [](const struct edge_to_collapse &a, const struct edge_to_collapse &b) {return a.cost > b.cost; });

	// ========================================
	// Start Decimation
	// ========================================

	// Initialize time stamps
	long nEdge = edge_Index;
	unique_ptr<long[]> edge_time_stamps = make_unique<long[]>(nEdge);
	for (long i = 0; i < nEdge; i++)edge_time_stamps[i] = 0;

	// Start decimation loop
	while (cur_collapse < total_collapse)
	{
		// get the edge with the min cost
		// in the tutorial, this was achieve by a min heap

		pop_heap(edges_heap.begin(), edges_heap.end(), [](const struct edge_to_collapse &a, const struct edge_to_collapse &b) {return a.cost > b.cost; });
		// CHECK if this edge info is valid
		// collapse the edge
		float min_cost = edges_heap.back().cost;
		long i_he = edges_heap.back().i_he;
		long time_stamp = edges_heap.back().cur_collapse;
		long i_vertex = edges_heap.back().i_vertex;
		long j_vertex = edges_heap.back().j_vertex;
		vertex v_opt = edges_heap.back().v_opt;
		edges_heap.pop_back();

		// if edge has been removed
		long min_cost_edge = he2e[i_he];
		if(min_cost_edge == INVALID_EDGE)continue;

		// if cost is obsolete
		if (time_stamp != edge_time_stamps[min_cost_edge])continue;

		if (fabs(min_cost - INF_COST) < 1e-6)
		{
		    printf("encounter INF cost, cannot be decimate further\n");
			break;
		}

		// check if collapse is valid
		if (!half_edge.is_collapse_valid(vertices, i_he, v_opt))
		{
		    edge_time_stamps[min_cost_edge] = cur_collapse;

			edges_heap.push_back({ INF_COST, cur_collapse, i_he, i_vertex, j_vertex, v_opt });
			push_heap(edges_heap.begin(), edges_heap.end(), [](const struct edge_to_collapse &a, const struct edge_to_collapse &b) {return a.cost > b.cost; });

			continue;
		}

		//=================================================

		// Notation
		//          k
        //         /  \         
        //        /    \     
		// hennt / henn \
        //      /     hen\hent     
        //     /    he    \  
        //    i------------j
        //     \    het   /   
		// hetnt\hetn    /
        //       \ hetnn/hetnnt       
        //        \    /         
        //         \  /       
        //          l
		long i_hen = half_edge.next(i_he);
		long i_henn = half_edge.next(i_hen);
		long i_hent = half_edge.twin(i_hen); // could be ghost half edge
		long i_hennt = half_edge.twin(i_henn); // could be ghost half edge
		long i_het = half_edge.twin(i_he); // could be ghost half edge
		long i_hetn, i_hetnn, i_hetnt, i_hetnnt;

		if (i_het == GHOST_HALF_EDGE)
		{
			// if he is a boundary half edge
			i_hetn = GHOST_HALF_EDGE;
			i_hetnn = GHOST_HALF_EDGE;
			i_hetnt = GHOST_HALF_EDGE;
			i_hetnnt = GHOST_HALF_EDGE;
		}
		else
		{
			i_hetn = half_edge.next(i_het);
			i_hetnn = half_edge.next(i_hetn);
			i_hetnt = half_edge.twin(i_hetn); // could be ghost half edge
			i_hetnnt = half_edge.twin(i_hetnn); // could be ghost half edge;
		}


		// start post edge collapse
		cur_collapse++;

		//print progress
		if (cur_collapse % print_every_iterations == 0)
		{
			printf("Decimating %d/%d\n", cur_collapse, total_collapse);
		}

		// topo update tips (we always keep vertex i)
		vector<long> he_list_tip_i = half_edge.vertex_one_ring_half_edges_from_half_edge(i_henn, false);
		vector<long> he_list_tip_j = half_edge.vertex_one_ring_half_edges_from_half_edge(i_he, false);

		for (const long &he : he_list_tip_j) half_edge.update_tip(he, i_vertex); //we always keep vertex i
		for (const long &he : he_list_tip_i) half_edge.update_tip(he, i_vertex); //we always keep vertex i
	
		// topo update twins
		half_edge.update_twin(i_hennt, i_hent);
		half_edge.update_twin(i_hetnt, i_hetnnt);

		// move vertex
		vertices[i_vertex] = v_opt;
		vertices[j_vertex] = GHOST_VERTEX_LOCATION;

		// update vertex quadrics
		Qv[i_vertex] = Qv[i_vertex] + Qv[j_vertex];
		Qv[j_vertex].fill(INF_COST);

		// remove info from tips and twins
		for (const long& he : set<long>({ i_he, i_hen, i_henn, i_het, i_hetn, i_hetnn }))
		{
			half_edge.update_tip(he, INVALID_VERTEX_INDEX);
			half_edge.update_twin(he, INVALID_HALF_EDGE);
		}

		// update the he2e list
		if (i_hetn != GHOST_HALF_EDGE)	
			he2e[i_hetnnt] = he2e[i_hetn];
		for (const long& he : set<long>({ i_he, i_hen, i_henn, i_het, i_hetn, i_hetnn }))
			he2e[i_hetnn] = INVALID_EDGE;

		// update costs for edge one-ring half-edges
		long i_he_new = min(i_hennt, i_hetnnt);
		i_he_new = min(i_he_new, half_edge.next(i_hent));
		assert(half_edge.tail_vertex(i_he_new) == i_vertex);

		vector<long> he_list_new = half_edge.vertex_one_ring_half_edges_from_half_edge(i_he_new, true);

		CQuadricData Qi_ = Qv[i_vertex];
		for (const long& he : he_list_new)
		{
			long j_vertex_ = half_edge.tip_vertex(he);
			CQuadricData Qj_ = Qv[j_vertex_];
			CQuadricData Qeij_ = Qi_ + Qj_;

			float cost_;
			vertex v_opt_;

			// find optimal location and cost
			quadric.optimal_location_and_cost(Qeij_, v_opt_, cost_);

			edges_heap.push_back({ cost_, cur_collapse, he, i_vertex, j_vertex_, v_opt_ });
			push_heap(edges_heap.begin(), edges_heap.end(), [](const struct edge_to_collapse& a, const struct edge_to_collapse& b) {return a.cost > b.cost; });

			edge_time_stamps[he2e[he]] = cur_collapse;
		}

		// if i is a bounary vertex, then we need to update one more edge
		if(half_edge.is_boundary_vertex_from_half_edge(he_list_new.back()))
		{
			long he = half_edge.next(half_edge.next(he_list_new.back()));
			long j_vertex_ = half_edge.tail_vertex(he);

			CQuadricData Qj_ = Qv[j_vertex_];
			CQuadricData Qeij_ = Qi_ + Qj_;

			float cost_;
			vertex v_opt_;

			// find optimal location and cost
			quadric.optimal_location_and_cost(Qeij_, v_opt_, cost_);

			// make sure(j_, i) is swapped here because the half edge is in the opposite direction
			edges_heap.push_back({ cost_, cur_collapse, he, j_vertex_, i_vertex, v_opt_ });
			push_heap(edges_heap.begin(), edges_heap.end(), [](const struct edge_to_collapse& a, const struct edge_to_collapse& b) {return a.cost > b.cost; });

			edge_time_stamps[he2e[he]] = cur_collapse;
		}
	}

	// ========================================
	// update the face indices from half edge data structure
	half_edge.face_from_half_edge_data(faces_indices);
	// remove unferenced vertices
	remove_unreferenced(faces_indices, vertices);

}

