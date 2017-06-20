#include "stdafx.h"
#include "OpenGLMesh.h"
#include "OpenGLScene.h"

using OpenMesh::Vec3f;

const QVector3D OpenGLMesh::DEFAULT_COLOR{42.f, 42.f, 42.f};

#define _split3(v) (v)[0], (v)[1], (v)[2]

bool _FileExists(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}

// (x, y, z) -> (-x, z, y)
Vec3f trans_coord(Vec3f &p)
{
    return{ -p[0], p[2], p[1] };
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
    QString tetra_name = file_location_ + "tetra/" + file_name_;
    if (!_FileExists(tetra_name + TETRA_ELE_EXTENSION) && NEED_TETRA)
    {
        TriMesh temp_mesh = this->mesh_;
        mesh_unify(1.0, true, temp_mesh); // unify to 1.0 before tetra().
        TetrahedralizationSolution ts{ temp_mesh, (tetra_name).toStdString() };
        ts.tetra();
    }

    if (mesh_file_name.contains("coodtr"))
    {
        for (auto vh : mesh_.vertices())
            mesh_.point(vh) = trans_coord(mesh_.point(vh));
    }

    if (need_scale_)
        mesh_unify(scale_, need_centralize_);
    else
        scale_ = get_sacle();
    
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

    if (NEED_TETRA)
        ReadTetra(tetra_name);

    update();
}

void OpenGLMesh::tag_change()
{
    changed_ = true;
}

Vec3f qvec2vec3f(const QVector3D &qc)
{
    return{ qc[0], qc[1], qc[2] };
}
QVector3D qvec2vec3f(const Vec3f &v)
{
    return{ v[0], v[1], v[2] };
}

void OpenGLMesh::set_point(int idx, QVector3D p)
{
    auto v_handle = mesh_.vertex_handle(idx);
    mesh_.set_point(v_handle, qvec2vec3f(p - position_));
}

void OpenGLMesh::slice(const LayerConfig& slice_config)
{
    this->slice_config_ = slice_config;
    update();
    tag_change();
}

