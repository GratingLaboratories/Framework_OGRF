#include "stdafx.h"
#include "OffsetSolution.h"

#define SPLIT(v) (v)[0], (v)[1], (v)[2]
#define VERBOSE verbose_
#define MOD(x, m) (((x) + (m)) % (m))
#define lout() {if (VERBOSE) printf("\n");}
#define dout(d) {if (VERBOSE) printf(#d " = %f\n", (d));}
#define vout(v) {if (VERBOSE) printf(#v " = (%f, %f, %f)\n", SPLIT(v));}
#define mout(x) {if (VERBOSE&&x.rows()<50)std::cout<<#x<<"\n"<<x<<"\n\n";}

template <typename T1, typename T2>
T2 vec_cast(const T1 &v)
{
    return{ v[0], v[1], v[2] };
}

OffsetSolution::~OffsetSolution()
{
}

void OffsetSolution::offset()
{
    verbose_ = tcl_.get_bool("Verbose");

    msg_.log("start offset.");

    auto &mesh = scene_.get("Main")->mesh();

    if (mesh.vertices_empty())
    {
        msg_.log("mesh empty.");
        return;
    }

    n_vertices = mesh.n_vertices();
    n_edges = mesh.n_edges();
    n_faces = mesh.n_faces();
    //if (n_vertices + n_faces - n_edges != 2)
    //{
    //    msg_.log("V + F - E != 2.");
    //    return;
    //}

    vector<T> tv_Lap;
    Basic_Prepare_and_Calculate_Laplacian(tv_Lap);

    // MATLAB engine
    Engine *ep;
    if ((ep = engOpen("")) == nullptr)
    {
        msg_.log("MATLAB engine fail to start.");
        return;
    }

    Input_Variables_to_Engine(ep);

    // After this sub-routine, sparse matrix 'Lap' added to workspace.
    Input_Laplacian_to_Engine(tv_Lap, ep);

    // After this sub-routine, n*3 matrix 'X_0' added to workspace.
    Input_Positions_to_Engine(ep);

    // After this sub-routine, n*3 matrix 'V' added to workspace.
    Input_OffsetVector_to_Engine(ep);

    // run MATLAB script
    QFile script_file{ tcl_.get_string("Script_Filename") };
    script_file.open(QFile::ReadOnly | QFile::Text);
    QTextStream tsv{ &script_file };
    QString script = tsv.readAll();
    engEvalString(ep, script.toStdString().c_str());

    // Write Back
    //mxArray *ResX = engGetVariable(ep, "X");
    //for (int i = 0; i < n_vertices; i++)
    //{
    //    Vector3f pos{ 0, 0, 0 };
    //    pos[0] = mxGetPr(ResX)[i];
    //    pos[1] = mxGetPr(ResX)[i + n_vertices];
    //    pos[2] = mxGetPr(ResX)[i + n_vertices * 2];
    //    mesh_.point(mesh_.vertex_handle(i)) = { SPLIT(pos) };
    //}

    msg_.log("complete offset.");
}

// return a vector of Triples contains rows, cols, vals for the Laplacian.
// use a cotangent-weight Laplacian
// Complish the prepare: position and geometry topology extraction, offset-vector.
void OffsetSolution::Basic_Prepare_and_Calculate_Laplacian(vector<T> &tv_Lap)
{
    auto &mesh_ = scene_.get("Main")->mesh();

    positions.clear();
    for (auto vh : mesh_.vertices())
    {
        positions.push_back(vec_cast<OpenMesh::Vec3f, Eigen::Vector3f>(mesh_.point(vh)));
    }

    adjacency = vector<vector<bool>>(n_vertices, vector<bool>(n_vertices, false));
    degrees = vector<int>(n_vertices, 0);
    neighbors = vector<vector<int>>(n_vertices);
    offset_vector = vector<Vector3f>(n_vertices, {0, 0, 0});
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

    Calculate_OffsetVector();

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
            /*if (weight < 0.0f)
            weight = 0.0f;*/
            weight_sum += weight;
            tv_Lap.push_back(T{ vi, vj, weight });
        }
        tv_Lap.push_back(T{ vi, vi, -weight_sum });
    }
}

void OffsetSolution::Calculate_OffsetVector()
{
    //auto &mesh = scene_.get("Main")->mesh();
    auto &mesh = scene_.get("Main")->mesh();
    auto &skel = scene_.get("Skeleton")->mesh();

    for (int vi = 0; vi < mesh.n_vertices(); ++vi)
    {
        auto mesh_pos = vec_cast<OpenMesh::Vec3f, Eigen::Vector3f>(mesh.point(mesh.vertex_handle(vi)));
        auto skel_pos = vec_cast<OpenMesh::Vec3f, Eigen::Vector3f>(skel.point(skel.vertex_handle(vi)));

        Eigen::Vector3f delta = skel_pos - mesh_pos;
        delta.normalize();

        offset_vector[vi] = delta;
    }
}

void OffsetSolution::Input_Variables_to_Engine(Engine* ep)
{
    // scalar
    mxArray *scalar_Weight_Preserve = mxCreateDoubleMatrix(1, 1, mxREAL);
    mxGetPr(scalar_Weight_Preserve)[0] = tcl_.get_value("Weight_Preserve");
    engPutVariable(ep, "Weight_Preserve", scalar_Weight_Preserve);
    mxArray *scalar_Iteration_Limit = mxCreateDoubleMatrix(1, 1, mxREAL);
    mxGetPr(scalar_Iteration_Limit)[0] = tcl_.get_value("Iteration_Limit");
    engPutVariable(ep, "Iteration_Limit", scalar_Iteration_Limit);
}

void OffsetSolution::Input_Laplacian_to_Engine(const vector<T> &tv_Lap, Engine* ep)
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

void OffsetSolution::Input_Positions_to_Engine(Engine* ep)
{
    // matrix: X_0
    // size:   n * 3
    mxArray *X_0 = mxCreateDoubleMatrix(n_vertices, 3, mxREAL);

    for (int i = 0; i < n_vertices; i++)
    {
        mxGetPr(X_0)[i] = positions[i][0];
        mxGetPr(X_0)[i + n_vertices] = positions[i][1];
        mxGetPr(X_0)[i + n_vertices * 2] = positions[i][2];

    }
    engPutVariable(ep, "X_0", X_0);
}

void OffsetSolution::Input_OffsetVector_to_Engine(Engine* ep)
{
    // matrix: V
    // size:   n * 3
    mxArray *V = mxCreateDoubleMatrix(n_vertices, 3, mxREAL);

    for (int i = 0; i < n_vertices; i++)
    {
        mxGetPr(V)[i] = offset_vector[i][0];
        mxGetPr(V)[i + n_vertices] = offset_vector[i][1];
        mxGetPr(V)[i + n_vertices * 2] = offset_vector[i][2];
    }
    engPutVariable(ep, "V", V);
}

// return con(angle AOB)
float OffsetSolution::_cotangent_for_angle_AOB(int A, int O, int B)
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