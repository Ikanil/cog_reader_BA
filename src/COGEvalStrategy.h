#pragma once
#include "CogReader.h"
#include "RasterWindow.h"

class COGEvalStrategy
{
public:
    COGEvalStrategy(CogReader& reader);
    RasterWindowType prepare(int bandIndex, const BoundingBoxWorldType& bboxWorld) const;
    double eval(const RasterWindowType& window, double worldX, double worldY) const;
private:
    CogReader& m_reader;

};