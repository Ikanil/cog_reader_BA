#pragma once
#include <string>
#include "cog_types.h"
#include "cog_config.h"
#include <gdal_priv.h>



// später kann mit CogReader reader; ein objekt reader erstellen welches Funktionen 
// wie open, close und isOpen besitzt also alles was im public berecieh deffiniert ist

class CogReader {
    public: 
        CogReader();
        ~CogReader();
        void open(const std::string& url);
        void close();
        bool isOpen() const;
        RasterInfoType rasterInfo() const;
        GeoTransformType geoTransform() const;
        PixelPointType worldToPixel(const WorldPointType& point) const;
        PixelWindowType worldBoundsToPixelWindow(const WorldBoundingBoxType& box) const;
        PreparedWindowType readWindow(const PixelWindowType& window, int band) const;
    private:
        bool p_isOpen = false;
        //dataset_ptr ist mein pointer auf das GDALDataset
        GDALDataset* p_datasetptr = nullptr;
        RasterInfoType p_rasterInfo{};
        GeoTransformType p_geoTransform{};
};