#pragma once
#include <string>


enum class InterpolationModeConfig
{
    Nearest, Bilinear
};


struct CogConfig
{
    std::string url;
    int band = 1;
    InterpolationModeConfig interpolation = InterpolationModeConfig::Nearest;
};