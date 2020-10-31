#include "converter.h"

#include "parserft3.h"
#include "parserjtxml.h"
#include "parserjtz.h"
#include "parsermei.h"
#include "parsermusicxml.h"
#include "parsermxl.h"
#include "parsertab.h"
#include "parsertabcode.h"
#include "genmei.h"
#include "genmusicxml.h"
#include "genmxl.h"
#include "gentab.h"
#include "gentabcode.h"
#include "piece.h"

#include <stdexcept>

namespace luteconv
{

void Converter::Convert(const Options& options)
{
    Piece piece;
    
    switch (options.m_srcFormat)
    {
    case FormatUnknown:
    {
        throw std::runtime_error(std::string("Error: Unknown source file format: ") + options.m_srcFilename);
    }
    case FormatFt3:
    {
        ParserFt3 parser;
        parser.Parse(options, piece);
        break;
    }
    case FormatJtxml:
    {
        ParserJtxml parser;
        parser.Parse(options, piece);
        break;
    }
    case FormatJtz:
    {
        ParserJtz parser;
        parser.Parse(options, piece);
        break;
    }
    case FormatMei:
    {
        ParserMei parser;
        parser.Parse(options, piece);
        break;
    }
    case FormatMusicxml:
    {
        ParserMusicXml parser;
        parser.Parse(options, piece);
        break;
    }
    case FormatMxl:
    {
        ParserMxl parser;
        parser.Parse(options, piece);
        break;
    }
    case FormatTab:
    {
        ParserTab parser;
        parser.Parse(options, piece);
        break;
    }
    case FormatTabCode:
    {
        ParserTabCode parser;
        parser.Parse(options, piece);
        break;
    }
    default:
    {
        throw std::runtime_error(std::string("Error: source file format not supported: ") + options.m_srcFilename);
    }
    }
    
    switch (options.m_dstFormat)
    {
    case FormatUnknown:
    {
        throw std::runtime_error(std::string("Error: Unknown destination file format: ") + options.m_dstFilename);
    }
    case FormatMei:
    {
        GenMei generator;
        generator.Generate(options, piece);
        break;
    }
    case FormatMusicxml:
    {
        GenMusicXml generator;
        generator.Generate(options, piece);
        break;
    }
    case FormatMxl:
    {
        GenMxl generator;
        generator.Generate(options, piece);
        break;
    }
    case FormatTab:
    {
        GenTab generator;
        generator.Generate(options, piece);
        break;
    }
    case FormatTabCode:
    {
        GenTabCode generator;
        generator.Generate(options, piece);
        break;
    }
    default:
    {
        throw std::runtime_error(std::string("Error: destination file format not supported: ") + options.m_dstFilename);
    }
    }
}

} // namespace luteconv