OpenGLMesh::OpenGLMesh(const OpenGLMesh& rhs)
{
    vbuffer = rhs.vbuffer;
    voffset = rhs.voffset;
    ebuffer = rhs.ebuffer;

    name_ = rhs.name_ + QString("_clone");
    file_location_ = rhs.file_location_;
    file_name_ = rhs.file_name_ + QString("_clone");
    mesh_extension_ = rhs.mesh_extension_;
    need_scale_ = rhs.need_scale_;
    need_centralize_ = rhs.need_centralize_;
    use_face_normal_ = rhs.use_face_normal_;
    show_tetra_ = rhs.show_tetra_;
    scale_ = rhs.scale_;
    center_ = rhs.center_;
    max_point = rhs.max_point;
    min_point = rhs.min_point;
    scale_x = rhs.scale_x;
    scale_y = rhs.scale_y;
    scale_z = rhs.scale_z;
    position_ = rhs.position_;
    color_ = rhs.color_;
    changed_ = rhs.changed_;
    mesh_ = rhs.mesh_;
    tetra_ = rhs.tetra_;
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

void OpenGLMesh::update()
{
    vbuffer.clear();
    ebuffer.clear();
    //mesh_.update_normals();
    int i = 0;
    if (show_tetra_)
    {
        for (int v_i = 0; v_i < tetra_.n_vertices; ++v_i)
        {
            //if (slice_no_in_show_area(_split3(tetra_.point[v_i])))
            //    continue;

            _push_vec(vbuffer, tetra_.point[v_i]);

            if (v_i < tetra_.n_vertices_boundary)
            {
                if (this->color_ == DEFAULT_COLOR)
                    _push_vec(vbuffer, Vec3f{
                    sinf((i + 0) * 3.14f / 30) * 0.2f + 0.8f,
                    sinf((i + 0) * 3.14f / 60) * 0.2f + 0.8f,
                    sinf((i + 0) * 3.14f / 120) * 0.2f + 0.8f
                });
                else
                    _push_vec(vbuffer, this->color_);
            }
            else
            {
                _push_vec(vbuffer, 
                    cosf(tetra_.point[v_i][0] / scale_x * PI) * 0.5f + 0.5f,
                    sinf(tetra_.point[v_i][1] / scale_y * PI) * 0.5f + 0.5f,
                    sinf(tetra_.point[v_i][2] / scale_z * PI) * 0.5f + 0.5f
                );                                
            }

            if (v_i < tetra_.n_vertices_boundary)
                _push_vec(vbuffer, mesh_.normal(mesh_.vertex_handle(v_i))); // vertex normal
            else
                _push_vec(vbuffer, 1.0f, 1.0f, 1.0f);
            i++;
        }
//        assert(vbuffer.size() == tetra_.n_vertices * TOTAL_ATTRIBUTE_SIZE);

        for (auto f_it : mesh_.faces())
        {
            auto fv_it = mesh_.fv_iter(f_it);
            for (; fv_it; ++fv_it)
                ebuffer.push_back(fv_it->idx());
        }

        for (int t_i = 0; t_i < tetra_.n_tetras; ++t_i)
        {
            int x, y, z, w;
            x = tetra_.tetra_vertices[t_i][0];
            y = tetra_.tetra_vertices[t_i][1];
            z = tetra_.tetra_vertices[t_i][2];
            w = tetra_.tetra_vertices[t_i][3];

            ebuffer.push_back(x);
            ebuffer.push_back(y);
            ebuffer.push_back(z);

            ebuffer.push_back(x);
            ebuffer.push_back(z);
            ebuffer.push_back(w);

            ebuffer.push_back(x);
            ebuffer.push_back(w);
            ebuffer.push_back(y);

            ebuffer.push_back(z);
            ebuffer.push_back(y);
            ebuffer.push_back(w);
        }
    }
    else if (use_face_normal_)
    {
        int vid = 0;
        for (auto f_it : mesh_.faces())
        {
            auto fv_it = mesh_.fv_iter(f_it);
            bool show = true;
            for (; fv_it; ++fv_it)
            {
                if (slice_no_in_show_area(_split3(mesh_.point(fv_it))))
                {
                    show = false;
                    break;
                }
            }
            if (!show)
                continue;
            fv_it = mesh_.fv_iter(f_it);
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

        for (auto f_it : mesh_.faces())
        {
            auto fv_it = mesh_.fv_iter(f_it);
            bool show = true;
            for (; fv_it; ++fv_it)
            {
                if (slice_no_in_show_area(_split3(mesh_.point(fv_it))))
                {
                    show = false;
                    break;
                }
            }
            fv_it = mesh_.fv_iter(f_it);
            if (show)
                for (; fv_it; ++fv_it)
                {
                    ebuffer.push_back(fv_it->idx());
                }
        }
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

// this is to calculate the scale factor of the current model
// when "NeedScale" is false, in which case the mesh_unify will
// not be called. But we need a scale factor according to the
// original mesh for further calculation, e.g., rebuild the
// tetra info from the .ele file, which is based on a identity-
// unified mesh.
float OpenGLMesh::get_sacle()
{
    assert(this->need_scale_ == false); // when NeedScale is true, this should not be called.
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
    max_point = { xmax, ymax, zmax };
    min_point = { xmin, ymin, zmin };

    // here we use height(z) as scale target.
    float scaleX = xmax - xmin;
    float scaleY = ymax - ymin;
    float scaleZ = zmax - zmin;
    float scaleMax = scaleZ;
    scale_x = scaleX;
    scale_y = scaleY;
    scale_z = scaleZ;

    return scaleMax;
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
    max_point = { xmax, ymax, zmax };
    min_point = { xmin, ymin, zmin };

    // here we use height(z) as scale target.
    float scaleX = xmax - xmin;
    float scaleY = ymax - ymin;
    float scaleZ = zmax - zmin;
    float scaleMax = scaleZ;

    //scaleMax = std::max(scaleX, scaleY);
    //scaleMax = std::max(scaleMax, scaleZ);

    float scaleV = scale / scaleMax;

    scale_x = scaleX * scaleV;
    scale_y = scaleY * scaleV;
    scale_z = scaleZ * scaleV;

    Vec3f center((xmin + xmax) / 2.f, (ymin + ymax) / 2.f, (zmin + zmax) / 2.f);
    center_ = qvec2vec3f(center);
    if (centralize == false)
        center_ = { 0,0,0 };
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
    max_point = (max_point - qvec2vec3f(center)) * scaleV;
    min_point = (min_point - qvec2vec3f(center)) * scaleV;
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

#define DELTA 1.0e-6

bool OpenGLMesh::slice_no_in_show_area(float x, float y, float z)
{
    //if (slice_config_.revX)
    //{
    //    if (x > min_point.x() + slice_config_.sliceX * scale_x + DELTA)
    //        return true;
    //}
    //else
    //{
    //    if (x < min_point.x() + slice_config_.sliceX * scale_x - DELTA)
    //        return true;
    //}
    //if (slice_config_.revY)
    //{
    //    if (y > min_point.y() + slice_config_.sliceY * scale_y + DELTA)
    //        return true;
    //}
    //else
    //{
    //    if (y < min_point.y() + slice_config_.sliceY * scale_y - DELTA)
    //        return true;
    //}
    //if (slice_config_.revZ)
    //{
    //    if (z > min_point.z() + slice_config_.sliceZ * scale_z + DELTA)
    //        return true;
    //}
    //else
    //{
    //    if (z < min_point.z() + slice_config_.sliceZ * scale_z - DELTA)
    //        return true;
    //}

    return false;
}

void OpenGLMesh::ReadTetra(const QString& name)
{
    using namespace std;

    tetra_.n_vertices_boundary = this->mesh_.n_vertices();

    auto name_node = name + ".node";
    auto name_face = name + ".face";
    auto name_ele  = name + ".ele";

    QFile input_node(name_node);
    if (input_node.open(QIODevice::ReadOnly))
    {
        QTextStream in(&input_node);
        in >> tetra_.n_vertices;
        in.readLine();

        for (int i = 0; i < tetra_.n_vertices; ++i)
        {
            int index;
            float x, y, z;
            in >> index >> x >> y >> z;
            in.readLine();

            // Note that tetra info in files are based on
            // identity-unified mesh.
            tetra_.point.push_back(qvec2vec3f(position_) + Vec3f{
                x * scale_,
                y * scale_,
                z * scale_
            });
        }

        input_node.close();
    }

    QFile input_face(name_face);
    if (input_face.open(QIODevice::ReadOnly))
    {
        QTextStream in(&input_face);
        in >> tetra_.n_faces;
        in.readLine();

        for (int i = 0; i < tetra_.n_faces; ++i)
        {
            int index;
            int x, y, z;
            in >> index >> x >> y >> z;
            in.readLine();
            tetra_.face_vertices.push_back({ x, y, z });
        }

        input_face.close();
    }

    QFile input_ele(name_ele);
    if (input_ele.open(QIODevice::ReadOnly))
    {
        QTextStream in(&input_ele);
        in >> tetra_.n_tetras;
        in.readLine();

        for (int i = 0; i < tetra_.n_tetras; ++i)
        {
            int index;
            int x, y, z, w;
            in >> index >> x >> y >> z >> w;
            tetra_.tetra_vertices.push_back({ x, y, z, w });
        }

        input_ele.close();
    }

}
