// Hier kommt die komplette GDAL-Logik rein.

#include "CogReader.h"
#include <stdexcept>
#include <cmath>
#include <algorithm>

CogReader::CogReader()
{
    GDALAllRegister();
}

CogReader::~CogReader()
{
    close();
}

bool CogReader::open(const std::string& url)
{
    close();
    m_dataset = static_cast<GDALDataset*>(GDALOpen(url.c_str(), GA_ReadOnly));
    return m_dataset != nullptr;
}

void CogReader::close()
{
    if (m_dataset != nullptr)
    {
        GDALClose(m_dataset);
        m_dataset = nullptr;
    }
}

bool CogReader::isOpen() const
{
    return m_dataset != nullptr;
}

RasterInfoType CogReader::getRasterInfo() const
{
    RasterInfoType info;
    info.width = m_dataset->GetRasterXSize();
    info.height = m_dataset->GetRasterYSize();
    info.bandCount = m_dataset->GetRasterCount();
    return info;
}

bool CogReader::isValidBand(int bandIndex) const
{
    if (!m_dataset) return false;
    return bandIndex >= 1 && bandIndex <= m_dataset->GetRasterCount();
}

BandInfoType CogReader::getBandInfo(int bandIndex) const
{
    GDALRasterBand* band = m_dataset->GetRasterBand(bandIndex);

    BandInfoType info;
    info.dataTypeName = GDALGetDataTypeName(band->GetRasterDataType());
    info.colorInterpretationName = GDALGetColorInterpretationName(band->GetColorInterpretation());
    info.overviewCount = band->GetOverviewCount();
    info.minimum = band->GetMinimum();
    info.maximum = band->GetMaximum();

    return info;
}

bool CogReader::isValidBoundingBox(const BoundingBoxPixelsType& bbox) const
{
    if (!m_dataset) return false;

    int width = m_dataset->GetRasterXSize();
    int height = m_dataset->GetRasterYSize();

    return !(bbox.xOff < 0 || bbox.xOff >= width ||
             bbox.yOff < 0 || bbox.yOff >= height ||
             bbox.xEnd < 0 || bbox.xEnd >= width ||
             bbox.yEnd < 0 || bbox.yEnd >= height ||
             bbox.xEnd <= bbox.xOff ||
             bbox.yEnd <= bbox.yOff);
}

BoundingBoxResultType CogReader::readBoundingBox(int bandIndex, const BoundingBoxPixelsType& bbox) const
{
    GDALRasterBand* band = m_dataset->GetRasterBand(bandIndex);

    BoundingBoxResultType result;
    result.width = bbox.xEnd - bbox.xOff;
    result.height = bbox.yEnd - bbox.yOff;
    result.values.resize(result.width * result.height);

    CPLErr err = band->RasterIO(
        GF_Read,
        bbox.xOff,
        bbox.yOff,
        result.width,
        result.height,
        result.values.data(),
        result.width,
        result.height,
        GDT_Float64,
        0,
        0
    );

    if (err != CE_None)
    {
        throw std::runtime_error("Bounding Box konnte nicht gelesen werden.");
    }

    return result;
}

GeoTransformType CogReader::readGeoTransformation() const
{
    GeoTransformType info;  // Variable namens info vom Typ GeoTransformType uas der CogTypes.h
    if (!m_dataset)
    {
        return info;
    }
    double gtArray[6];  // Array mit 6 doubles da Gdal die Geotransformation als 6 Zahlen liefert
    CPLErr err = m_dataset -> GetGeoTransform(gtArray);     // CPLErr ist der Rückgabetyp von GDAL für den Status einer Operation.
    if (err == CE_None)   // CE_None bedeutet: kein Fehler, GDAL konnte die Geotransformation erfolgreich lesen
    {
        info.xOrigin = gtArray[0];
        info.pixelWidth = gtArray[1];
        info.xRotation = gtArray[2];
        info.yOrigin = gtArray[3];
        info.yRotation = gtArray[4];
        info.pixelHeight = gtArray[5];
        info.valid = true;
    }
    return info;
}

