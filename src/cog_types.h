#pragma once
#include <vector>

// Von UG4 erhaltene Koordinaten eines Auswertungspunktes (als Weltkoordinaten interpretiert)
struct  WorldPointType
{
    double x;
    double y;
};


// Punkt im Rasterkoordinatensystem
struct PixelPointType
{
    double col;
    double row;
};


// UG4 Element ist z.B dreieck also boundingbox die dieses Element umschließt in Weltkoordinaten
struct WorldBoundingBoxType
{
    double minX;
    double minY;
    double maxX;
    double maxY;
};


// die Pixelkoordinaten Version der boundingBox
struct PixelWindowType
{
    int xOffset;
    int yOffset;
    int width;
    int height;
};


// GeoTransfrom in COG beschreibt die beziehung zwischen Pixel und Weltkoordinaten
struct GeoTransformType
{
    double originX;
    double pixelWidth;
    double rotationX;

    double originY;
    double rotationY;
    double pixelHeight;
};


// Zwischenspeicher zwischen prepare und eval. 
struct PreparedWindowType
{
    PixelWindowType window;
    int width;
    int height;
    std::vector<double> values;
};

struct RasterInfoType
{
    int width;
    int height;
    int bandCount;
};