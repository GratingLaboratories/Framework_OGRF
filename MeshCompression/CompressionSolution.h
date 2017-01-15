#pragma once
// OpenMesh structures
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
// MATLAB Engine
#include <engine.h>
#include <QString>
#include <map>
#include <vector>

typedef OpenMesh::TriMesh_ArrayKernelT<>  TriMesh;

using OpenMesh::VertexHandle;

using Vec3f_om = OpenMesh::Vec3f;
/// PositionMap: map each vertex handler to target position
typedef std::map<VertexHandle, Vec3f_om> PositionMap;
/// DifferenceMap: map each vertex handler to difference
typedef std::map<VertexHandle, float> DifferenceMap;

typedef struct Triple
{
    int row, col;
    float value;
    Triple(int r, int c, float v = 0.0f)
        :row(r), col(c), value(v) {  }
} Triple;

class CompressionSolution
{
public:
    CompressionSolution(TriMesh &mesh);
    ~CompressionSolution();
    void            compress(int precision = 100);
    bool            ok() const { return ok_; }
    QString         msg() const { return msg_; }
    PositionMap     getCompressedPositions() const { return result_position_map_; }
    DifferenceMap   getCompressedDifferences() const { return result_difference_map_; }
    float           getMaxDifference() const { return result_max_difference; }
    float           getMinDifference() const { return result_min_difference; }

private:
    // helper functions
    bool MATLABsubroutine(const std::vector<Triple> &builder,
        PositionMap &position_map,
            DifferenceMap &diff_map);

    // local variables
    // solution state
    TriMesh        &mesh_;
    int             precision_;
    bool            ok_;
    QString         msg_;

    // mesh meta-data
    int             num_verts_;
    std::vector<VertexHandle>       vert_handles_;
    std::vector<Vec3f_om>           vert_positions_;
    std::vector<std::vector<bool>>  adjacency_;
    std::vector<int>                degrees_;
    std::map<VertexHandle, int>     vert_ids_;

    // result 
    PositionMap     result_position_map_;
    DifferenceMap   result_difference_map_;
    float           result_max_difference;
    float           result_min_difference;
};

