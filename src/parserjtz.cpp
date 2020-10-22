#include "parserjtz.h"

#include <vector>
#include <string>

#include "parserjtxml.h"
#include "unzipper.h"

namespace luteconv
{

void ParserJtz::Parse(const Options& options, Piece& piece)
{
    std::vector<char> image;
    std::string zipFilename;
    Unzipper::Unzip(options.m_srcFilename, image, zipFilename);
    ParserJtxml parser;
    parser.Parse(zipFilename, image.data(), image.size(), options, piece);
}

} // namespace luteconv
