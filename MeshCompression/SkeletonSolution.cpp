#include "stdafx.h"
#include "SkeletonSolution.h"
#include <engine.h>

#define SPLIT(v) (v)[0], (v)[1], (v)[2]
#define VERBOSE false
#define MOD(x, m) (((x) + (m)) % (m))
#define lout() {if (VERBOSE) printf("\n");}
#define dout(d) {if (VERBOSE) printf(#d " = %f\n", (d));}
#define vout(v) {if (VERBOSE) printf(#v " = (%f, %f, %f)\n", SPLIT(v));}
#define mout(x) {if (VERBOSE&&x.rows()<50)std::cout<<#x<<"\n"<<x<<"\n\n";}

#define W_P 20.0f

SkeletonSolution::SkeletonSolution(TriMesh &mesh, ConsoleMessageManager &msg) :
    msg_(msg), n_vertices(0), n_edges(0), n_faces(0), mesh_(mesh),
    tcl_{ "./config/skeleton.config" }
{
}

SkeletonSolution::~SkeletonSolution() {  }

template <typename T1, typename T2>
T2 vec_cast(const T1 &v)
{
    return{ v[0], v[1], v[2] };
}

// return con(angle AOB)
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

// return a vector of Triples contains rows, cols, vals for the Laplacian.
// use a cotangent-weight Laplacian
void SkeletonSolution::Basic_Prepare_and_Calculate_Laplacian(vector<T> &tv_Lap)
{
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

    for (int vi = 0; vi < n_vertices; ++vi)
    {
        float weight_sum = 0.0f;
        int degree = degrees[vi];
        for (int j = 0; j < degree; ++j)
        {
            int vj = neighbors[vi][j];
            int vleft = neighbors[vi][MOD(j + 1, degree)];
            int vright = neighbors[vi][MOD(j - 1, degree)];
            auto weight = _cotangent_for_angle_AOB(vi, vleft, vj) + _cotangent_for_angle_AOB(vi, vright, vj);
            if (weight < 0.0f)
                weight = 0.0f;
            weight_sum += weight;
            tv_Lap.push_back(T{ vi, vj, weight });
        }
        tv_Lap.push_back(T{ vi, vi, -weight_sum });
    }
}

void SkeletonSolution::Input_Laplacian_to_Engine(const vector<T> &tv_Lap, Engine* ep)
{
    // sparse matrix: Laplacian
    int builder_length = tv_Lap.size();
    mxArray
        *LapRow = mxCreateDoubleMatrix(builder_length, 1, mxREAL),
        *LapCol = mxCreateDoubleMatrix(builder_length, 1, mxREAL),
        *LapVal = mxCreateDoubleMatrix(builder_length, 1, mxREAL);

    for (int i = 0; i < builder_length; i++)
    {
        auto t = tv_Lap[i];
        mxGetPr(LapRow)[i] = t.row() + 1;     // MATLAB index starts
        mxGetPr(LapCol)[i] = t.col() + 1;     // from 1.
        mxGetPr(LapVal)[i] = t.value();
    }
    engPutVariable(ep, "LapRow", LapRow);
    engPutVariable(ep, "LapCol", LapCol);
    engPutVariable(ep, "LapVal", LapVal);
    engEvalString(ep, "Lap = sparse(LapRow, LapCol, LapVal);");
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

    vector<T> tv_Lap;
    Basic_Prepare_and_Calculate_Laplacian(tv_Lap);

    // MATLAB engine
    Engine *ep;
    if ((ep = engOpen("")) == nullptr)
    {
        msg_.log("MATLAB engine fail to start.");
        return;
    }

    // After this sub-routine, sparse matrix 'Lap' added to workspace.
    Input_Laplacian_to_Engine(tv_Lap, ep);

    // 

    msg_.log("complete skeletonize.");
}
