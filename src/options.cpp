#include "options.h"

#include <popl/include/popl.hpp>

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

void Options::PrintHelp(const std::string & allowed)
{
    std::cout << "luteconv " << m_version << std::endl
            << "Convert between lute tablature file formats." << std::endl
            << "Supported source formats: ft3, jtxml, jtz, mei, musicxml, mxl, tab, tc" << std::endl
            << "Supported desination formats: mei, musicxml, mxl, tab, tc" << std::endl
            << "Usage: luteconv [options ...] source-file [destination-file]" << std::endl
            << std::endl
            << allowed << std::endl
            << "The destination-file can be specified either using the --output option" << std::endl
            << "or as the 2nd positional parameter, this conforms with GNU options guidelines." << std::endl
            << std::endl
            << "tabtype = \"french\" | \"german\" | \"italian\" | \"spanish\"" << std::endl
            << "   The source tablature type is usually deduced from the source-file.  However," << std::endl
            << "   for tab files it is necessary to distinguish between italian and spanish" << std::endl
            << "   tablatures. The default destination tablature type is french." << std::endl
            << std::endl
            << "format = \"ft3\" | \"jtxml\" | \"jtz\" | \"mei\" | \"musicxml\" | \"mxl\" | \"tab\" | \"tc\"" << std::endl
            << "   if a file format is not specified then the filetype is used." << std::endl
            << std::endl
            << "tuning = Courses in scientific pitch notation, in increasing course number." << std::endl
            << "    Luteconv uses the tuning specifed by option --tuning, if given; otherwise" << std::endl
            << "    the tuning specified in the source file, if any; otherwise the" << std::endl
            << "    tuning is based on the number of courses used in the piece as follows:" << std::endl
            << "    = 8   \"G4 D4 A3 F3 C3 G2 F2 D2\"" << std::endl
            << "    <= 10 \"G4 D4 A3 F3 C3 G2 F2 Eb2 D2 C2\"" << std::endl
            << "    >= 11 \"F4 D4 A3 F3 D3 A2 G2 F2 E2 D2 C2 B1 A1\"" << std::endl
            << "    Option --7tuning, if given, will then modify the tuning of the" << std::endl
            << "    7th, 8th, ... courses." << std::endl
            << std::endl
            << "Where the source format allows more than one piece per file" << std::endl
            << "the --index option selects the desired piece, counting from 0.  Default 0." << std::endl
            << std::endl
            << "Report bugs to: paul@bayleaf.org.uk" << std::endl
            << "pkg home page: <https://bitbucket.org/bayleaf/luteconv/src/master/>" << std::endl
            << "General help using GNU software: <https://www.gnu.org/gethelp/>" << std::endl
            ;
}

void Options::ProcessArgs(int argc, char** argv)
{
    // Using popl library rather than POSIX getopt so can compile for non-POSIX platforms, e.g. Windows
    using namespace popl;
    
    OptionParser op("Allowed options");
    auto helpOption = op.add<Switch>("h", "help", "Show help");
    auto versionOption = op.add<Switch>("v", "version", "Show version");
    auto outputOption = op.add<Value<std::string>>("o", "output", "Set destination-file", "", &m_dstFilename);
    auto srcTabTypeOption = op.add<Value<std::string>>("S", "Srctabtype", "Set source tablature type");
    auto dstTabTypeOption = op.add<Value<std::string>>("D", "Dsttabtype", "Set destination tablature type", "french");
    auto srcFormatOption = op.add<Value<std::string>>("s", "srcformat", "Set source format");
    auto dstFormatOption = op.add<Value<std::string>>("d", "dstformat", "Set destination format");
    auto tuningOption = op.add<Value<std::string>>("t", "tuning", "Set tuning for all courses");
    auto sevenTuningOption = op.add<Value<std::string>>("7", "7tuning", "Set tuning from 7th course");
    auto indexOption = op.add<Value<std::string>>("i", "index", "Set section index", "0", &m_index);
    auto flagsOption = op.add<Value<int>>("f", "flags", "Add flags to destination rhythm", 0, &m_flags);
    auto verboseOption = op.add<Switch>("V", "Verbose", "Set verbose output");
    
    op.parse(argc, argv);
    
    if (!op.unknown_options().empty())
    {
        std::ostringstream ss;
        ss << "Error: unknown option " << op.unknown_options()[0];
        throw std::runtime_error(ss.str().c_str());
    }
    
    if (op.non_option_args().size() > 2)
    {
        throw std::runtime_error("Error: too many arguments");
    }
    
    if (helpOption->is_set())
    {
        std::ostringstream ss;
        ss << op;
        PrintHelp(ss.str());
        exit(0);
    }
        
    if (versionOption->is_set())
    {
        std::cout << "luteconv " << m_version << std::endl
        << "Copyright (C) 2020 Paul Overell" << std::endl
        << "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>" << std::endl
        << "This is free software: you are free to change and redistribute it." << std::endl
        << "There is NO WARRANTY, to the extent permitted by law." << std::endl
        ;
        exit(0);
    }
    
    if (srcTabTypeOption->is_set())
    {
        m_srcTabType = GetTabType(srcTabTypeOption->value());
    }    

    if (dstTabTypeOption->is_set())
    {
        m_dstTabType = GetTabType(dstTabTypeOption->value());
    }    

    if (srcFormatOption->is_set())
    {
        m_srcFormat = GetFormat(srcFormatOption->value());
    }
    
    if (dstFormatOption->is_set())
    {
        m_dstFormat = GetFormat(dstFormatOption->value());
    }

    if (tuningOption->is_set())
    {
        Pitch pitch;
        pitch.SetTuning(tuningOption->value().c_str(), m_tuning);
    }

    if (sevenTuningOption->is_set())
    {
        Pitch pitch;
        pitch.SetTuning(sevenTuningOption->value().c_str(), m_7tuning);
    }

    if (verboseOption->is_set())
    {
        Logger::SetVerbose(true);
    }
    
    // source filename
    if (op.non_option_args().size() >= 1)
    {
        m_srcFilename = op.non_option_args()[0];
    }
    else
    {
        throw std::runtime_error(std::string("Error: source filename missing"));
    }
    
    if (op.non_option_args().size() >= 2)
    {
        m_dstFilename = op.non_option_args()[1];
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
    else if (tabType == "german")
        return TabGerman;
    // not supported (yet!)
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
    else if (format == "mei")
        return FormatMei;
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
