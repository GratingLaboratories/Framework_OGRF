#include "stdafx.h"
#include "SimulatorSimpleSpring_Midpoint.h"

using std::vector;

void SimulatorSimpleSpring_Midpoint::simulate_util()
{
    const float density = 1000.0f; // \ro: 1000 kg/m^3, water like.
    const Vector3f g{ 0.0f, 0.0f, -9.8f }; // g: jyokuryo kassodoku.
    const float k = 200000.0f; // k;
    const float mu = 0.03f; // mu;
    auto &tmesh = ball->tmesh();
    std::vector<Vector3f> force(tmesh.n_vertices, { 0,0,0 });
    std::vector<float> masses(tmesh.n_vertices);
    // for all vertices:
    for (int vi = 0; vi < tmesh.n_vertices; ++vi)
    {
        // STEP 1:  Apply external forces.
        float m = density * vert_volume[vi];   // mass of the vertex, derived from volume.
                                               // UNIFY MASS
        m = 1.0f;

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
        std::array<Vector3f, 4> p_ori;
        p_ori[0] = position_original[tvs[0]];
        p_ori[1] = position_original[tvs[1]];
        p_ori[2] = position_original[tvs[2]];
        p_ori[3] = position_original[tvs[3]];

        int edge_idxs[6][2] = { { 0,1 },{ 0,2 },{ 0,3 },{ 1,2 },{ 1,3 },{ 2,3 } };
        for (int ei = 0; ei < 6; ++ei)
        {
            int i1 = edge_idxs[ei][0], i2 = edge_idxs[ei][1];
            Vector3f l = p[i1] - p[i2];
            Vector3f l_ori = p_ori[i1] - p_ori[i2];
            float delta = l.norm() - l_ori.norm();
            if (abs(delta) > l_ori.norm() * 0.5f)
                delta = delta * 3.0f;
            float force_value = k * (l.norm() - l_ori.norm()); // positive->compressed; negative->stressed
            force[tvs[i1]] += force_value * -l.normalized();
            force[tvs[i2]] += force_value * l.normalized();
        }
    }
    // Balance
    for (int vi = 0; vi < tmesh.n_vertices; ++vi)
    {
        // if (position[vi][2] <= 0.0f && position[vi][1] <= 0.25f / 0.33f)
        if (position[vi][2] <= 0.0f)
        {
            force[vi][2] += -position[vi][2] * k * 10.0f;
        }
    }

    auto mid_point = vector<Vector3f>(tmesh.n_vertices, { 0,0,0 });
    // update velocity and position
    for (int vi = 0; vi < tmesh.n_vertices; ++vi)
    {
        // v_i += f_i * dt / m_i
        Vector3f velocity_temp = velocity[vi] + dt * force[vi] / masses[vi];
        // p_i += v_i * dt
        //position[vi] = position[vi] + dt * velocity[vi];
        mid_point[vi] = position[vi] + dt * velocity_temp * 0.5f; // mid point.
    }

    // STEP 3, use mid points to get velocity.
    force = vector<Vector3f>(tmesh.n_vertices, { 0,0,0 }); // clear
    // for all vertices:
    for (int vi = 0; vi < tmesh.n_vertices; ++vi)
    {
        // STEP 3.1:  Apply external forces.
        float m = masses[vi];

        // STEP 3.1.1 Gravity.
        Vector3f f_g = m * g;

        // STEP 3.1.2 Collision force.
        // Not here,
        // we do a force balance after elastic force has been calculated.

        force[vi] += f_g;
    }
    // for all tetras:
    for (int ti = 0; ti < tmesh.n_tetras; ++ti)
    {
        // STEP 2:  Apply elastic force.
        auto tvs = tmesh.tetra_vertices[ti]; // contain indices
        std::array<Vector3f, 4> p;
        p[0] = mid_point[tvs[0]];
        p[1] = mid_point[tvs[1]];
        p[2] = mid_point[tvs[2]];
        p[3] = mid_point[tvs[3]];
        std::array<Vector3f, 4> p_ori;
        p_ori[0] = position_original[tvs[0]];
        p_ori[1] = position_original[tvs[1]];
        p_ori[2] = position_original[tvs[2]];
        p_ori[3] = position_original[tvs[3]];

        int edge_idxs[6][2] = { { 0,1 },{ 0,2 },{ 0,3 },{ 1,2 },{ 1,3 },{ 2,3 } };
        for (int ei = 0; ei < 6; ++ei)
        {
            int i1 = edge_idxs[ei][0], i2 = edge_idxs[ei][1];
            Vector3f l = p[i1] - p[i2];
            Vector3f l_ori = p_ori[i1] - p_ori[i2];
            float delta = l.norm() - l_ori.norm();
            if (abs(delta) > l_ori.norm() * 0.5f)
                delta = delta * 3.0f;
            float force_value = k * (l.norm() - l_ori.norm()); // positive->compressed; negative->stressed
            force[tvs[i1]] += force_value * -l.normalized();
            force[tvs[i2]] += force_value * l.normalized();
        }
    }
    // Balance
    for (int vi = 0; vi < tmesh.n_vertices; ++vi)
    {
        // if (position[vi][2] <= 0.0f && position[vi][1] <= 0.25f / 0.33f)
        if (mid_point[vi][2] <= 0.0f)
        {
            force[vi][2] += -mid_point[vi][2] * k * 10.0f;
        }
    }

    // Final
    // update velocity and position
    for (int vi = 0; vi < tmesh.n_vertices; ++vi)
    {
        // v_i += f_i * dt / m_i
        velocity[vi] = velocity[vi] + dt * force[vi] / masses[vi];
        // p_i += v_i * dt
        position[vi] = position[vi] + dt * velocity[vi];
    }
}
