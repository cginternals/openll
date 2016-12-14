#pragma once

#include <vector>

#include <openll/openll_api.h>

namespace gloperate_text
{

class FontFace;

struct Label;
struct LabelPlacement;

void OPENLL_API constantLayout(std::vector<Label> & labels);
void OPENLL_API randomLayout(std::vector<Label> & labels);
void OPENLL_API greedyLayout(std::vector<Label> & labels);

}
