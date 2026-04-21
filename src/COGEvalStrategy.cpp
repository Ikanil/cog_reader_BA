// oberste logikschicht, nimmt eine Welt Bounding Box, benutzt den Reader und gibt ein fertiges Datenpaket mit allem nötigen zurück

#include "COGEvalStrategy.h"
#include <stdexcept>
#include <cmath>

COGEvalStrategy::COGEvalStrategy(CogReader& reader): m_reader(reader) {}
RasterWindowType COGEvalStrategy::prepare(int bandIndex, const BoundingBoxWorldType& bboxWorld) const
{
    if (!m_reader.isValidBand(bandIndex))
    {
        throw std::runtime_error("Ungültiges Band");
    }
    BoundingBoxPixelsType pixelWindow = m_reader.worldToPixelBoundingBox(bboxWorld);

    if (!m_reader.isValidBoundingBox(pixelWindow))
    {
        throw std::runtime_error("Ungültiges Pixelfenster");
    }

    BoundingBoxResultType result = m_reader.readBoundingBox(bandIndex, pixelWindow);
    RasterWindowType window;
    window.bandIndex = bandIndex;
    window.worldWindow = bboxWorld;
    window.pixelWindow = pixelWindow;
    window.width = result.width;
    window.height = result.height;
    window.values = result.values;
    return window;
}


double COGEvalStrategy::eval(const RasterWindowType& window, double worldX, double worldY) const
{
    PixelPositionType globalPixel = m_reader.worldToPixelPosition(worldX, worldY);
    double localPixelX = globalPixel.x - window.pixelWindow.xOff;
    double localPixelY = globalPixel.y - window.pixelWindow.yOff;

    int pixelX = static_cast<int>(std::floor(localPixelX));
    int pixelY = static_cast<int>(std::floor(localPixelY));
    if(pixelX < 0 || pixelX >= window.width || pixelY < 0 || pixelY >= window.height)
    {
        throw std::runtime_error("Punkt liegt außerhalb des vorbereiteten Fensters.");
    }

    int index = pixelY * window.width + pixelX;
    if (index < 0 || index >= static_cast<int>(window.values.size()))
    {
        throw std::runtime_error("Ungültiger Index im vorbereiteten Fenster.");
    }
    return window.values[index];

}