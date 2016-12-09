
#include <openll/FontLoader.h>

#include <fstream>
#include <sstream>
#include <string>
#include <functional>
#include <set>
#include <map>
#include <algorithm>

#include <glbinding/gl/enum.h>

#include <openll/RawFile.h>

#include <openll/FontFace.h>

namespace {

std::string directoryPath(std::string path)
{
    auto pos = path.find_last_of('/');

    if (pos == std::string::npos)
        return std::string();

    return path.substr(0, pos+1); // Add trailing slash
}

template <typename Type>
Type fromString(const std::string & string)
{
    std::stringstream stream(string);
    auto value = typename std::remove_const<Type>::type();
    stream >> value;
    return value;
}

std::vector<std::string> split(const std::string & input, char delimiter)
{
    std::vector<std::string> result;
    result.push_back(std::string());
    for (const char & c : input)
    {
        if (c == delimiter)
        {
            result.push_back(std::string());
            continue;
        }

        result.back().push_back(c);
    }
    return result;
}

std::string stripped(const std::string & string, const std::set<char> & blacklist)
{
    auto result = string;
    result.erase(std::remove_if(result.begin(), result.end(), [&blacklist](char x) {
        return blacklist.count(x) > 0; }), result.end());

    return result;
}

bool hasSuffix(const std::string & string, const std::string & suffix)
{
    return string.size() >= suffix.size() && string.compare(string.size() - suffix.size(), suffix.size(), suffix) == 0;
}

}


namespace gloperate_text
{


FontLoader::FontLoader()
{
}

FontFace * FontLoader::load(const std::string & filename) const
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);

    if (!in)
        return nullptr;

    auto fontFace = new FontFace();


    auto line = std::string();
    auto identifier = std::string();
    auto fontSize = 0.f;

    while (std::getline(in, line))
    {
        std::stringstream ss;
        ss << line;

        if (!std::getline(ss, identifier, ' '))
        {
            assert(false);
            continue;
        }

        if      (identifier == "info")
        {
            handleInfo(ss, *fontFace, fontSize);
        }
        else if (identifier == "common")
        {
            handleCommon(ss, *fontFace, fontSize);
        }
        else if (identifier == "page")
        {
            handlePage(ss, *fontFace, filename);
        }
        else if (identifier == "char")
        {
            handleChar(ss, *fontFace);
        }
        else if (identifier == "kerning")
        {
            handleKerning(ss, *fontFace);
        }
    }

    if (fontFace->glyphTexture())
        return fontFace;

    delete fontFace;
    return nullptr;
}

void FontLoader::handleInfo(std::stringstream & stream, FontFace & fontFace, float & fontSize) const
{
    auto pairs = readKeyValuePairs(stream, { "size", "padding" });

    fontSize = fromString<float>(pairs.at("size"));

    auto values = split(pairs.at("padding"), ',');
    assert(values.size() == 4);

    auto padding = glm::vec4();
    padding[0] = fromString<float>(values[2]); // top
    padding[1] = fromString<float>(values[1]); // right
    padding[2] = fromString<float>(values[3]); // bottom
    padding[3] = fromString<float>(values[0]); // left

    fontFace.setGlyphTexturePadding(padding);
}

void FontLoader::handleCommon(std::stringstream & stream, FontFace & fontFace, const float fontSize) const
{
    auto pairs = readKeyValuePairs(stream, { "lineHeight", "base", "scaleW", "scaleH" });

    fontFace.setAscent(fromString<float>(pairs.at("base")));
    fontFace.setDescent(fontFace.ascent() - fontSize);

    assert(fontFace.size() > 0.f);
    fontFace.setLineHeight(fromString<float>(pairs.at("lineHeight")));

    fontFace.setGlyphTextureExtent({
        fromString<float>(pairs.at("scaleW")),
        fromString<float>(pairs.at("scaleH")) });
}

void FontLoader::handlePage(std::stringstream & stream, FontFace & fontFace, const std::string & filename) const
{
    auto pairs = readKeyValuePairs(stream, { "file" });

    const auto path = directoryPath(filename);
    const auto file = stripped(pairs.at("file"), { '"', '\r' });

    assert(hasSuffix(file, ".raw"));

    auto texture = new globjects::Texture(gl::GL_TEXTURE_2D);
    auto raw = gloperate_text::RawFile(path + "/" + file);

    if (!raw.isValid())
    {
        assert(false);
        return;
    }

    const auto extent = glm::ivec2(fontFace.glyphTextureExtent());
    texture->image2D(0, gl::GL_R8, extent, 0
        , gl::GL_RED, gl::GL_UNSIGNED_BYTE, static_cast<const gl::GLvoid *>(raw.data()));

    fontFace.setGlyphTexture(texture);

    fontFace.glyphTexture()->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
    fontFace.glyphTexture()->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
    fontFace.glyphTexture()->setParameter(gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
    fontFace.glyphTexture()->setParameter(gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
}

void FontLoader::handleChar(std::stringstream & stream, FontFace & fontFace) const
{
    auto pairs = readKeyValuePairs(stream, { "id", "x", "y", "width", "height", "xoffset", "yoffset", "xadvance" });

    auto index = fromString<GlyphIndex>(pairs.at("id"));
    assert(index > 0);

    auto glyph = Glyph();

    glyph.setIndex(index);

    const auto extentScale = 1.f / glm::vec2(fontFace.glyphTextureExtent());
    const auto extent = glm::vec2(
        fromString<float>(pairs.at("width")),
        fromString<float>(pairs.at("height")));

    glyph.setSubTextureOrigin({
        fromString<float>(pairs.at("x")) * extentScale.x,
        1.f - (fromString<float>(pairs.at("y")) + extent.y) * extentScale.y});

    glyph.setExtent(extent);
    glyph.setSubTextureExtent(extent * extentScale);

    glyph.setBearing(fontFace.ascent(),
        fromString<float>(pairs.at("xoffset")),
        fromString<float>(pairs.at("yoffset")));

    glyph.setAdvance(fromString<float>(pairs.at("xadvance")));

    fontFace.addGlyph(glyph);
}

void FontLoader::handleKerning(std::stringstream & stream, FontFace & fontFace) const
{
    auto pairs = readKeyValuePairs(stream, { "first", "second", "amount" });

    auto first = fromString<GlyphIndex>(pairs.at("first"));
    assert(first > 0);

    auto second = fromString<GlyphIndex>(pairs.at("second"));
    assert(second > 0);

    auto kerning = fromString<float>(pairs.at("amount"));

    fontFace.setKerning(first, second, kerning);
}

FontLoader::StringPairs FontLoader::readKeyValuePairs(
    std::stringstream & stream
    , const std::initializer_list<const char *> & mandatoryKeys)
{
    auto key = std::string();
    auto value = std::string();

    auto pairs = StringPairs();

    while (stream)
    {
        if (!std::getline(stream, key, '='))
            continue;

        if (!std::getline(stream, value, ' '))
            continue;

        pairs.insert(std::pair<std::string, std::string>(key, value));
    }

    // check if all required keys are provided
    auto valid = true;
    for (const auto mandatoryKey : mandatoryKeys)
    {
        valid |= pairs.find(mandatoryKey) != pairs.cend();
    }

    if (!valid)
        return StringPairs();

    return pairs;
}


} // namespace gloperate_text
