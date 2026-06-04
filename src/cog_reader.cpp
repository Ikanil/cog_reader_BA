#include "cog_reader.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <vector>
#include <utility>


CogReader::CogReader() {}


CogReader::~CogReader() 
{
    close();
}


void CogReader::open(const std::string& url)
{
    if (p_isOpen) //falls was geöffnet ist erst schließen
    {
        close();
    }

    //Registriert die GDAL Treiber damit GDAL weiß welche Dateiformate es lesen kann wie zb TIFF
    GDALAllRegister();

    // std::string wird in c string gecastet für GDAL
    p_datasetptr = static_cast<GDALDataset*>(GDALOpen(url.c_str(), GA_ReadOnly)); 

    // wenn p_datasetptr leer also nullptr dann konnte GDAL nichts öffnen
    if  (p_datasetptr == nullptr) 
    {
        throw std::runtime_error("Couldnt open dataset: " + url);
    }

    // p_datasetptr zeigt auf GDALDataset und speichert in .width die breite des TIFF
    p_rasterInfo.width = p_datasetptr -> GetRasterXSize();
    p_rasterInfo.height = p_datasetptr-> GetRasterYSize();
    p_rasterInfo.bandCount = p_datasetptr -> GetRasterCount();

    // Erstell Array geoTransformArr
    double geoTransformArr[6];
    // gdal nimmt sich geotransform aus dem geöffnetet dataset und schreibt die 6 Werte ins Array
    if (p_datasetptr->GetGeoTransform(geoTransformArr) != CE_None)
    {
        throw std::runtime_error("Couldnt read GeoTransform. ");
    }
    // kopier die Werte aus dem Array in die private variable p_geotransfrom vom Typ GeoTransformType
    p_geoTransform.originX = geoTransformArr[0];
    p_geoTransform.pixelWidth = geoTransformArr[1];
    p_geoTransform.rotationX = geoTransformArr[2];
    p_geoTransform.originY = geoTransformArr[3];
    p_geoTransform.rotationY = geoTransformArr[4];
    p_geoTransform.pixelHeight = geoTransformArr[5];

    // wenn öffnen geklappt hat wird p_isOpen auf true gesetzt
    p_isOpen = true; 
}


void CogReader::close()
{
    // Wenn Dataset geöffnet also nicht auf nullptr ist dann schließen und p_datasetptr auf nullptr setzen
    if (p_datasetptr != nullptr)
    {
        GDALClose(p_datasetptr);
        p_datasetptr = nullptr;
    }
    p_isOpen = false;
}


bool CogReader::isOpen() const
{
    return p_isOpen;
}


RasterInfoType CogReader::rasterInfo() const
{
    return p_rasterInfo;
}


GeoTransformType CogReader::geoTransform() const
{
    return p_geoTransform;
}


/*
UG4 arbeitet mit Weltkoordinaten x,y und das Raster mit Pixelkoordinaten
GDAl beschreibt mit GeoTransfrom Pixel zu Welt:
M=  x = originX + pixelWidth * col + rotationX * row
    y = originY + rotationY   * col + pixelHeight * row
wir brauchen aber welt zu pixelposition, deshlab wird diese Matrix invertiert
*/ 
// bekommt weltkoordinaten punkt "point"
PixelPointType CogReader::worldToPixel(const WorldPointType& point) const
{
    // Abstand vom ursprung in X und Y richtung
    const double dx = point.x - p_geoTransform.originX; 
    const double dy = point.y - p_geoTransform.originY;
    // Der GeoTransform beschreibt eine 2x2 matrix: [ pixelWidth   rotationX   ] 
    // der beschreibt wie Pixel in Weltkoorddinaten [ rotationY    pixelHeight ]
    // umgerechent werden                                           
    // Um Welt -> Pixel zu kriegen muss Matrix umgekehrt werden was nur geht wenn determinante nicht 0
    const double det = p_geoTransform.pixelWidth * p_geoTransform.pixelHeight
                    - p_geoTransform.rotationX * p_geoTransform.rotationY;
    if (det == 0.0)
    {
        throw std::runtime_error("GeoTransform is not invertible.");
    }
    // Ergebnisvariable vom pixel type namens pixel
    PixelPointType pixel;

    // hier berechnet sich dann die Spalte und Zeile im Raster
    /*
    dx = 30
    pixelWidth = 10
    col = 30 / 10 = 3
    */
    pixel.col = (p_geoTransform.pixelHeight * dx - p_geoTransform.rotationX * dy) / det;
    pixel.row = (-p_geoTransform.rotationY * dx + p_geoTransform.pixelWidth * dy) / det;

    return pixel;
}

