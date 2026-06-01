#pragma once
#include "cog_types.h"
#include "cog_config.h"
#include "cog_reader.h"
#include "sampler.h"
#include <stdexcept>


class COGEvalStrategy
{
    public:
        COGEvalStrategy(CogReader& reader, const CogConfig& config) : p_reader(reader), p_config(config) {}
        void prepareBox(const WorldBoundingBoxType& box)
        {
            p_currentWindow = p_reader.worldBoundsToPixelWindow(box);
            p_currentBlock = p_reader.readWindow(p_currentWindow, p_config.band);
            p_hasPreparedBlock = true;
        }
        double evalPoint(const WorldPointType& point) const
        {
            if (!p_hasPreparedBlock)
            {
                throw std::runtime_error("COGevalstrategy: no prepared block.");
            }
            PixelPointType pixel = p_reader.worldToPixel(point);
            return p_sampler.nearest(p_currentBlock, pixel);
        }

        double evalDummy(const WorldPointType& point) const
        {
            return 1.0;
        }
    private:
        CogReader& p_reader;
        CogConfig p_config;
        Sampler p_sampler;
        PixelWindowType p_currentWindow{};
        PreparedWindowType p_currentBlock{};
        bool p_hasPreparedBlock = false;
};