/*
Nur für technischen Zugriff auf ein COG: öffnen, Metadaten lesen, Koordinaten umrechnen, Rasterfebster lesen.
*/
#pragma once
#include <string>
#include <gdal_priv.h>
#include "CogTypes.h"

class CogReader
{
public:
    CogReader();
    ~CogReader();

    bool open(const std::string& url);
    void close();
    bool isOpen() const;
    RasterInfoType getRasterInfo() const;
    bool isValidBand(int bandIndex) const;
    BandInfoType getBandInfo(int bandIndex) const;
    bool isValidBoundingBox(const BoundingBoxPixelsType& bbox) const;
    BoundingBoxResultType readBoundingBox(int bandIndex, const BoundingBoxPixelsType& bbox) const;
    GeoTransformType readGeoTransformation() const;
    BoundingBoxPixelsType worldToPixelBoundingBox(const BoundingBoxWorldType& bboxWorld) const;
    PixelPositionType worldToPixelPosition(double worldX, double worldY) const;


private:
    GDALDataset* m_dataset = nullptr;
};