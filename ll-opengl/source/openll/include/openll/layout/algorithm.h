#pragma once

#include <vector>

#include <openll/openll_api.h>

namespace gloperate_text
{

struct Label;

namespace layout
{

void OPENLL_API constant(std::vector<Label> & labels);
void OPENLL_API random(std::vector<Label> & labels);
void OPENLL_API greedy(std::vector<Label> & labels);
void OPENLL_API discreteGradientDescent(std::vector<Label> & labels);

}

}
