#include "sampler.h"
#include <cmath>
#include <stdexcept>

double Sampler::nearest(const PreparedWindowType& block, const PixelPointType& globalPixel) const
{
    const double localColDouble = globalPixel.col - block.window.xOffset;
    const double localRowDouble = globalPixel.row - block.window.yOffset;

    const int localCol = static_cast<int>(std::round(localColDouble));
    const int localRow = static_cast<int>(std::round(localRowDouble));

    if (localCol < 0 || localCol>= block.width || localRow < 0 || localRow >= block.height)
    {
        throw std::runtime_error("Pixel point outside prepared raster block. ");
    }

    const std::size_t index = static_cast<std::size_t>(localRow)*static_cast<std::size_t>(block.width) +
        static_cast<std::size_t>(localCol);
    
    return block.values[index];
}