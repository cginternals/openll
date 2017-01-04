#pragma once

#include <vector>
#include <functional>

#include <openll/openll_api.h>

namespace gloperate_text
{

struct Label;
struct LabelArea;

namespace layout
{

using ScoringFunction = float (const LabelArea &, const LabelArea &);
ScoringFunction OPENLL_API overlapArea;
ScoringFunction OPENLL_API overlapCount;

void OPENLL_API constant(std::vector<Label> & labels);
void OPENLL_API random(std::vector<Label> & labels);
void OPENLL_API greedy(std::vector<Label> & labels, ScoringFunction scoringFunction);
void OPENLL_API discreteGradientDescent(std::vector<Label> & labels, ScoringFunction scoringFunction);

}

}
