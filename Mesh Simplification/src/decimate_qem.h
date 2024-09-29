#include <vector>
#include "vertex.h"

using namespace std;

void decimate_qem(vector<long> & faces_indices, vector<vertex>& vertices,
	long num_targe_vertices,
	double triangle_quality_threshold = 0.1,
	int print_every_iterations = 500,
	float boundary_quadric_weight = 1.0,
	double boundary_quadric_regularization = 1.e-6, 
	bool verbose = true);

