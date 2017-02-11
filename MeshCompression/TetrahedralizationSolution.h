#pragma once
#include "OpenMeshBasic.h"
#define TETLIBRARY
#include <tetgen.h> 

class TetrahedralizationSolution 
{
public:
    TetrahedralizationSolution() = delete;
    TetrahedralizationSolution(const TriMesh &mesh) : mesh_(mesh) {   }
    bool tetra();
    ~TetrahedralizationSolution();

private:
    const TriMesh &mesh_;
};

