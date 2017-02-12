#include "stdafx.h"
#include "OpenGLMesh.h"

using OpenMesh::Vec3f;

const QVector3D OpenGLMesh::DEFAULT_COLOR{42.f, 42.f, 42.f};

bool _FileExists(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}

void OpenGLMesh::init()
{
    mesh_.request_vertex_normals();

    OpenMesh::IO::Options opt;
    QString mesh_file_name = file_location_ + file_name_ + mesh_extension_;
    if (!OpenMesh::IO::read_mesh(mesh_, mesh_file_name.toStdString(), opt))
    {
        std::cerr << "Error loading mesh_ from file " << mesh_file_name.toStdString() << std::endl;
    }

    // try find tetrahedralization.
    if (!_FileExists(file_location_ + "tetra/" + file_name_ + TETRA_ELE_EXTENSION))
    {
        TriMesh temp_mesh = this->mesh_;
        mesh_unify(1.0, true, temp_mesh); // unify to 1.0 before tetra().
        TetrahedralizationSolution ts{ this->mesh_, (file_location_ + "tetra/" + file_name_).toStdString() };
        ts.tetra();
    }

    if (need_scale_)
        mesh_unify(scale_, need_centralize_);
    
    // If the file did not provide vertex normals, then calculate them
    if (!opt.check(OpenMesh::IO::Options::VertexNormal))
    {
        // we need face normals to update the vertex normals
        mesh_.request_face_normals();

        // let the mesh_ update the normals
        mesh_.update_normals();

        // maybe face normal has future usage.
        ////// dispose the face normals, as we don't need them anymore
        ////mesh_.release_face_normals();
    }

    update();
}

OpenGLMesh::~OpenGLMesh()
{
}

inline void _push_vec(std::vector<GLfloat> &v, const OpenMesh::Vec3f &data)
{
    v.push_back(data[0]);
    v.push_back(data[1]);
    v.push_back(data[2]);
}
inline void _push_vec(std::vector<GLfloat> &v, const QVector3D &data)
{
    v.push_back(data[0]);
    v.push_back(data[1]);
    v.push_back(data[2]);
}

inline void _push_vec(std::vector<GLfloat> &v, GLfloat a, GLfloat b, GLfloat c)
{
    v.push_back(a);
    v.push_back(b);
    v.push_back(c);
}

// unused.
// dir, right, up -> x, y, z in common coordinates sys
Vec3f trans_coord(Vec3f &p)
{
    return{ p[2], p[0], p[1] };
}

Vec3f qvec2vec3f(const QVector3D &qc)
{
    return{ qc[0], qc[1], qc[2] };
}

void OpenGLMesh::update()
{
    // test
    auto temp = mesh_;
    temp.clear();
    auto a = mesh_.n_vertices();
    auto b = temp.n_vertices();

    vbuffer.clear();
    ebuffer.clear();
    int i = 0;
    if (use_face_normal_)
    {
        int vid = 0;
        for (auto f_it : mesh_.faces())
        {
            auto fv_it = mesh_.fv_iter(f_it);
            for (; fv_it; ++fv_it)
            {
                _push_vec(vbuffer, mesh_.point(*fv_it) + qvec2vec3f(position_));
                if (this->color_ == DEFAULT_COLOR)
                    _push_vec(vbuffer, Vec3f{
                        sinf((i + 0) * 3.14f / 30) * 0.2f + 0.8f,
                        sinf((i + 0) * 3.14f / 60) * 0.2f + 0.8f,
                        sinf((i + 0) * 3.14f / 120) * 0.2f + 0.8f
                });
                else
                    _push_vec(vbuffer, this->color_);
                _push_vec(vbuffer, mesh_.normal(f_it)); // face normal
                i++;

                ebuffer.push_back(vid++);
            }
        }
        assert(ebuffer.size() == mesh_.n_faces() * VERTICES_PER_FACE);
    }
    else
    {
        for (auto v_it : mesh_.vertices())
        {
            _push_vec(vbuffer, mesh_.point(v_it) + qvec2vec3f(position_));
            if (this->color_ == DEFAULT_COLOR)
                _push_vec(vbuffer, Vec3f{
                    sinf((i + 0) * 3.14f / 30) * 0.2f + 0.8f,
                    sinf((i + 0) * 3.14f / 60) * 0.2f + 0.8f,
                    sinf((i + 0) * 3.14f / 120) * 0.2f + 0.8f
                });
            else
                _push_vec(vbuffer, this->color_);
            _push_vec(vbuffer, mesh_.normal(v_it)); // vertex normal
            i++;
        }
        assert(vbuffer.size() == mesh_.n_vertices() * TOTAL_ATTRIBUTE_SIZE);

        for (auto f_it : mesh_.faces())
        {
            auto fv_it = mesh_.fv_iter(f_it);
            for (; fv_it; ++fv_it)
                ebuffer.push_back(fv_it->idx());
        }
        assert(ebuffer.size() == mesh_.n_faces() * VERTICES_PER_FACE);
    }

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

void OpenGLMesh::mesh_unify(float scale, bool centralize)
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

    // here we use height(z) as scale target.
    float scaleX = xmax - xmin;
    float scaleY = ymax - ymin;
    float scaleZ = zmax - zmin;
    float scaleMax = scaleZ;

    //scaleMax = std::max(scaleX, scaleY);
    //scaleMax = std::max(scaleMax, scaleZ);

    float scaleV = scale / scaleMax;
    Vec3f center((xmin + xmax) / 2.f, (ymin + ymax) / 2.f, (zmin + zmax) / 2.f);
    for (auto v : mesh_.vertices())
    {
        Vec3f pt = mesh_.point(v);
        Vec3f res;
        if (centralize)
            res = (pt - center) * scaleV; // VS cannot detect some of the operation, fake error (in my computer)
        else
            res = pt * scaleV;
        OpenMesh::Vec3f res_om{ res[0], res[1], res[2] };
        mesh_.set_point(v, res_om);
    }
    // REMARK: OpenMesh::Vec3f has conflict with Vec3f;
}

void OpenGLMesh::mesh_unify(float scale, bool centrailze, TriMesh& mesh) const
{
    using OpenMesh::Vec3f;

    Vec3f max_pos(-INF, -INF, -INF);
    Vec3f min_pos(+INF, +INF, +INF);

    for (auto v : mesh.vertices())
    {
        auto point = mesh.point(v);
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

    // here we use height(z) as scale target.
    float scaleX = xmax - xmin;
    float scaleY = ymax - ymin;
    float scaleZ = zmax - zmin;
    float scaleMax = scaleZ;

    //scaleMax = std::max(scaleX, scaleY);
    //scaleMax = std::max(scaleMax, scaleZ);

    float scaleV = scale / scaleMax;
    Vec3f center((xmin + xmax) / 2.f, (ymin + ymax) / 2.f, (zmin + zmax) / 2.f);
    for (auto v : mesh.vertices())
    {
        Vec3f pt = mesh.point(v);
        Vec3f res;
        if (centrailze)
            res = (pt - center) * scaleV; // VS cannot detect some of the operation, fake error (in my computer)
        else
            res = pt * scaleV;
        OpenMesh::Vec3f res_om{ res[0], res[1], res[2] };
        mesh.set_point(v, res_om);
    }
    // REMARK: OpenMesh::Vec3f has conflict with Vec3f;
}
