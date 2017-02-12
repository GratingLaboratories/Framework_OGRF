#include "stdafx.h"
#include "SimulatorBase.h"
#include <QString>

void SimulatorBase::init(const double& time)
{
    init_time_ = last_time_ = time;
    auto ball = scene_.get("Ball");
    x_0 = ball->position_;
}

void SimulatorBase::simulate(const double& time)
{
    curr_time_ = time;
    t = curr_time_ - init_time_;
    simulate_util();
    last_time_ = curr_time_;
}

void SimulatorBase::simulate_util()
{
    auto ball = scene_.get("Ball");
    ball->position_ = x_0 + QVector3D{float(sin(t)), 0.0f, 0.0f};
    ball->update();
}

void Simulator::init(const double& t)
{
    SimulatorBase::init(t);
}

void Simulator::simulate_util()
{
}
