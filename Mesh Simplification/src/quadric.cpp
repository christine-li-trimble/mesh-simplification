#include "quadric.h"
#include <iostream> //Tina's implementation
#include <cmath> //Tina's implementation

void CQuadricData::fill(double input)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m_quadric[i][j] = input;
		}
	}
}

void CQuadricData::set(double input, int i, int j)
{
    m_quadric[i][j] = input;
}

double CQuadricData::get(int i, int j)
{
    return m_quadric[i][j];
}

CQuadricData::~CQuadricData()
{
	//TODO: implement the constructor?
}

CQuadric::CQuadric(const vector<long> &faces_indices, const vector<vertex>& vertices)
{
    F = faces_indices;
    V = vertices;
}

CQuadric::~CQuadric()
{
	//TODO: implement the constructor?

}

std::vector<CQuadricData> CQuadric::vertex_quadric(CHalfEdge half_edge, float boundary_quadric_weight, double boundary_quadric_regularization)
{
    // Initialize face quadrics
    size_t nF = F.size() / 3;
    m_Qf.resize(nF, CQuadricData(0.0f));

    // Initialize vertex quadrics
    size_t nV = V.size();
    m_Qv.resize(nV, CQuadricData(0.0f));

    // face area
    std::vector<double> FA(nF);

    for (long f = 0; f < nF; ++f)
    {
        long v0_idx = F[3 * f + 0];
        long v1_idx = F[3 * f + 1];
        long v2_idx = F[3 * f + 2];

        const vertex& v0 = V[v0_idx];
        const vertex& v1 = V[v1_idx];
        const vertex& v2 = V[v2_idx];

        // Compute the normal of the face
        vertex v01 = v1 - v0;
        vertex v02 = v2 - v0;
        vertex normal = v01.cross(v02);
        FA[f] = 0.5f * std::sqrt(normal.dot(normal));  // Triangle area
        normal = normal.normalize();

        // Compute the plane equation for the face
        double d = -normal.dot(v0);
        double plane[4] = { normal.x, normal.y, normal.z, d };

        // Compute the face quadric
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                m_Qf[f].set(plane[i] * plane[j], i, j);
            }
        }
    }

    //// Compute and normalize face areas
    //// Normalize face areas
    //double mean_FA = std::accumulate(FA.begin(), FA.end(), 0.0) / FA.size();
    //for (long f = 0; f < nF; ++f)
    //{
    //    FA[f] /= mean_FA;  // Normalize so the mean is 1
    //}

    // Scale face quadrics by normalized areas divided by 3
    for (long f = 0; f < nF; ++f)
    {
        double scaling_factor = FA[f] / 3.0f;

        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                m_Qf[f].set(m_Qf[f].get(i, j) * scaling_factor, i, j);
            }
        }

        // Add the scaled face quadric back to the vertex quadrics
        long v0_idx = F[3 * f + 0];
        long v1_idx = F[3 * f + 1];
        long v2_idx = F[3 * f + 2];

        // Sum face quadrics to compute vertex quadrics
        m_Qv[v0_idx] = m_Qv[v0_idx] + m_Qf[f];
        m_Qv[v1_idx] = m_Qv[v1_idx] + m_Qf[f];
        m_Qv[v2_idx] = m_Qv[v2_idx] + m_Qf[f];
    }

    size_t nHe = half_edge.size();

    for (int he = 0; he < nHe; ++he)
    {
        if (half_edge.is_boundary_half_edge(he))
        {
            long v_tip_idx = half_edge.tip_vertex(he);
            long v_tail_idx = half_edge.tail_vertex(he);
            long v_opp_index = half_edge.tip_vertex(half_edge.next(he));

            const vertex& v_tip = V[v_tip_idx];
            const vertex& v_tail = V[v_tail_idx];
            const vertex& v_opp = V[v_opp_index];

            // Compute face normal
            vertex fn = (v_tip - v_tail).cross(v_opp - v_tail);
            fn = fn.normalize();

            // Compute half edge normal
            vertex he_n = (v_tip - v_tail).cross(fn);
            he_n = he_n.normalize();

            // Compute half edge plane equation
            double he_d = -he_n.dot(v_tail);
            double he_p[4] = { he_n.x, he_n.y, he_n.z, he_d };

            // Compute half edge quadric
            CQuadricData he_K(0.0f);
            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    he_K.set(he_p[i] * he_p[j], i, j);
                }
            }

            // Edge length
            double edge_length = (v_tip - v_tail).length();

            // Add to vertex quadrics
            m_Qv[v_tail_idx] = m_Qv[v_tail_idx] + boundary_quadric_weight * edge_length * edge_length * he_K;
            m_Qv[v_tip_idx] = m_Qv[v_tip_idx] + boundary_quadric_weight * edge_length * edge_length * he_K;
        }
    }

    return m_Qv;
	//return m_Qv;
}

// For Tina's implementation
double CQuadric::determinant(const double A[3][3]) {
    return A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1])
        - A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0])
        + A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);
}

