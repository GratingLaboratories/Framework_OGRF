#include "stdafx.h"
#include "SimulatorBase.h"

#include <QString>

void SimulatorBase::init(const double& time)
{
    init_time_ = last_time_ = time;
}

void SimulatorBase::simulate(const double& time)
{
    curr_time_ = time;
    t = curr_time_ - init_time_;
    dt = curr_time_ - last_time_;
    simulate_util();
    simulate_rebuild();
    last_time_ = curr_time_;
}

void SimulatorBase::simulate_util()
{
    //auto ball = scene_.get("Ball");
    //for (auto v : ball->mesh().vertices())
    //{
    //    auto &mesh = ball->mesh();
    //    auto pos = mesh.point(v);
    //    ball->mesh().set_point(v, pos * 1.002);
    //}
    //ball->update();
}

void SimulatorBase::simulate_rebuild()
{
    //auto ball = scene_.get("Ball");
    //for (auto v : ball->mesh().vertices())
    //{
    //    auto &mesh = ball->mesh();
    //    auto pos = mesh.point(v);
    //    ball->mesh().set_point(v, pos * 1.002);
    //}
    //ball->update();
}

using OpenMesh::Vec3f;

float _tetra_volume(QVector3D a, QVector3D b, QVector3D c, QVector3D d)
{
    auto x = b - a;
    auto y = c - a;
    auto z = d - a;

    auto v = QVector3D::dotProduct(x, QVector3D::crossProduct(y, z));
    return 1.0f / 6 * abs(v);
}

float _tetra_volume(Vector3f a, Vector3f b, Vector3f c, Vector3f d)
{
    auto x = b - a;
    auto y = c - a;
    auto z = d - a;

    auto v = x.dot(y.cross(z));
    return 1.0f / 6 * abs(v);
}

Vector3f qv_to_ev(const QVector3D &v)
{
    return{ v[0], v[1], v[2] };
}

Vector3f ov_to_ev(const OpenMesh::Vec3f &v)
{
    return{ v[0], v[1], v[2] };
}

OpenMesh::Vec3f ev_to_ov(const Vector3f &v)
{
    return{ v[0], v[1], v[2] };
}

QVector3D ev_to_qv(const Vector3f &v)
{
    return{ v[0], v[1], v[2] };
}

void SimulatorSimpleFED::init(const double& t)
{
    SimulatorBase::init(t);
    ball = scene_.get("Ball");
    ground = scene_.get("Ground");
    if (ball == nullptr || ground == nullptr)
    {
        init_ok_ = false;
        return;
    }
    auto &tmesh = ball->tmesh();
    velocity = std::vector<Vector3f>(tmesh.n_vertices, { 0,0,0 });
    vert_volume = std::vector<float>(tmesh.n_vertices, 0.0f);
    tetra_volume = std::vector<float>(tmesh.n_tetras, 0.0f);
    //position = std::vector<Vector3f>(tmesh.n_vertices, { 0,0,0 });
    for (int i = 0; i < tmesh.n_vertices; ++i)
    {
        position.push_back(ov_to_ev(tmesh.point[i]));
    }
    for (int i = 0; i < tmesh.n_tetras; ++i)
    {
        auto tvs = tmesh.tetra_vertices[i];
        auto a = position[tvs[0]];
        auto b = position[tvs[1]];
        auto c = position[tvs[2]];
        auto d = position[tvs[3]];
        tetra_volume[i] = _tetra_volume(a, b, c, d);
        for (int j = 0; j < 4; ++j)
            vert_volume[tvs[j]] += tetra_volume[i] * 0.25f;
        Vector3f x_1 = (b - a);
        Vector3f x_2 = (c - a);
        Vector3f x_3 = (d - a);
        Matrix3f X_i;
        X_i << x_1, x_2, x_3;
        X_bar.push_back(X_i.inverse());
    }

    init_ok_ = true;
}

