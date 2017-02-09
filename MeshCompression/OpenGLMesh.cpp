#include "stdafx.h"
#include "OpenGLMesh.h"

using OpenMesh::Vec3f;

OpenGLMesh::OpenGLMesh(const QString &name)
{
    mesh_.request_vertex_normals();

    OpenMesh::IO::Options opt;
    if (!OpenMesh::IO::read_mesh(mesh_, name.toStdString(), opt))
    {
        std::cerr << "Error loading mesh_ from file " << name.toStdString() << std::endl;
    }

    mesh_unify(1.0f);

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

// dir, right, up -> x, y, z in common coordinates sys
Vec3f trans_coord(Vec3f &p)
{
    return{ p[2], p[0], p[1] };
}

void OpenGLMesh::update()
{
    vbuffer.clear();
    for (auto v_it : mesh_.vertices())
    {
        _push_vec(vbuffer, trans_coord(mesh_.point(v_it)));
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

    changed_ = true;
}

// return whether buffer should get renew.
// whether or not, changed_ turn to false. 
bool OpenGLMesh::changed() 
{
    if (changed_)
    {
        changed_ = false;
        return true;
    }
    return false;
}

void OpenGLMesh::mesh_unify(float scale)
{
    using OpenMesh::Vec3f;

    Vec3f max_pos(-INF, -INF, -INF);
    Vec3f min_pos(+INF, +INF, +INF);

    for (auto v : mesh_.vertices())
    {
        auto point = mesh_.point(v);
        for (int i = 0; i < 3; i++)
        {
            float t = point[i];
            if (t > max_pos[i])
                max_pos[i] = t;
            if (t < min_pos[i])
                min_pos[i] = t;
        }
    }

    float xmax = max_pos[0], ymax = max_pos[1], zmax = max_pos[2];
    float xmin = min_pos[0], ymin = min_pos[1], zmin = min_pos[2];

    float scaleX = xmax - xmin;
    float scaleY = ymax - ymin;
    float scaleZ = zmax - zmin;
    float scaleMax;

    scaleMax = std::max(scaleX, scaleY);
    scaleMax = std::max(scaleMax, scaleZ);

    float scaleV = scale / scaleMax;
    Vec3f center((xmin + xmax) / 2.f, (ymin + ymax) / 2.f, (zmin + zmax) / 2.f);
    for (auto v : mesh_.vertices())
    {
        OpenMesh::Vec3f pt_om = mesh_.point(v);
        Vec3f pt{ pt_om[0], pt_om[1], pt_om[2] };
        Vec3f res = (pt - center) * scaleV;
        OpenMesh::Vec3f res_om{ res[0], res[1], res[2] };
        mesh_.set_point(v, res_om);
    }
    // REMARK: OpenMesh::Vec3f has conflict with Vec3f;
}
