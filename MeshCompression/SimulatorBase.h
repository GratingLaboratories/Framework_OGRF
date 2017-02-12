#pragma once
#include "OpenGLScene.h"

class SimulatorBase
{
public:
    explicit SimulatorBase(OpenGLScene &scene) : scene_(scene), t(0) {  }
    virtual ~SimulatorBase() {  };
    virtual void init(const double &t);
    virtual void simulate_util();
    void simulate(const double &t);
    double get_time() const { return t; }

protected:
    OpenGLScene &scene_;
    double last_time_;
    double init_time_;
    double curr_time_;

    double t;
    QVector3D x_0;
};

class Simulator : public SimulatorBase
{
public:
    explicit Simulator(OpenGLScene& scene) : SimulatorBase(scene) {  }
    ~Simulator() override {  };
    void init(const double& t) override;
    virtual void simulate_util() override;
};