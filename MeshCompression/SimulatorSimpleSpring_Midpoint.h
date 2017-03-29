#pragma once
#include "SimulatorBase.h"
class SimulatorSimpleSpring_Midpoint :
    public SimulatorSimpleSpring
{
public:
    explicit SimulatorSimpleSpring_Midpoint(OpenGLScene& scene) : SimulatorSimpleSpring(scene) {  }
    ~SimulatorSimpleSpring_Midpoint() override {  }
    void simulate_util() override;
};

