#include "parsermxl.h"

#include <vector>
#include <string>

#include "parsermusicxml.h"
#include "unzipper.h"

namespace luteconv
{

void ParserMxl::Parse(const Options& options, Piece& piece)
{
    std::vector<char> image;
    std::string zipFilename;
    Unzipper::Unzip(options.m_srcFilename, image, zipFilename);
    ParserMusicXml parser;
    parser.Parse(zipFilename, image.data(), image.size(), options, piece);
}

} // namespace luteconv
