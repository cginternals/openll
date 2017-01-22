#pragma once

#include <vector>

#include <openll/layout/layoutbase.h>

int labelOverlaps(const std::vector<gloperate_text::Label> & labels);
float labelOverlapArea(const std::vector<gloperate_text::Label> & labels);
int labelsHidden(const std::vector<gloperate_text::Label> & labels);
std::vector<unsigned int> labelPositionDesirability(const std::vector<gloperate_text::Label> & labels);
