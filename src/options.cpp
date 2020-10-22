#include "options.h"
#include <getopt.h>
#include <iostream>

#include "logger.h"

namespace luteconv
{

#define XSTRINGIFY(s) STRINGIFY(s)
#define STRINGIFY(s) #s

Options::Options()
: m_version{XSTRINGIFY(VERSION)}
{
}

#undef STRINGIFY
#undef XSTRINGIFY

void Options::PrintHelp()
{
    std::cout << "luteconv " << m_version << std::endl
            << "Convert between lute tablature file formats." << std::endl
            << "Supported source formats: ft3, jtxml, jtz, musicxml, mxl, tab, tc" << std::endl
            << "Supported desination formats: musicxml, mxl, tab, tc" << std::endl
            << "Usage: luteconv [options ...] source-file [destination-file]" << std::endl
            << std::endl
            << "options:" << std::endl
            << "--help                      Show help" << std::endl
            << "--version                   Show version" << std::endl
            << "--output <destination-file> Set destination-file" << std::endl
            << "--Srctabtype <tabtype>      Set source tablature type" << std::endl
            << "--Dsttabtype <tabtype>      Set destination tablature type" << std::endl
            << "--srcformat <format>        Set source format" << std::endl
            << "--dstformat <format>        Set destination format" << std::endl
            << "--tuning <tuning>           Set tuning for all courses" << std::endl
            << "--7tuning <tuning>          Set tuning from 7th course" << std::endl
            << "--index                     Set section index" << std::endl
            << "--Verbose                   Set verbose output" << std::endl
            << std::endl
            << "The destination-file can be specified either using the --output option" << std::endl
            << "or as the 2nd positional parameter, this conforms with GNU options guidelines." << std::endl
            << std::endl
            << "tabtype = \"french\" | \"italian\" | \"spanish\"" << std::endl
            << "   The source tablature type is usually deduced from the source-file.  However," << std::endl
            << "   for tab files it is necessary to distinguish between italian and spanish" << std::endl
            << "   tablatures. The default destination tablature type is french." << std::endl
            << std::endl
            << "format = \"ft3\" | \"jtxml\" | \"jtz\" | \"musicxml\" | \"mxl\" | \"tab\" | \"tc\"" << std::endl
            << "   if a file format is not specified then the filetype is used." << std::endl
            << std::endl
            << "tuning = Courses in scientific pitch notation, in increasing course number." << std::endl
            << "   Use --tuning if specified" << std::endl
            << "   else use tuning specified in the source file, if any" << std::endl
            << "   else based on number of courses used:" << std::endl
            << "   = 8   \"G4 D4 A3 F3 C3 G2 F2 D2\"" << std::endl
            << "   <= 10 \"G4 D4 A3 F3 C3 G2 F2 Eb2 D2 C2\"" << std::endl
            << "   >= 11 \"F4 D4 A3 F3 D3 A2 G2 F2 E2 D2 C2 B1 A1\"" << std::endl
            << "   --7tuning will then modify the above" << std::endl
            << std::endl
            << "Where the source format allows more that one piece per file" << std::endl
            << "the --index option selects the desired piece, counting from 0.  Default 0." << std::endl
            << std::endl
            << "Report bugs to: paul@bayleaf.org.uk" << std::endl
            << "pkg home page: <https://bitbucket.org/bayleaf/luteconv/src/master/>" << std::endl
            << "General help using GNU software: <https://www.gnu.org/gethelp/>" << std::endl
            ;
}

void Options::ProcessArgs(int argc, char** argv)
{
    const char* const short_opts = "7:d:D:hi:o:S:s:t:vV";
    const option long_opts[] = {
            {"7tuning", required_argument, nullptr, '7'},
            {"dstformat", required_argument, nullptr, 'd'},
            {"Dsttabtype", required_argument, nullptr, 'D'},
            {"help", no_argument, nullptr, 'h'},
            {"index", required_argument, nullptr, 'i'},
            {"output", required_argument, nullptr, 'o'},
            {"Srctabtype", required_argument, nullptr, 'T'},
            {"srcformat", required_argument, nullptr, 's'},
            {"tuning", required_argument, nullptr, 't'},
            {"version", no_argument, nullptr, 'v'},
            {nullptr, no_argument, nullptr, 0}
    };

    while (true)
    {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (-1 == opt)
            break;

        switch (opt)
        {
        case '7':
        {
            Pitch pitch;
            pitch.SetTuning(optarg, m_7tuning);
            break;
        }
        
        case 'D':
            m_dstTabType = GetTabType(optarg);
            break;
            
        case 'd':
            m_dstFormat = GetFormat(optarg);
            break;
            
        case 'h': // -h or –help
            PrintHelp();
            exit(0);
            
        case 'i':
            m_index = optarg;
            break;
            
        case 'o':
            m_dstFilename = optarg;
            break;

        case 'S':
            m_srcTabType = GetTabType(optarg);
            break;
            
        case 's':
            m_srcFormat = GetFormat(optarg);
            break;

        case 't':
        {
            Pitch pitch;
            pitch.SetTuning(optarg, m_tuning);
            break;
        }

        case 'v':
            std::cout << "luteconv " << m_version << std::endl
            << "Copyright (C) 2020 Paul Overell" << std::endl
            << "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>" << std::endl
            << "This is free software: you are free to change and redistribute it." << std::endl
            << "There is NO WARRANTY, to the extent permitted by law." << std::endl
            ;
            exit(0);
            break;

        case 'V':
            Logger::SetVerbose(true);
            break;
            
        case '?': // Unrecognized option
        default:
            exit(1);
        }
    }
    
    // source filename
    if (optind < argc)
    {
        m_srcFilename = argv[optind++];
    }
    else
    {
        throw std::runtime_error(std::string("Error: source filename missing"));
    }
    
    if (optind < argc)
    {
        m_dstFilename = argv[optind];
    }
    
    if (m_dstFilename.empty())
        throw std::runtime_error(std::string("Error: destination filename missing"));

    if (m_dstTabType == TabUnknown)
        throw std::runtime_error(std::string("Error: unknown destination tablature type"));
    
    // if file format is not specified use filetype
    SetFormatFilename();
}

void Options::SetFormatFilename()
{
    if (m_srcFormat == FormatUnknown)
        m_srcFormat = GetFormatFilename(m_srcFilename);

    if (m_dstFormat == FormatUnknown)
        m_dstFormat = GetFormatFilename(m_dstFilename);
}

TabType Options::GetTabType(const std::string& tabType)
{
    if (tabType == "french")
        return TabFrench;
    else if (tabType == "italian")
        return TabItalian;
    // not supported (yet!)
//  else if (tabType == "german")
//      return TabGerman;
//  else if (tabType == "neopolitan")
//      return TabNeopolitan;
    else if (tabType == "spanish")
        return TabSpanish;
    
    return TabUnknown;
}

Format Options::GetFormatFilename(const std::string& filename)
{
    
    const size_t found = filename.find_last_of(".");
    if (found != std::string::npos)
        return GetFormat(filename.substr(found + 1));

    return FormatUnknown;
}

Format Options::GetFormat(const std::string& format)
{
    if (format == "ft3")
        return FormatFt3;
    else if (format == "jtxml")
        return FormatJtxml;
    else if (format == "jtz")
        return FormatJtz;
    else if (format == "musicxml")
        return FormatMusicxml;
    else if (format == "mxl")
        return FormatMxl;
    else if (format == "tab")
        return FormatTab;
    else if (format == "tc")
        return FormatTabCode;
    else
        return FormatUnknown;
}

} // namespace luteconv
