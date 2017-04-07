
#pragma once


namespace gloperate_text
{


enum class SuperSampling : unsigned int
{
    None     = 0u,
    Grid1x3  = 1u,
    Grid2x4  = 2u,
    RGSS2x2  = 3u,
    Quincunx = 4u,
    Rooks8   = 5u,
    Grid3x3  = 6u,
    Grid4x4  = 7u
};


} // namespace gloperate_text
