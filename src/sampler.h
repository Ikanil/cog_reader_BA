#pragma once
#include "cog_types.h"

class Sampler
{
    public:
        double nearest(const PreparedWindowType& block, const PixelPointType& globalPixel) const;
};