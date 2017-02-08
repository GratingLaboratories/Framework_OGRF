#pragma once
// OpenMesh structures
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <QString>
#include <map>
#include <vector>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

typedef OpenMesh::TriMesh_ArrayKernelT<> TriMesh;

using OpenMesh::VertexHandle;

class OpenGLMesh
{
public:
    OpenGLMesh(const QString &name);
    ~OpenGLMesh();
    void update();
    void draw(const QOpenGLShaderProgram *shader);

private:
    TriMesh                     mesh_;
    QOpenGLBuffer              *vbo, *veo;
    QOpenGLVertexArrayObject   *vao;
    GLfloat                    *vbuffer;
    GLuint                     *ebuffer;
};

