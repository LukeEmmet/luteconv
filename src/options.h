#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include "pitch.h"

#include <string>
#include <sstream>

namespace luteconv
{

enum Format
{
    FormatUnknown,
    FormatFt3,          // Fronimo
    FormatJtxml,        // Fandando
    FormatJtz,          // Fandango zip
    FormatMei,          // Music Encoding Initiative (Tablature Interest Group)
    FormatMusicxml,     // MusicXML
    FormatMxl,          // MusicXML zip
    FormatTab,          // Tab
    FormatTabCode,      // TabCode
};

enum TabType
{
    TabUnknown,
    TabFrench,          // letters for frets, staff line 6 = course 1
    TabItalian,         // numbers for frets, staff line 1 = course 1
    TabGerman,          // no staff, different symbol for each course/fret
    TabNeopolitan,      // As spanish except open string is 1
    TabSpanish,         // numbers for frets, staff line 6 = course 1.  Aka milan, modern guitar tab
};

class Options
{
public:
    
    /**
     * Constructor
     */
    Options();
    
    /**
     * Destructor
     */
    ~Options() = default;
    
    /**
     * Process options
     */
    void ProcessArgs(int argc, char** argv);
    
    /**
     * If not set, set the format from the filenames
     * 
     */
    void SetFormatFilename();
    
    Format m_srcFormat{FormatUnknown};
    Format m_dstFormat{FormatUnknown};
    TabType m_srcTabType{TabUnknown};
    TabType m_dstTabType{TabFrench};
    std::vector<Pitch> m_tuning;
    std::vector<Pitch> m_7tuning;
    std::string m_srcFilename;
    std::string m_dstFilename;
    const std::string m_version;
    std::string m_index{"0"};
    int m_flags{0};
    int m_wrapThreshold{25};
    
private:
    void PrintHelp(const std::string & allowed);
    Format GetFormat(const std::string& format);
    Format GetFormatFilename(const std::string& filename);
    TabType GetTabType(const std::string& tabType);
};

} // namespace luteconv

#endif // _OPTIONS_H_
