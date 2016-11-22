
#pragma once

#include <string>
#include <iosfwd>
#include <map>

#include <openll/openll_api.h>


namespace gloperate_text
{


class FontFace;



class OPENLL_API FontLoader
{
public:
    FontLoader();

    FontFace * load(const std::string & filename) const;

protected:

    void handleInfo    (std::stringstream & stream, FontFace & fontFace, float & fontSize) const;
    void handleCommon  (std::stringstream & stream, FontFace & fontFace, float fontSize) const;
    void handlePage    (std::stringstream & stream, FontFace & fontFace
        , const std::string & filename) const;
    void handleChar    (std::stringstream & stream, FontFace & fontFace) const;
    void handleKerning (std::stringstream & stream, FontFace & fontFace) const;

    using StringPairs = std::map<std::string, std::string>;
    static StringPairs readKeyValuePairs(
        std::stringstream & stream
    ,   const std::initializer_list<const char *> & mandatoryKeys);

};


} // namespace openll
