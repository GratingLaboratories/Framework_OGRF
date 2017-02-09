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
    OpenGLMesh(const QString &name);
    ~OpenGLMesh();
    void update();
    const TriMesh &mesh() const { return mesh_; }
    bool changed(); 

    //private:
    bool changed_;
    TriMesh mesh_;
    std::vector<GLfloat> vbuffer;
    std::vector<GLuint>  ebuffer;
    
private:
    void mesh_unify(float scale = 1.0);
};