// For Tina's implementation
bool CQuadric::inverse(const double A[3][3], double invA[3][3]) {
    double det = determinant(A);
    if (fabs(det) < 1e-9) {
        // Matrix is singular
        return false;
    }

    double invDet = 1.0 / det;

    invA[0][0] = (A[1][1] * A[2][2] - A[1][2] * A[2][1]) * invDet;
    invA[0][1] = (A[0][2] * A[2][1] - A[0][1] * A[2][2]) * invDet;
    invA[0][2] = (A[0][1] * A[1][2] - A[0][2] * A[1][1]) * invDet;

    invA[1][0] = (A[1][2] * A[2][0] - A[1][0] * A[2][2]) * invDet;
    invA[1][1] = (A[0][0] * A[2][2] - A[0][2] * A[2][0]) * invDet;
    invA[1][2] = (A[0][2] * A[1][0] - A[0][0] * A[1][2]) * invDet;

    invA[2][0] = (A[1][0] * A[2][1] - A[1][1] * A[2][0]) * invDet;
    invA[2][1] = (A[0][1] * A[2][0] - A[0][0] * A[2][1]) * invDet;
    invA[2][2] = (A[0][0] * A[1][1] - A[0][1] * A[1][0]) * invDet;

    return true;
}

// Function to solve the linear system A * v_opt = b
// For Tina's implementation
bool CQuadric::solve_linear_system(const double A[3][3], const vertex& b, vertex& v_opt) {
    double invA[3][3];
    if (!inverse(A, invA)) {
        // A is singular
        return false;
    }

    // Compute v_opt = invA * b
    v_opt.x = invA[0][0] * b.x + invA[0][1] * b.y + invA[0][2] * b.z;
    v_opt.y = invA[1][0] * b.x + invA[1][1] * b.y + invA[1][2] * b.z;
    v_opt.z = invA[2][0] * b.x + invA[2][1] * b.y + invA[2][2] * b.z;

    return true;
}

void CQuadric::optimal_location_and_cost(CQuadricData Qeij_, vertex& v_opt, double& cost)
{
    //calculate v_opt and cost

    //// Johan's implementation
    //// Decompose the quadric matrix Qeij_ into A, b, and c components
    //double A[3][3];
    //double b[3];
    //double c = Qeij_.getMatrix()[3][3];

    //// Extract A (3x3 top-left submatrix of Qeij_)
    //for (int i = 0; i < 3; i++) {
    //    for (int j = 0; j < 3; j++) {
    //        A[i][j] = Qeij_.getMatrix()[i][j];
    //    }
    //}

    //// Extract b (negation of the first three elements of the fourth column of Qeij_)
    //for (int i = 0; i < 3; i++) {
    //    b[i] = -Qeij_.getMatrix()[i][3];
    //}


    //Eigen::Matrix3d eigenA;
    //Eigen::Vector3d eigenB;
    //// Copying data from A and b into Eigen types for further calculations
    //for (int i = 0; i < 3; ++i) {
    //    eigenB(i) = b[i];
    //    for (int j = 0; j < 3; ++j) {
    //        eigenA(i, j) = A[i][j];
    //    }
    //}

    //// Solve for the optimal vertex position using Eigen's QR decomposition
    //Eigen::Vector3d v_opt_eigen = eigenA.colPivHouseholderQr().solve(eigenB);

    //// Calculate the quadric error at the optimal vertex location
    //cost = (v_opt_eigen.transpose() * eigenA * v_opt_eigen)(0, 0) - 2 * eigenB.transpose() * v_opt_eigen + c;

    //// Update the vertex structure with the optimal location
    //v_opt.x = v_opt_eigen(0);
    //v_opt.y = v_opt_eigen(1);
    //v_opt.z = v_opt_eigen(2);

    // Tina's implementation
    // Extract A (3x3), b (3x1), and c from the 4x4 quadric matrix Qeij_
    double A[3][3] = {
        { Qeij_.get(0, 0), Qeij_.get(0, 1), Qeij_.get(0, 2) },
        { Qeij_.get(1, 0), Qeij_.get(1, 1), Qeij_.get(1, 2) },
        { Qeij_.get(2, 0), Qeij_.get(2, 1), Qeij_.get(2, 2) }
    };

    vertex b(-Qeij_.get(0, 3), -Qeij_.get(1, 3), -Qeij_.get(2, 3));

    // Attempt to solve for v_opt (A * v_opt = b)
    vertex v_opt_temp;
    bool success = solve_linear_system(A, b, v_opt_temp);

    if (success) {
        v_opt = v_opt_temp;

        // Compute cost: v_opt^T * A * v_opt + 2 * b^T * v_opt + c
        double c = Qeij_.get(3, 3);
        cost = (v_opt.dot(A[0]) * v_opt.x + v_opt.dot(A[1]) * v_opt.y + v_opt.dot(A[2]) * v_opt.z) - 2.0 * b.dot(v_opt) + c;
    }
    else {
        // Handle degenerate case (e.g., A is singular)
        v_opt = vertex(0.0, 0.0, 0.0);  // Default value in case of failure
        cost = std::numeric_limits<float>::max();  // Max cost to indicate failure
    }
}