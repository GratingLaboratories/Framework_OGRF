#include "stdafx.h"

#include "TetrahedralizationSolution.h"

bool TetrahedralizationSolution::tetra()
{
    tetgenio in, out;
    tetgenio::facet *f;
    tetgenio::polygon *p;

    // All indices start from 1.
    in.firstnumber = 0;

    in.numberofpoints = mesh_.n_vertices();
    in.pointlist = new REAL[in.numberofpoints * 3];
    for (auto v_it : mesh_.vertices())
    {
        auto point = mesh_.point(v_it);
        in.pointlist[v_it.idx() * 3 + 0] = point[0];
        in.pointlist[v_it.idx() * 3 + 1] = point[1];
        in.pointlist[v_it.idx() * 3 + 2] = point[2];
    }

    in.numberoffacets = mesh_.n_faces();

    in.facetlist = new tetgenio::facet[in.numberoffacets];
    in.facetmarkerlist = new int[in.numberoffacets];

    for (auto f_it : mesh_.faces())
    {
        f = &in.facetlist[f_it.idx()];
        f->numberofpolygons = 1;
        f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
        f->numberofholes = 0;
        f->holelist = NULL;
        p = &f->polygonlist[0];
        p->numberofvertices = 3;
        p->vertexlist = new int[p->numberofvertices];

        in.facetmarkerlist[f_it.idx()] = 0;

        auto fv_it = mesh_.cfv_iter(f_it);
        int i = 0;
        for (; fv_it; ++fv_it)
        {
            p->vertexlist[i++] = fv_it->idx();
        }
    }
   

    // Tetrahedralize the PLC. Switches are chosen to read a PLC (p),
    //   do quality mesh generation (q) with a specified quality bound
    //   (1.414), and apply a maximum volume constraint (a0.1).

    tetrahedralize("pq1.414a0.1", &in, &out);

    in.save_nodes(name_ori_);
    in.save_poly(name_ori_);

    out.save_nodes(name_);
    out.save_elements(name_);
    out.save_faces(name_);

    return true;
}

TetrahedralizationSolution::~TetrahedralizationSolution()
{
    delete[] name_;
    delete[] name_ori_;
}
