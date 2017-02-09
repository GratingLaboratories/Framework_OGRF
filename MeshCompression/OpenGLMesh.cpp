#include "stdafx.h"
#include "OpenGLMesh.h"

OpenGLMesh::OpenGLMesh(const QString &name)
{
    mesh_.request_vertex_normals();

    OpenMesh::IO::Options opt;
    if (!OpenMesh::IO::read_mesh(mesh_, name.toStdString(), opt))
    {
        std::cerr << "Error loading mesh_ from file " << name.toStdString() << std::endl;
    }

    // If the file did not provide vertex normals, then calculate them
    if (!opt.check(OpenMesh::IO::Options::VertexNormal))
    {
        // we need face normals to update the vertex normals
        mesh_.request_face_normals();

        // let the mesh_ update the normals
        mesh_.update_normals();

        ////// dispose the face normals, as we don't need them anymore
        ////mesh_.release_face_normals();
    }

    update();
}

OpenGLMesh::~OpenGLMesh()
{
}

inline void _push_vec(std::vector<GLfloat> &v, OpenMesh::Vec3f data)
{
    v.push_back(data[0]);
    v.push_back(data[1]);
    v.push_back(data[2]);
}

void OpenGLMesh::update()
{
    vbuffer.clear();
    for (auto v_it : mesh_.vertices())
    {
        _push_vec(vbuffer, mesh_.point(v_it));
        _push_vec(vbuffer, { 1.0f, 1.0f, 1.0f });
        _push_vec(vbuffer, mesh_.normal(v_it));
    }
    assert(vbuffer.size() == mesh_.n_vertices() * TOTAL_ATTRIBUTE_SIZE);

    ebuffer.clear();
    for (auto f_it : mesh_.faces())
    {
        auto fv_it = mesh_.fv_iter(f_it);
        for (; fv_it; ++fv_it)
            ebuffer.push_back(fv_it->idx());
    }
    assert(ebuffer.size() == mesh_.n_faces() * VERTICES_PER_FACE);
}