#include "stdafx.h"
#include "CompressionSolution.h"

using std::vector;
using std::map;

CompressionSolution::CompressionSolution(TriMesh& mesh) :
    mesh_(mesh),
    precision_(100),
    ok_(false),
    msg_{"Not initialized."}, 
    result_max_difference(0), 
    result_min_difference(0)
{
    if (mesh_.vertices_empty())
    {
        msg_ = QString{"Initialized fail: empty mesh."};
        return;
    }

    // extract vertex handles and positions.
    for (auto v : mesh_.vertices())
    {
        vert_handles_.push_back(v);
        vert_positions_.push_back(mesh_.point(v));
    }
    num_verts_ = vert_handles_.size();

    // index the vertices.
    int id = 0;
    for (auto v : mesh_.vertices())
        vert_ids_[v] = id++;

    // extract adjacency and degrees.
    adjacency_ = vector<vector<bool>>(num_verts_, vector<bool>(num_verts_, false));
    degrees_ = vector<int>(num_verts_, 0);
    for (auto v : mesh_.vertices())
    {
        int degree = 0;
        int vi = vert_ids_[v];
        auto he_first = mesh_.halfedge_handle(v);
        auto he = mesh_.halfedge_handle(v);
        do
        {
            auto v_neighbor = mesh_.to_vertex_handle(he);
            int vi_neighbor = vert_ids_[v_neighbor];
            adjacency_[vi][vi_neighbor] = true;
            degree += 1;
            he = mesh.next_halfedge_handle(mesh.opposite_halfedge_handle(he));
        }
        while (he != he_first);
        degrees_[vi] = degree;
    }

    msg_ = QString{"Initialized successfully."};
}

CompressionSolution::~CompressionSolution()
{
}

void CompressionSolution::compress(int precision)
{
    precision_ = precision;

    if (precision_ > num_verts_)
    {
        msg_ = QString("Precision more than number of vertices of the mesh (%0).")
            .arg(num_verts_);
        return;
    }

    // Build a Sparse Matrix Builder vector.
    // TODO: current change
    // make Laplacian symmetric.
    vector<Triple> mat_builder_Laplacian;
    for (int row = 0; row < num_verts_; row++)
    {
        /*  <original>
        mat_builder_Laplacian.push_back({ row, row, 1.0f });
        float value = -1.0f / degrees_[row];
        for (int col = 0; col < num_verts_; col++)
        {
            if (adjacency_[row][col])
            {
                mat_builder_Laplacian.push_back({ row, col, value });
            }
        }  
        */      
        mat_builder_Laplacian.push_back({ row, row, static_cast<float>(degrees_[row]) });
        float value = -1.0f;
        for (int col = 0; col < num_verts_; col++)
        {
            if (adjacency_[row][col])
            {
                mat_builder_Laplacian.push_back({ row, col, value });
            }
        }
    }

    // MATLAB engine subroutine:
    //      take the builder vector,
    //      return the compression result and difference.
    bool success = MATLABsubroutine(mat_builder_Laplacian, result_position_map_, result_difference_map_);
    if (!success)
        return;

    // calculate max and min difference.
    result_max_difference = -1.0f;
    result_min_difference = +9e9f;
    for (auto pair : result_difference_map_)
    {
        if (pair.second > result_max_difference)
            result_max_difference = pair.second;
        else if (pair.second < result_min_difference)
            result_min_difference = pair.second;
    }

    // mark success state.
    ok_ = true;
    msg_ = QString{ "Compression Success." };
}