// Erhält BBox in Weltkoordinaten (minX, maxX,...) und transformiert daraus Pixel Fenster für 
// GDAL (xOffset, width,...)
PixelWindowType CogReader::worldBoundsToPixelWindow(const WorldBoundingBoxType& box) const
{
    // die vier Ecken der Bounding Box, z.B. minX, minY (unten links) werden an worldToPixel 
    // übergeben und erhalten 4 Punkte z.B p1
    // wir nehmen alle 4 da Rasterachsen manchmal negativ verlaufen besonders bei pixelHeight.
    const PixelPointType p1 = worldToPixel(WorldPointType{box.minX, box.minY}); // links unten
    const PixelPointType p2 = worldToPixel(WorldPointType{box.minX, box.maxY}); // links oben
    const PixelPointType p3 = worldToPixel(WorldPointType{box.maxX, box.minY}); // rechts unten
    const PixelPointType p4 = worldToPixel(WorldPointType{box.maxX, box.maxY}); // rechts oben

    // hier wird in jeder Zeile die kleinsten und größten row und col ermittelt
    const double minColDouble = std::min(std::min(p1.col, p2.col), std::min(p3.col, p4.col));
    const double maxColDouble = std::max(std::max(p1.col, p2.col), std::max(p3.col, p4.col));
    const double minRowDouble = std::min(std::min(p1.row, p2.row), std::min(p3.row, p4.row));
    const double maxRowDouble = std::max(std::max(p1.row, p2.row), std::max(p3.row, p4.row));

    // die Werte von werden nach unten und oben gerundet und als int gecastet
    int minCol= static_cast<int>(std::floor(minColDouble))-1;
    int maxCol= static_cast<int>(std::ceil(maxColDouble))+1;
    int minRow= static_cast<int>(std::floor(minRowDouble))-1;
    int maxRow= static_cast<int>(std::ceil(maxRowDouble))+1;

    // nimm den größeren wert von 0 und minCol denn falls minCol negativ ist wird das Fenster richtig abgeschnitten
    minCol = std::max(0, minCol);
    minRow = std::max(0, minRow);
    maxCol = std::min(p_rasterInfo.width, maxCol);
    maxRow = std::min(p_rasterInfo.height, maxRow);

    // Offset ist der anfang des Fensters und width und height beschreiben wie breit und hoch ausgehend 
    // vom offset das Fenster sein wird
    PixelWindowType window;
    window.xOffset = minCol;
    window.yOffset = minRow;
    window.width = maxCol - minCol;
    window.height = maxRow - minRow;

    // existiert überhaupt ein Fenster > 0 prüfen
    if (window.width <= 0 || window.height <=0)
    {
        throw std::runtime_error("Boudning box does not overlap raster.");
    }
    return window;
}  


PreparedWindowType CogReader::readWindow(const PixelWindowType& window, int band) const
{
    if(p_datasetptr == nullptr)
    {
        throw std::runtime_error("No Dataset is open.");
    }
    if(band <1 || band > p_rasterInfo.bandCount)
    {
        throw std::runtime_error("Invalid band Index.");
    }
    // liegt Fenster innerhalb des Rasters
    if (window.xOffset < 0 || window.yOffset < 0 || window.width <= 0 || window.height <=0 ||
        window.xOffset + window.width > p_rasterInfo.width || window.yOffset + window.height > p_rasterInfo.height)
    {
        throw std::runtime_error("Invalid raster window.");
    }
    // Speicher für Pixelwerte allokieren und Band holen
    std::vector<double> values(static_cast<std::size_t>(window.width)*static_cast<std::size_t>(window.height));
    GDALRasterBand* rasterBand = p_datasetptr->GetRasterBand(band);

    if (rasterBand == nullptr)
    {
        throw std::runtime_error("Couldnt get raster Band.");
    }

    // liest aus rasterBand ein Rechteck ab Position xOffset/yOffset mit größe
    // width/height und schreibe die Werte als double in den Vektor values
    CPLErr err = rasterBand->RasterIO(
        GF_Read, window.xOffset, window.yOffset, window.width, window.height, values.data(),
        window.width, window.height, GDT_Float64, 0, 0);
    
    if (err != CE_None)
    {
        throw std::runtime_error("RasterIO failed. ");
    }

    PreparedWindowType result;
    result.window = window;
    result.width = window.width;
    result.height = window.height;
    result.values = std::move(values);
    return result;

}