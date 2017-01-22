#pragma once

#include <vector>
#include <functional>

#include <openll/openll_api.h>

#include <openll/layout/RelativeLabelPosition.h>

namespace gloperate_text
{

struct Label;
struct LabelArea;

namespace layout
{

using PenaltyFunction = float (int overlapCount, float overlapArea, RelativeLabelPosition position, bool displayed);
PenaltyFunction OPENLL_API overlapArea;
PenaltyFunction OPENLL_API overlapCount;

void OPENLL_API constant(std::vector<Label> & labels);
void OPENLL_API random(std::vector<Label> & labels);

// penaltyFunction should be chosen so that a lower value is better
void OPENLL_API greedy(std::vector<Label> & labels, PenaltyFunction penaltyFunction);
void OPENLL_API discreteGradientDescent(std::vector<Label> & labels, PenaltyFunction penaltyFunction);
void OPENLL_API simulatedAnnealing(std::vector<Label> & labels, PenaltyFunction penaltyFunction);

}

}