void SimulatorSimpleFED::simulate_util()
{
    const float density = 1000.0f; // \ro: 1000 kg/m^3, water like.
    const Vector3f g{ 0.0f, 0.0f, -9.8f }; // g: jyokure kassodoku.
    const float E = 0.033e9f; // E: Young’s modulus (G Pascal)
    // 0.001-0.1 ~ rubber
    // 10        ~ wood
    // 100       ~ medal
    // 1000      ~ diamond
    auto &tmesh = ball->tmesh();
    std::vector<Vector3f> force(tmesh.n_vertices, {0,0,0});
    std::vector<float> masses(tmesh.n_vertices);
    // for all vertices:
    for (int vi = 0; vi < tmesh.n_vertices; ++vi)
    {
        // STEP 1:  Apply external forces.
        float m = density * vert_volume[vi];   // mass of the vertex, derived from volume.
        // STEP 1.1 Gravity.
        Vector3f f_g = m * g;

        // STEP 1.2 Collision force.
        // Not here, 
        // we do a force balance after elastic force has been calculated.

        force[vi] += f_g;
        masses[vi] = m;
    }
    // for all tetras:
    for (int ti = 0; ti < tmesh.n_tetras; ++ti)
    {
        // STEP 2:  Apply elastic force.
        auto tvs = tmesh.tetra_vertices[ti]; // contain indices
        std::array<Vector3f, 4> p;
        p[0] = position[tvs[0]];
        p[1] = position[tvs[1]];
        p[2] = position[tvs[2]];
        p[3] = position[tvs[3]];
        // P = [p_i1 - p_i0, p_i2 - p_i0, p_i3 - p_i0] * \bar{X_i}
        Matrix3f p_123;
        p_123 << (p[1] - p[0]), (p[2] - p[0]), (p[3] - p[0]);
        Matrix3f P = p_123 * X_bar[ti];
        // Spatial derivative
        Matrix3f grad_u = P - Matrix3f::Identity();
        Matrix3f grad_u_T = grad_u.transpose();
        // Strain
        Matrix3f epsilon = 0.5f * (grad_u + grad_u_T + grad_u_T * grad_u);
        // Stress
        Matrix3f sigma = E * epsilon;
        // for all faces on the tetra:
        int face_idxs[4][4] = {{0,1,2,3}, {0,2,3,1}, {0,3,1,2}, {1,2,3,0}}; // j0, j1, j2, j_unuse
        for  (int fi = 0; fi < 4; ++fi)
        {
            int j[4] = { face_idxs[fi][0], face_idxs[fi][1], face_idxs[fi][2], face_idxs[fi][3] };
            // area_normal: face area * normal of the face.
            Vector3f area_normal = 0.5f * (p[j[1]] - p[j[0]]).cross(p[j[2]] - p[j[0]]);
            // Assure that the normal points to outside of the face.
            if (area_normal.dot(p[j[3]] - p[j[0]]) > 0.0f)
                area_normal = -area_normal;
            // Calculate elastic force applied on face and assign it to 3 vertices.
            Vector3f f_face = sigma * area_normal;
            for (int i = 0; i < 3; ++i)
                force[tvs[i]] += 1.0f / 3.0f * f_face;
        }
    }
    // Balance
    for (int vi = 0; vi < tmesh.n_vertices; ++vi)
    {
        if (position[vi][2] <= 0.0f)
        {
            force[vi][2] = 0;
            if (velocity[vi][2] < 0.0f)
                velocity[vi][2] = -velocity[vi][2];
        }
    }
    // update velocity and position
    for (int vi = 0; vi < tmesh.n_vertices; ++vi)
    {
        // v_i += f_i * dt / m_i
        velocity[vi] = velocity[vi] + dt * force[vi] / masses[vi];
        // p_i += v_i * dt
        position[vi] = position[vi] + dt * velocity[vi];
    }
}

void SimulatorSimpleFED::simulate_rebuild()
{
    auto &tmesh = ball->tmesh();
    auto &mesh = ball->mesh();
    for (int vi = 0; vi < tmesh.n_vertices; ++vi)
    {
        if (vi < tmesh.n_vertices_boundary)
        {
            ball->set_point(vi, ev_to_qv(position[vi]));
        }
        tmesh.point[vi] = ev_to_ov(position[vi]);
    }
    ball->update();
}