// Erhält einen Punkt in Weltkoordinaten
// holt sich GeoTransform mit readGeoTransformation -> GeoTransform sagt wie Rasterkoordinaten mit Weltkoordinaten zusammen hängen
// holt sich aus dem GeoTransform die 4 Werte die die lineare abbildung beschrieben also a,b,c,d.
// Prüft ob Transformation umkehrbar um Welt -> Ratser zu machen. Dafür wird die invberse abbildung benötigt
// Verschiebt dann dernn Weltpunkt relativ zum Ursprung vom Raster, dx = wie weit ist Punkt X vom Rasterursprung entfernt
// Rechnet aus an welcher Pixelposition der Weltpunkt liegt und kriegt einen Punkt im Pixelraum über pixel.x und pixel.y
PixelPositionType CogReader:: worldToPixelPosition (double worldX, double worldY) const
{
    if (!m_dataset)
    {
        throw std::runtime_error("Kein Dataset geoeffnet");
    }

    GeoTransformType transform = readGeoTransformation();
    if (!transform.valid)
    {
        throw std:: runtime_error("Geotransformation konnte nicht gelesen werden");
    }

    double a = transform.pixelWidth;
    double b = transform.xRotation;
    double c = transform.yRotation;
    double d = transform.pixelHeight;
    double det = a*d-b*c;
    if (det == 0.0)
    {
        throw std::runtime_error("Geotransformation ist nicht invertierbar");
    }
    double dx = worldX - transform.xOrigin;
    double dy = worldY -transform.yOrigin;
    PixelPositionType pixel;
    pixel.x = (d*dx-b*dy) / det;
    pixel.y = (-c*dx+a*dy) /det;
    return pixel;
}



//  Erhält eine komplette bbox in Weltkoordinaten minX, maxY etc
BoundingBoxPixelsType CogReader:: worldToPixelBoundingBox(const BoundingBoxWorldType& bboxWorld) const
{
// nimmt vier ecken der welt bounding box und rechnet jede Ecke mit worldToPixelPosition in den pixelraum um 
    PixelPositionType p1 =worldToPixelPosition(bboxWorld.minX, bboxWorld.minY);
    PixelPositionType p2 =worldToPixelPosition(bboxWorld.minX, bboxWorld.maxY);
    PixelPositionType p3 =worldToPixelPosition(bboxWorld.maxX, bboxWorld.minY);
    PixelPositionType p4 =worldToPixelPosition(bboxWorld.maxX, bboxWorld.maxY);
// sucht aus diesen 4 punkten den kleinsten x und y wert und den größten x und y wert um ein Rechteck zu bauen was das ganze umschließt
    double minPixelX = std::min(std::min(p1.x, p2.x), std::min(p3.x, p4.x));
    double maxPixelX = std::max(std::max(p1.x, p2.x), std::max(p3.x, p4.x));
    double minPixelY = std::min(std::min(p1.y, p2.y), std::min(p3.y, p4.y));
    double maxPixelY = std::max(std::max(p1.y, p2.y), std::max(p3.y, p4.y));
    BoundingBoxPixelsType bboxPixels;
// floor links/oben und ceil rechts/unten um das gesamte fenster zu beachten
    bboxPixels.xOff = static_cast<int>(std::floor(minPixelX));
    bboxPixels.yOff = static_cast<int>(std::floor(minPixelY));
    bboxPixels.xEnd = static_cast<int>(std::ceil(maxPixelX));
    bboxPixels.yEnd = static_cast<int>(std::ceil(maxPixelY));
    int rasterWidth = m_dataset->GetRasterXSize();
    int rasterHeight = m_dataset->GetRasterYSize();
    bboxPixels.xOff = std::max(0, bboxPixels.xOff);
    bboxPixels.yOff = std::max(0, bboxPixels.yOff);
    bboxPixels.xEnd = std::min(rasterWidth, bboxPixels.xEnd);
    bboxPixels.yEnd = std::min(rasterHeight, bboxPixels.yEnd);

    if (bboxPixels.xEnd <= bboxPixels.xOff || bboxPixels.yEnd <= bboxPixels.yOff)
    {
        throw std::runtime_error("Welt-Bounding-Box ergibt kein gültiges Pixel Fenster");
    }
    return bboxPixels;
}