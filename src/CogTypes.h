/*
Definieren eigener Datentypen. STatte überall einzelne variablen rumliegen zu haben, gruppiere ich sie in logische EInheiten.
Ein struct ist ein zusammengesetzter Datentype, z.B:
struct RasterInfo
{
    int width = 0;
    int height = 0;
    int bandCount = 0;
};
Es gibt jetzt einen neuen Typ namens RasterInfo, und der enthält drei Integer-Werte:
width, height, bandCount.
Später kann ich dann folgendes machen um info die drei Teile info.width, info.height und info.bandCount zu geben:
RasterInfo info;
A. Raster-Infos --> raster_width, raster_height, raster_count
B. Band-Infos --> Datentyp, Color Interpretation, OverviewCount, Minimum, Maximum
C. Bounding Box in Pixelkoordinaten --> x_off, y_off, x_end, y_end
D. Bounding-Box-Ergebnis --> bbox_width, bbox_height, bbox_values
*/

#pragma once
#include <string>
#include <vector>

// Speichert allgemeine Informationen über das gesamte Raster
struct RasterInfoType
{
    int width = 0;        // Anzahl der Pixel in x-Richtung
    int height = 0;       // Anzahl der Pixel in y-Richtung
    int bandCount = 0;    // Anzahl der vorhandenen Rasterbänder
};

// Speichert Informationen über ein einzelnes Rasterband
struct BandInfoType
{
    std::string dataTypeName;              // Datentyp des Bandes
    std::string colorInterpretationName;   // Bedeutung des Bandes
    int overviewCount = 0;                 // Anzahl der Overviews
    double minimum = 0.0;                  // Minimalwert des Bandes
    double maximum = 0.0;                  // Maximalwert des Bandes
};

// Beschreibt einen rechteckigen Ausschnitt über Pixelkoordinaten
struct BoundingBoxPixelsType
{
    int xOff = 0;   // Linke Grenze
    int yOff = 0;   // Obere Grenze
    int xEnd = 0;   // Rechte Grenze
    int yEnd = 0;   // Untere Grenze
};

// Enthält das Ergebnis eines gelesenen Rasterausschnitts
struct BoundingBoxResultType
{
    int width = 0;                 // Breite des Ausschnitts
    int height = 0;                // Höhe des Ausschnitts
    std::vector<double> values;    // Pixelwerte des Ausschnitts
};

// Speichert die Georeferenzierungs-Transformation zwischen Raster- und Weltkoordinaten
struct GeoTransformType
{
    double xOrigin = 0.0;      // Welt-X des Ursprungs
    double pixelWidth = 0.0;   // Pixelgröße in x-Richtung (veränderung der Koordinaten je Pixel)
    double xRotation = 0.0;    // Rotation/Scherung des Rasters
    double yOrigin = 0.0;      // Welt-Y des Ursprungs
    double yRotation = 0.0;    // Rotation/Scherung in y
    double pixelHeight = 0.0;  // Pixelgröße in y-Richtung
    bool valid = false;        // Wurde erfolgreich aus dem Dataset gelesen?
};


struct BoundingBoxWorldType
{
    double minX = 0.0;
    double minY = 0.0;
    double maxX = 0.0;
    double maxY = 0.0;
};

struct PixelPositionType
{
    double x = 0.0;
    double y = 0.0;
};