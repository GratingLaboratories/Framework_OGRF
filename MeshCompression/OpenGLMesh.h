#pragma once
#include "OpenMeshBasic.h"
#include "TetrahedralizationSolution.h"
#include <QString>
#include <memory>
#include <vector>
#include <array>

#define ATTRIBUTE_POSITION_LOCATION 0
#define ATTRIBUTE_POSITION_SIZE     3
#define ATTRIBUTE_COLOR_LOCATION    1
#define ATTRIBUTE_COLOR_SIZE        3
#define ATTRIBUTE_NORMAL_LOCATION   2
#define ATTRIBUTE_NORMAL_SIZE       3
#define TOTAL_ATTRIBUTE_SIZE        (ATTRIBUTE_POSITION_SIZE + ATTRIBUTE_COLOR_SIZE + ATTRIBUTE_NORMAL_SIZE)
#define VERTICES_PER_FACE           3

#define INF                 9.9e9f
#define PI                  3.1415926f

using OpenMesh::VertexHandle;

struct TetraMesh
{
public:
    int n_vertices;
    int n_vertices_boundary;
    int n_faces;
    int n_tetras;
    std::vector<OpenMesh::Vec3f> point; // boundary vertices before internal ones.
    std::vector<std::array<int, 3>> face_vertices;
    std::vector<std::array<int, 4>> tetra_vertices;

    QVector3D point_qv(int i) 
    { 
        auto p = point[i];
        return{ p[0], p[1], p[2] };
    }

    TetraMesh copy() const
    {
        TetraMesh t;
        t.n_vertices = n_vertices;
        t.n_vertices_boundary = n_vertices_boundary;
        t.n_faces = n_faces;
        t.n_tetras = n_tetras;
        t.point = point;
        t.face_vertices = face_vertices;
        t.tetra_vertices = tetra_vertices;
        return t;
    }
};

class OpenGLMesh
{
public:
    OpenGLMesh() = default;
    ~OpenGLMesh();
    void update();
    void init();
    void tag_change();
    void set_point(int idx, QVector3D p);
    TriMesh &mesh() { return mesh_; }
    TetraMesh &tmesh() { return tetra_; }
    bool changed(); 

    std::vector<GLfloat> vbuffer;
    GLuint voffset;
    std::vector<GLuint>  ebuffer;

    QString name_;
    QString file_location_;
    QString file_name_;
    QString mesh_extension_;
    bool need_scale_;
    bool need_centralize_;
    bool use_face_normal_;
    bool show_tetra_;
    float scale_;
    QVector3D center_;
    float scale_x;
    float scale_y;
    float scale_z;
    QVector3D position_;
    QVector3D color_;

    static const QVector3D DEFAULT_COLOR;

private:
    bool changed_;
    TriMesh mesh_;

    float get_sacle();
    void mesh_unify(float scale = 1.0, bool centrailze = false);
    void mesh_unify(float scale, bool centrailze, TriMesh &mesh) const;

    void ReadTetra(const QString &name);
    TetraMesh tetra_;
};

