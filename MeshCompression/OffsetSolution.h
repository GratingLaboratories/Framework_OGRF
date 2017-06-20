#pragma once
//#include "OpenGLScene.h"
//#include "ConsoleMessageManager.h"
//#include "TextConfigLoader.h"
//#include <vector>
//#include <array>
//#include <Eigen/Dense>
//#include <Eigen/Sparse>
//#include <engine.h>
//
//using std::vector;
//using std::array;
//
//using Vector3f = Eigen::Vector3f;
//using std::vector;
//using SpMat = Eigen::SparseMatrix<float>;
//using Mat = Eigen::MatrixXf;
//using Vec = Eigen::VectorXf;
//using T = Eigen::Triplet<float>;
//
//class OffsetSolution
//{
//public:
//    OffsetSolution(OpenGLScene &scene, ConsoleMessageManager &msg) :
//        msg_(msg), scene_(scene), tcl_{ "./config/offset.config" } {  }
//    ~OffsetSolution();
//    void offset();
//
//private:
//    ConsoleMessageManager &msg_;
//    OpenGLScene &scene_;
//    TextConfigLoader tcl_;
//    bool verbose_;
//
//    int n_vertices;
//    int n_edges;
//    int n_faces;
//
//    vector<Vector3f>        positions;
//    vector<Vector3f>        offset_vector;
//    vector<float>           offset_bounds;
//    vector<vector<bool>>    adjacency;
//    vector<int>             degrees;
//    vector<vector<int>>     neighbors;
//    vector<array<int, 3>>   triangles;
//
//    float _cotangent_for_angle_AOB(int, int, int);
//    void Basic_Prepare_and_Calculate_Laplacian(vector<T>& tv_Lap);
//    void Calculate_OffsetVector();
//    void Input_Variables_to_Engine(Engine* ep);
//    void Input_Laplacian_to_Engine(const vector<T> &tv_Lap, Engine* ep);
//    void Input_Positions_to_Engine(Engine* ep);
//    void Input_Topology_to_Engine(Engine* ep);
//    void Input_OffsetVector_to_Engine(Engine* ep);
//};
