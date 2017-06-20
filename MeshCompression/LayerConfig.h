#pragma once

#include <string>

#define MAX_LAYER 25

struct LayerConfig
{
    std::string mask{"00111022200"};
    int  num_layer{11};
    float ppl{11.3f};
    int   offset_block{0};
    float offset_grid{0.0f};
};