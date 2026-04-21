#include <iostream>
#include "CogReader.h"
#include "COGEvalStrategy.h"

int main()
{
    const std::string url =
        "/vsicurl/https://sentinel-cogs.s3.us-west-2.amazonaws.com/"
        "sentinel-s2-l2a-cogs/36/Q/WD/2020/7/"
        "S2A_36QWD_20200701_0_L2A/B08.tif";

    CogReader reader;

    try
    {
        if (!reader.open(url))
        {
            std::cout << "COG konnte nicht geoeffnet werden.\n";
            return 1;
        }

        std::cout << "COG erfolgreich geoeffnet.\n";

        GeoTransformType transformation = reader.readGeoTransformation();
        if (!transformation.valid)
        {
            std::cout << "Geotransformation konnte nicht gelesen werden.\n";
            return 1;
        }


        int bandIndex = 1;

        COGEvalStrategy strategy(reader);

        BoundingBoxWorldType bboxWorld;
        bboxWorld.minX = transformation.xOrigin;
        bboxWorld.maxX = transformation.xOrigin + 100.0;
        bboxWorld.maxY = transformation.yOrigin;
        bboxWorld.minY = transformation.yOrigin - 100.0;

        RasterWindowType window = strategy.prepare(bandIndex, bboxWorld);
        
//test
        double testWorldX = bboxWorld.minX + 50.0;
        double testWorldY = bboxWorld.maxY - 50.0;
        double value = strategy.eval(window, testWorldX, testWorldY);
        std::cout << "Wert am Testpunkt: " << value << "\n";

//test end
        std::cout << "Fennster erfolgreich vorbereitet. " << "Breite: " << window.width << ", Höhe: " << window.height << "\n";

    }
    catch (const std::exception& e)
    {
        std::cout << "Fehler: " << e.what() << "\n";
        return 1;
    }

    return 0;
}