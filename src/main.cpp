#include <iostream>
#include <exception>
#include <string>

#include "cog_reader.h"
#include "sampler.h"
#include "cog_eval_strategy.h"

int main()
{
    try
    {
        // ------------------------------------------------------------
        // 1. Remote-COG-URL definieren
        // ------------------------------------------------------------
        // /vsicurl/ ist ein GDAL-Präfix.
        // Damit kann GDAL eine Datei direkt per HTTP/HTTPS lesen,
        // ohne sie vorher komplett herunterzuladen.
        const std::string url =
            "/vsicurl/https://sentinel-cogs.s3.us-west-2.amazonaws.com/"
            "sentinel-s2-l2a-cogs/36/Q/WD/2020/7/"
            "S2A_36QWD_20200701_0_L2A/B08.tif";


        // ------------------------------------------------------------
        // 2. CogReader erzeugen und Remote-COG öffnen
        // ------------------------------------------------------------
        // Der CogReader kapselt den GDAL-Zugriff.
        // Er öffnet das COG, liest RasterInfo und GeoTransform.
        CogReader reader;
        reader.open(url);


        // ------------------------------------------------------------
        // 3. RasterInfo ausgeben
        // ------------------------------------------------------------
        // RasterInfo enthält grundlegende Rasterdaten:
        // - Breite in Pixeln
        // - Höhe in Pixeln
        // - Anzahl der Bänder
        RasterInfoType info = reader.rasterInfo();

        std::cout << "Raster width: " << info.width << std::endl;
        std::cout << "Raster height: " << info.height << std::endl;
        std::cout << "Band count: " << info.bandCount << std::endl;


        // ------------------------------------------------------------
        // 4. GeoTransform ausgeben
        // ------------------------------------------------------------
        // Der GeoTransform beschreibt die Beziehung zwischen
        // Pixelkoordinaten und Weltkoordinaten.
        //
        // Für dieses COG ist typisch:
        // pixelWidth  = 10
        // pixelHeight = -10
        //
        // Ein negatives pixelHeight ist normal,
        // weil Rasterzeilen nach unten laufen.
        GeoTransformType gt = reader.geoTransform();

        std::cout << "originX: " << gt.originX << std::endl;
        std::cout << "originY: " << gt.originY << std::endl;
        std::cout << "pixelWidth: " << gt.pixelWidth << std::endl;
        std::cout << "pixelHeight: " << gt.pixelHeight << std::endl;


        // ------------------------------------------------------------
        // 5. Test-Bounding-Box in Weltkoordinaten bauen
        // ------------------------------------------------------------
        // Diese Box beschreibt ungefähr den Bereich der ersten 10x10 Pixel.
        //
        // Später entspricht das der Element-Bounding-Box aus UG4.
        //
        // Wichtig:
        // Weil pixelHeight negativ ist, liegt minY unterhalb von originY.
        WorldBoundingBoxType box;

        box.minX = gt.originX;
        box.maxX = gt.originX + 10.0 * gt.pixelWidth;

        box.maxY = gt.originY;
        box.minY = gt.originY + 10.0 * gt.pixelHeight;


        // ------------------------------------------------------------
        // 6. prepare-Idee testen:
        //    Welt-Bounding-Box -> PixelWindow
        // ------------------------------------------------------------
        // GDAL liest nicht mit Weltkoordinaten.
        // Deshalb wird die Welt-Bounding-Box in ein PixelWindow übersetzt.
        PixelWindowType window = reader.worldBoundsToPixelWindow(box);

        std::cout << "Window xOffset: " << window.xOffset << std::endl;
        std::cout << "Window yOffset: " << window.yOffset << std::endl;
        std::cout << "Window width: " << window.width << std::endl;
        std::cout << "Window height: " << window.height << std::endl;


        // ------------------------------------------------------------
        // 7. prepare-Idee testen:
        //    PixelWindow -> PreparedWindowType
        // ------------------------------------------------------------
        // Jetzt wird das berechnete Fenster tatsächlich aus dem COG gelesen.
        //
        // PreparedWindowType enthält:
        // - das gelesene Fenster
        // - Breite und Höhe
        // - die Pixelwerte im values-Vektor
        PreparedWindowType block = reader.readWindow(window, 1);


        // ------------------------------------------------------------
        // 8. Test-Weltpunkt bauen
        // ------------------------------------------------------------
        // Dieser Weltpunkt entspricht ungefähr Pixel (5,5).
        //
        // Später kommt so ein Punkt von UG4 als Auswertungspunkt
        // innerhalb eines Elements.
        WorldPointType worldPoint;

        worldPoint.x = gt.originX + 5.0 * gt.pixelWidth;
        worldPoint.y = gt.originY + 5.0 * gt.pixelHeight;

        std::cout << "World point x: " << worldPoint.x << std::endl;
        std::cout << "World point y: " << worldPoint.y << std::endl;


        // ------------------------------------------------------------
        // 9. eval-Idee testen:
        //    Weltpunkt -> Pixelpunkt
        // ------------------------------------------------------------
        // Der Reader rechnet den Weltpunkt mithilfe des GeoTransforms
        // in eine Pixelposition im Gesamtraster um.
        PixelPointType pixel = reader.worldToPixel(worldPoint);

        std::cout << "Converted pixel col: " << pixel.col << std::endl;
        std::cout << "Converted pixel row: " << pixel.row << std::endl;


        // ------------------------------------------------------------
        // 10. eval-Idee testen:
        //     Pixelpunkt -> Rasterwert
        // ------------------------------------------------------------
        // Der Sampler nimmt den vorbereiteten Rasterblock und den
        // globalen Pixelpunkt.
        //
        // nearest() rechnet den globalen Pixelpunkt lokal in den Block um
        // und gibt den nächstgelegenen Pixelwert zurück.
        Sampler sampler;

        double directValue = sampler.nearest(block, pixel);

        std::cout << "Sample value at world point: "
                  << directValue << std::endl;


        // ------------------------------------------------------------
        // 11. COGEvalStrategy vorbereiten
        // ------------------------------------------------------------
        // Die Strategy bündelt später die UG4-Logik:
        //
        // prepare:
        //     Bounding Box -> PixelWindow -> readWindow
        //
        // eval:
        //     WorldPoint -> PixelPoint -> Sampler
        CogConfig config;
        config.url = url;
        config.band = 1;

        COGEvalStrategy strategy(reader, config);


        // ------------------------------------------------------------
        // 12. Dummy-Strategy-Test
        // ------------------------------------------------------------
        // Dieser Test zeigt nur:
        // Die Strategy kann erzeugt werden und evalDummy() ist aufrufbar.
        double dummyValue = strategy.evalDummy(worldPoint);

        std::cout << "Dummy strategy value: "
                  << dummyValue << std::endl;


        // ------------------------------------------------------------
        // 13. Echte prepare/eval-Strategy testen
        // ------------------------------------------------------------
        // prepareBox(box):
        //     liest den passenden Rasterausschnitt und speichert ihn intern.
        //
        // evalPoint(worldPoint):
        //     wertet aus diesem vorbereiteten Ausschnitt einen Punkt aus.
        strategy.prepareBox(box);

        double strategyValue = strategy.evalPoint(worldPoint);

        std::cout << "Strategy sampled value: "
                  << strategyValue << std::endl;


        // ------------------------------------------------------------
        // 14. Reader sauber schließen
        // ------------------------------------------------------------
        reader.close();
    }
    catch (const std::exception& e)
    {
        // ------------------------------------------------------------
        // Fehlerbehandlung
        // ------------------------------------------------------------
        // Falls GDAL nicht öffnen kann, RasterIO fehlschlägt,
        // ein Fenster ungültig ist oder ein Punkt außerhalb liegt,
        // landet die Fehlermeldung hier.
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}