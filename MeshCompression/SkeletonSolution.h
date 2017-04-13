#pragma once

#include "OpenGLMesh.h"
#include "ConsoleMessageManager.h"
#include <vector>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <engine.h>
#include <TextConfigLoader.h>

using Vector3f = Eigen::Vector3f;
using std::vector;
using SpMat = Eigen::SparseMatrix<float>;
using Mat = Eigen::MatrixXf;
using Vec = Eigen::VectorXf;
using T = Eigen::Triplet<float>;

class SkeletonSolution
{
public:
    SkeletonSolution(TriMesh &mesh, ConsoleMessageManager &msg);
    ~SkeletonSolution();

    void skeletonize();

private:
    ConsoleMessageManager &msg_;
    TextConfigLoader tcl_;
    TriMesh &mesh_;

    int n_vertices;
    int n_edges;
    int n_faces;

    vector<Vector3f>        positions;
    vector<vector<bool>>    adjacency;
    vector<int>             degrees;
    vector<vector<int>>     neighbors;

    float _cotangent_for_angle_AOB(int, int, int);
    void Basic_Prepare_and_Calculate_Laplacian(vector<T>& tv_Lap);
    void Input_Variables_to_Engine(Engine* ep);
    void Input_Laplacian_to_Engine(const vector<T> &tv_Lap, Engine* ep);
    void Input_Positions_to_Engine(Engine* ep);
};

