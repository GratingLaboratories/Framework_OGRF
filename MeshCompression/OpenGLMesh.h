#pragma once
// OpenMesh structures
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <QString>
#include <memory>
#include <vector>

#define ATTRIBUTE_POSITION_LOCATION 0
#define ATTRIBUTE_POSITION_SIZE     3
#define ATTRIBUTE_COLOR_LOCATION    1
#define ATTRIBUTE_COLOR_SIZE        3
#define ATTRIBUTE_NORMAL_LOCATION   2
#define ATTRIBUTE_NORMAL_SIZE       3
#define TOTAL_ATTRIBUTE_SIZE        (ATTRIBUTE_POSITION_SIZE + ATTRIBUTE_COLOR_SIZE + ATTRIBUTE_NORMAL_SIZE)
#define VERTICES_PER_FACE           3

#define INF                 9.9e9f

typedef OpenMesh::TriMesh_ArrayKernelT<> TriMesh;

using OpenMesh::VertexHandle;

class OpenGLMesh
{
public:
    OpenGLMesh() = default;
    ~OpenGLMesh();
    void update();
    void init();
    const TriMesh &mesh() const { return mesh_; }
    bool changed(); 

    std::vector<GLfloat> vbuffer;
    std::vector<GLuint>  ebuffer;

    QString name_;
    QString file_location_;
    QString file_name_;
    QString mesh_extension_;
    bool need_scale_;
    bool need_centralize_;
    float scale_;
    QVector3D position_;

private:
    bool changed_;
    TriMesh mesh_;

    void mesh_unify(float scale = 1.0, bool centrailze = false);
};

