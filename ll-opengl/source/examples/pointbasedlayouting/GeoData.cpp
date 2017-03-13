#include "GeoData.h"

#include <algorithm>
#include <fstream>

#include <glm/vector_relational.hpp>

#include <iostream>

namespace
{

std::vector<std::string> splitAll(const std::string& string,
                                  const std::string& at)
{
    std::vector<std::string> results;
    size_t lastPosition = 0;
    auto position = string.find(at);
    while (position != std::string::npos)
    {
        results.push_back(string.substr(lastPosition, position - lastPosition));
        position += at.size();
        lastPosition = position;
        position = string.find(at, position);
    }
    results.push_back(string.substr(lastPosition));
    return results;
}

}

GeoData::GeoData()
{
}

GeoData::~GeoData()
{
}

void GeoData::loadCSV(const std::string & filename, int nameColumn, int latitudeColumn, int longitudeColumn, int additionalColumn)
{
    std::ifstream file {filename};
    assert(file.is_open());
    while (!file.eof())
    {
        std::string line;
        std::getline(file, line);
        auto data = splitAll(line, ",");
        if (data.size() != 9) continue;
        auto name = data[nameColumn];
        glm::vec2 location {std::stof(data[latitudeColumn]), std::stof(data[longitudeColumn])};
        float additionalData = std::stof(data[additionalColumn]);
        m_features.push_back({name, location, additionalData});
    }
    std::sort(m_features.begin(), m_features.end(), [](Feature a, Feature b) {return a.additional > b.additional; });
}

std::vector<Feature> GeoData::featuresInArea(glm::vec2 lowerLeft, glm::vec2 upperRight)
{
    std::vector<Feature> result;
    auto inside = [&](Feature f) {
        return glm::all(glm::lessThan(lowerLeft, f.location)) && glm::all(glm::lessThan(f.location, upperRight));
    };
    auto size = upperRight - lowerLeft;
    std::copy_if(m_features.begin(), m_features.end(), std::back_inserter(result), inside);
    for (auto & feature : result)
    {
        feature.location -= lowerLeft;
        feature.location /= size;
        feature.location *= 2.f;
        feature.location -= 1.f;
    }
    return result;
}

std::pair<glm::vec2, glm::vec2> GeoData::textureCoordsForArea(glm::vec2 lowerLeft, glm::vec2 upperRight)
{
    auto convert = [](glm::vec2 coords) { return coords / glm::vec2(360.f, 180.f) + glm::vec2(.5f); };
    return {convert(lowerLeft), convert(upperRight)};
}
