#pragma once
#include "OpenGLScene.h"
#include <Eigen/Core>
#include <Eigen/Dense>

typedef OpenGLMesh Model;

using Eigen::Matrix3f;
using Eigen::Vector3f;

class SimulatorBase
{
public:
    explicit SimulatorBase(OpenGLScene &scene) : scene_(scene), t(0), init_ok_(false) {  }
    virtual ~SimulatorBase() {  };
    virtual void init(const double &t);
    virtual void simulate_util();
    virtual void simulate_rebuild();
    void simulate(const double &t);
    double get_time() const { return t; }

protected:
    OpenGLScene &scene_;
    bool init_ok_;

    double last_time_;
    double init_time_;
    double curr_time_;

    double t;
    double dt;
};

class SimulatorSimpleFED : public SimulatorBase
{
public:
    explicit SimulatorSimpleFED(OpenGLScene& scene) : SimulatorBase(scene) {  }
    ~SimulatorSimpleFED() override {  };
    void init(const double& t) override;
    void simulate_util() override;
    void simulate_rebuild() override;

protected:
    QVector3D x_0;
    std::shared_ptr<Model> ball;
    std::shared_ptr<Model> ground;
    std::vector<Vector3f> velocity;
    std::vector<Vector3f> position;
    std::vector<float> tetra_volume;
    std::vector<float> vert_volume;
    std::vector<Matrix3f> X_bar;
};