// return: true if success;
bool CompressionSolution::MATLABsubroutine(
    const std::vector<Triple>& builder,     // builder of sparse Laplacian
    PositionMap& position_map,          // (return) compressed geometry  
    DifferenceMap& diff_map         // (return) diff map
    )
{
    // MATLAB engine
    Engine *ep;

    if ((ep = engOpen("")) == nullptr)
    {
        msg_ = QString{ "MATLAB engine fail to start." };
        return false;
    }

    // scalar: precision
    mxArray *scalar_precision = mxCreateDoubleMatrix(1, 1, mxREAL);
    mxGetPr(scalar_precision)[0] = precision_;
    engPutVariable(ep, "p", scalar_precision);

    // sparse matrix: Laplacian
    int builder_length = builder.size();
    mxArray *LapRow = mxCreateDoubleMatrix(builder_length, 1, mxREAL),
        *LapCol = mxCreateDoubleMatrix(builder_length, 1, mxREAL),
        *LapVal = mxCreateDoubleMatrix(builder_length, 1, mxREAL);

    for (int i = 0; i < builder_length; i++)
    {
        auto T = builder[i];
        mxGetPr(LapRow)[i] = T.row + 1;     // MATLAB index starts
        mxGetPr(LapCol)[i] = T.col + 1;     // from 1.
        mxGetPr(LapVal)[i] = T.value;
    }
    engPutVariable(ep, "LapRow", LapRow);
    engPutVariable(ep, "LapCol", LapCol);
    engPutVariable(ep, "LapVal", LapVal);

    // long matrix: Geometry
    mxArray *GeoX = mxCreateDoubleMatrix(num_verts_, 1, mxREAL),
        *GeoY = mxCreateDoubleMatrix(num_verts_, 1, mxREAL),
        *GeoZ = mxCreateDoubleMatrix(num_verts_, 1, mxREAL);

    for (int i = 0; i < num_verts_; i++)
    {
        mxGetPr(GeoX)[i] = vert_positions_[i][0];
        mxGetPr(GeoY)[i] = vert_positions_[i][1];
        mxGetPr(GeoZ)[i] = vert_positions_[i][2];
    }
    engPutVariable(ep, "X", GeoX);
    engPutVariable(ep, "Y", GeoY);
    engPutVariable(ep, "Z", GeoZ);

    // MATLAB script
    engEvalString(ep, "Lap = sparse(LapRow, LapCol, LapVal);");
    engEvalString(ep, "[V, D] = eigs(Lap, size(Lap, 1));");
    engEvalString(ep, "Coef = V' * [X, Y, Z];");
    engEvalString(ep, "Geo = V(:, 1:p) * Coef(1:p, :);");
    engEvalString(ep, "Diff = sum(([X Y Z] - Geo).^ 2, 2).^ (0.5)");
    engEvalString(ep, "X_ = Geo(:, 1);");
    engEvalString(ep, "Y_ = Geo(:, 2);");
    engEvalString(ep, "Z_ = Geo(:, 3);");

    mxArray *ResX = engGetVariable(ep, "X_");
    mxArray *ResY = engGetVariable(ep, "Y_");
    mxArray *ResZ = engGetVariable(ep, "Z_");
    mxArray *Diff = engGetVariable(ep, "Diff");

    if (ResX == nullptr || ResY == nullptr || ResZ == nullptr || Diff == nullptr)
    {
        msg_ = QString{ "errors occur in MATLAB subroutine." };
        return false;
    }

    for (int i = 0; i < num_verts_; i++)
    {
        Vec3f_om pos{ 0, 0, 0 };
        pos[0] = mxGetPr(ResX)[i];
        pos[1] = mxGetPr(ResY)[i];
        pos[2] = mxGetPr(ResZ)[i];
        position_map[vert_handles_[i]] = pos;
        diff_map[vert_handles_[i]] = mxGetPr(Diff)[i];
    }

    mxDestroyArray(ResX);
    mxDestroyArray(ResY);
    mxDestroyArray(ResZ);
    mxDestroyArray(scalar_precision);
    mxDestroyArray(LapRow);
    mxDestroyArray(LapCol);
    mxDestroyArray(LapVal);
    mxDestroyArray(GeoX);
    mxDestroyArray(GeoY);
    mxDestroyArray(GeoZ);
    mxDestroyArray(Diff);
    engClose(ep);

    return true;
}
