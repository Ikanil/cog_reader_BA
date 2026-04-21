// speichert nur Daten wie Band, Weltfenster, Pixelfenster, Breite und Höge, Werte
#pragma once
#include <vector>
#include "CogTypes.h"

struct RasterWindowType
{
    int bandIndex = 1;
    BoundingBoxWorldType worldWindow;
    BoundingBoxPixelsType pixelWindow;
    int width = 0;
    int height = 0;
    std::vector<double> values;
};