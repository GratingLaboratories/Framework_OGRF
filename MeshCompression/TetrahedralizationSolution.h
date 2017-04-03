#pragma once
#include "OpenMeshBasic.h"
#define TETLIBRARY
#include <tetgen.h> 
#include <string>
#include <cstring>

#define TETRA_ELE_EXTENSION ".ele"

class TetrahedralizationSolution 
{
public:
    TetrahedralizationSolution() = delete;
    TetrahedralizationSolution(const TriMesh &mesh, const std::string &name) 
        : mesh_(mesh)
    {
        name_ = new char[name.size() + 1];
        name_ori_ = new char[name.size() + 1 + 4]; // + ".ori"
        strcpy_s(name_, name.size() + 1, name.c_str());
        strcpy_s(name_ori_, name.size() + 1 + 4, name.c_str());
        strcat_s(name_ori_, name.size() + 1 + 4, ".ori");
    }
    bool tetra();
    ~TetrahedralizationSolution();

private:
    const TriMesh &mesh_;
    char *name_;
    char *name_ori_;
};

