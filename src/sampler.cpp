#include "sampler.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>


double Sampler::nearest(const PreparedWindowType& block, const PixelPointType& globalPixel) const
{
    // Punkt wird als globale pixelposition übergeben, block.values enthält aber nur das Fenster um das UG4 element herum.
    // Deshalb muss in lokale pixelkoordinaten die innerhalb dieses Fensters liegen umgerechnet werden.
    const double localColDouble = globalPixel.col - block.window.xOffset;
    const double localRowDouble = globalPixel.row - block.window.yOffset;

    int localCol = static_cast<int>(std::round(localColDouble));
    int localRow = static_cast<int>(std::round(localRowDouble));

    localCol = std::max(0, std::min(localCol, block.width - 1));
    localRow = std::max(0, std::min(localRow, block.height - 1));

    if (localCol < 0 || localCol>= block.width || localRow < 0 || localRow >= block.height)
    {
        throw std::runtime_error("Pixel point outside prepared raster block. ");
    }

    // 2D -> 1D umrechnung und Index bestimmnug
    const std::size_t index = static_cast<std::size_t>(localRow)*static_cast<std::size_t>(block.width) +
        static_cast<std::size_t>(localCol);
    
    return block.values[index];
}