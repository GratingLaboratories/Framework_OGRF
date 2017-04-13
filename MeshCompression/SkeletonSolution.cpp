#include "stdafx.h"
#include "SkeletonSolution.h"

#define VERBOSE true
#define MOD(x, m) (((x) + (m)) % (m))
#define SPLIT(v) (v)[0], (v)[1], (v)[2]
#define lout() {if (VERBOSE) printf("\n");}
#define dout(d) {if (VERBOSE) printf(#d " = %f\n", (d));}
#define vout(v) {if (VERBOSE) printf(#v " = (%f, %f, %f)\n", SPLIT(v));}
#define mout(x) {if (VERBOSE&&x.rows()<50)std::cout<<#x<<"\n"<<x<<"\n\n";}

#define W_P 20.0f

SkeletonSolution::SkeletonSolution(TriMesh &mesh, ConsoleMessageManager &msg) :
    msg_(msg), n_vertices(0), n_edges(0), n_faces(0), mesh_(mesh)
{
}

SkeletonSolution::~SkeletonSolution() {  }

template <typename T1, typename T2>
T2 vec_cast(const T1 &v)
{
    return{ v[0], v[1], v[2] };
}

float SkeletonSolution::_cotangent_for_angle_AOB(int A, int O, int B)
{
    auto a = positions[A];
    auto o = positions[O];
    auto b = positions[B];

    auto oa = static_cast<Vector3f>(a - o);
    auto ob = static_cast<Vector3f>(b - o);
    vout(oa);
    vout(ob);
    
    auto cot = oa.dot(ob) / oa.cross(ob).norm();
    dout(cot);
    lout();

    return cot;
}

void SkeletonSolution::skeletonize()
{
    msg_.log("start skeletonize.");

    if (mesh_.vertices_empty())
    {
        msg_.log("mesh empty.");
        return;
    }

    n_vertices = mesh_.n_vertices();
    n_edges = mesh_.n_edges();
    n_faces = mesh_.n_faces();

    if (n_vertices + n_faces - n_edges != 2)
    {
        msg_.log("V + F - E != 2.");
        return;
    }

    positions.clear();
    for (auto vh : mesh_.vertices())
    {
        positions.push_back(vec_cast<OpenMesh::Vec3f, Eigen::Vector3f>(mesh_.point(vh)));
    }

    adjacency = vector<vector<bool>>(n_vertices, vector<bool>(n_vertices, false));
    degrees = vector<int>(n_vertices, 0);
    neighbors = vector<vector<int>>(n_vertices);
    for (auto vh : mesh_.vertices())
    {
        auto index_main = vh.idx();
        // for all the neighbors of the vertex:
        for (auto vvi = mesh_.vv_iter(vh); vvi.is_valid(); ++vvi)
        {
            auto vi = *vvi;
            auto index_sub = vi.idx();
            adjacency[index_main][index_sub] = true;
            degrees[index_main] += 1;
            neighbors[index_main].push_back(index_sub);
        }
    }

    SpMat spmat_Solve{ 2 * n_vertices, n_vertices };
    vector<T> tv_Lap;
    for (int vi = 0; vi < n_vertices; ++vi)
    {
        float weight_sum = 0.0f;
        int degree = degrees[vi];
        for (int j = 0; j < degree; ++j)
        {
            int vj      = neighbors[vi][j];
            int vleft   = neighbors[vi][MOD(j + 1, degree)];
            int vright  = neighbors[vi][MOD(j - 1, degree)];
            auto weight = _cotangent_for_angle_AOB(vi, vleft, vj) + _cotangent_for_angle_AOB(vi, vright, vj);
            weight_sum += weight;
            tv_Lap.push_back(T{ vi, vj, weight });
        }
        tv_Lap.push_back(T{ vi, vi, -weight_sum });
        tv_Lap.push_back(T{ vi + n_vertices, vi, W_P });
    }
    spmat_Solve.setFromTriplets(tv_Lap.begin(), tv_Lap.end());
    spmat_Solve.makeCompressed();

    Mat mat_B = Mat::Zero(2 * n_vertices, 3);
    for (int vi = 0; vi < n_vertices; ++vi)
    {
        for (int dim = 0; dim < 3; ++dim)
            mat_B(vi + n_vertices, dim) += W_P * positions[vi][dim];
    }
    mout(spmat_Solve);
    mout(mat_B);

    Eigen::SparseLU<SpMat, Eigen::COLAMDOrdering<int> > solver;
    // Solver
    // compute the ordering permutation vector from the structural pattern
    solver.analyzePattern(spmat_Solve);
    // compute the numerical factorization
    solver.factorize(spmat_Solve);
    if (solver.info() != Eigen::Success)
    {
        std::cout << "FAIL@ solver.prepare\n\n";
        return;
    }
    Mat mat_X = solver.solve(mat_B);
    if (solver.info() != Eigen::Success)
    {
        std::cout << "FAIL@ solver.solve(b);\n\n";
        return;
    }
    mout(mat_X);


    msg_.log("complete skeletonize.");
}
