#pragma once

#include <vector>
#include <string>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct Feature
{
    std::string name;
    glm::vec2 location;
    float population;
};

class GeoData
{
public:
    GeoData();
    ~GeoData();
    void loadCSV(const std::string & filename, int nameColumn, int longitudeColumn, int latitudeColumn, int additionalColumn);
    std::vector<Feature> featuresInArea(glm::vec2 lowerLeft, glm::vec2 upperRight);
    std::pair<glm::vec2, glm::vec2> textureCoordsForArea(glm::vec2 lowerLeft, glm::vec2 upperRight);

private:
    std::vector<Feature> m_features;
};
