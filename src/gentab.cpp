#include "gentab.h"

#include <algorithm>
#include <fstream>
#include <chrono>
#include <iomanip>

#include <functional>

namespace luteconv
{

void GenTab::Generate(const Options& options, const Piece& piece)
{
    std::fstream dst;
    dst.open(options.m_dstFilename.c_str(), std::fstream::out | std::fstream::trunc);
    if (!dst.is_open())
        throw std::runtime_error(std::string("Error: Can't open ") + options.m_dstFilename);
    
    Generate(options, piece, dst);
}

void GenTab::Generate(const Options& options, const Piece& piece, std::ostream& dst)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct std::tm * ptm = std::gmtime(&tt);
    
    // header
    dst << "% Converted to .tab by luteconv " << options.m_version << std::endl
        << "% encoding-date " << std::put_time(ptm,"%F") << std::endl
        << "-C" << std::endl
        << "-highlightparen" << std::endl
        << "-tuning " << Pitch::GetTuningTab(piece.m_tuning) << std::endl;

    if (piece.m_copyrightEnabled)
    {
        dst << "-G" << std::endl;
    }
    
    switch (options.m_dstTabType)
    {
    case TabFrench:
    {
        // letter frets
        if (piece.m_tuning.size() >= 11)
        {
            dst << "-b" << std::endl; // baroque font
        }
        else
        {
            dst << "$flagstyle=thin" << std::endl
                << "$charstyle=robinson" << std::endl;
        }
        break;
    }
    case TabItalian:
    {
        // 7+ course italian tablature, extra space after flags for diapasons
        if (piece.m_tuning.size() > 6)
            dst << "-s" << std::endl;
        
        // numeric frets
        dst << "$numstyle=italian" << std::endl
            << "$line=o" << std::endl;
        break;
    }
    case TabSpanish:
    {
        dst << "-milan" << std::endl;
        
        // numeric frets
        dst << "$numstyle=italian" << std::endl
            << "$line=o" << std::endl;
        break;
    }
    default:
        break;
    }
    
    if (!piece.m_copyright.empty())
    {
        dst << "$scribe=" << piece.m_copyright << std::endl;
    }
    
    // header text
        
    if (!piece.m_title.empty() || !piece.m_composer.empty())
        dst << "{" << piece.m_title << "/" << piece.m_composer << "}" << std::endl;

    for (const auto & credit : piece.m_credits)
    {
        switch (credit.m_align)
        {
        case AlignLeft:
            dst << "{" << credit.m_left << "}" << std::endl;
            break;
        case AlignRight:
            dst << "{/" << credit.m_right << "}" << std::endl;
            break;
        case AlignCenter:
            dst << "{\\CL/" << credit.m_left << "}" << std::endl;
            break;
        case AlignLeftRight:
            dst << "{" << credit.m_left << "/" << credit.m_right << "}" << std::endl;
            break;
        default:
            break;
        }
    }

    dst << std::endl;
     
    // body
    int staveNum{1};
    int barNum{1};
    int chordCount{0};
    std::string repForward;
    
    dst << "% Stave " << staveNum << std::endl;
    
    for (const auto & bar : piece.m_bars)
    {
        if (chordCount == 0)
        {
            // first bar on line
            dst <<  "% Bar " << barNum << std::endl;
            if (!repForward.empty())
            {
                dst << repForward << std::endl;
                repForward.clear();
            }
            else
            {
                dst << "b" << std::endl;
            }
        }
        
        // time signature
        const std::string timeSignature = GetTimeSignature(bar);
        if (!timeSignature.empty())
            dst << timeSignature << std::endl;
        
        // chords
        for (const auto & chord : bar.m_chords)
        {
            ++chordCount;
            
            // flags
            std::string line{GetFlagInfo(bar.m_chords, chord)};
            
            // notes
            std::vector<std::string> vert;
            vert.reserve(20);
            
            int vertOffset = 0;

            for (const auto & note : chord.m_notes)
            {
                int vertIndex{0};
                if (options.m_dstTabType == TabItalian)
                {
                    if (note.m_string >= 7)
                    {
                        vertIndex = 0;
                    }
                    else
                    {
                        // upside down
                        vertIndex = 7 - std::min(note.m_string, 6) - 1 - vertOffset;
                    
                        if (piece.m_tuning.size() > 6)
                            ++vertIndex; // extra space after flags for 7+ course italian
                    }
                }
                else
                {
                    vertIndex = std::min(note.m_string, 7) - 1 - vertOffset;
                }
                
                while (vertIndex >= static_cast<int>(vert.size()))
                    vert.emplace_back(" "); // reserve unused strings
                
                const std::string rightFingering = GetRightFingering(note);
                const std::string leftOrnament = GetLeftOrnament(note);
                
                // Ornaments #*- on first course may clash with flags, put default - as 2nd character
                if (!leftOrnament.empty() && note.m_string == 1 && line.size() == 1)
                {
                    line += "-";
                }
                vert[vertIndex] =     leftOrnament // before the letter
                                    + GetLeftFingering(note) // before the letter
                                    + GetFret(note, options) // fret letter
                                    + GetRightOrnament(note) // after the letter
                                    + rightFingering;  // after the letter
                
                // right fingering pushes the vertical position out of place, compensate
                vertOffset += rightFingering.size();
            }
            
            for (const auto & s : vert)
            {
                line += s;
            }
            
            // remove trailing spaces
            line.erase(std::find_if(line.rbegin(), line.rend(), std::bind1st(std::not_equal_to<char>(), ' ')).base(), line.end());
            
            dst << line << std::endl;
        }
        
        // end of current bar
        ++barNum;
        dst <<  "% Bar " << barNum << std::endl;
        
        // TODO does Fronimo record stave endings?
        // Tab doesn't automatically add stave endings.  Use herustic:
        // count chords, when the threshold is reached end the stave at the end of
        // the current bar.  Except for last bar.
        const int threshold = 25; // seems to work OK
        const bool lineBreak = barNum < static_cast<int>(piece.m_bars.size()) && chordCount > threshold;
        
        std::string barStyle;
        switch (bar.m_barStyle)
        {
        case BarStyleHeavy:
            barStyle = "B";
            break;
        case BarStyleLightLight:
            barStyle = "bb";
            break;
        default:
            barStyle = "b";
        }
        
        switch (bar.m_repeat)
        {
        case RepNone:
            break;
        case RepForward:
            barStyle += ".";
            break;
        case RepBackward:
            barStyle = "." + barStyle;
            break;
        case RepJanus:
            if (lineBreak)
            {
                // backward repeat here, forward repeat in next bar, next line
                repForward = barStyle + ".";
                barStyle = "." + barStyle;
            }
            else
            {
                barStyle = "." + barStyle + ".";
            }
        }
        
        if (bar.m_fermata)
        {
            dst << "Y" << barStyle << std::endl;
        }
        else
        {
            dst << barStyle << std::endl;
        }
        
        if (lineBreak)
        {
            dst << std::endl;
            ++staveNum;
            chordCount = 0;
            dst << "% Stave " << staveNum << std::endl;
        }
    }
    
    dst << "e" << std::endl; // end of piece
}
    
std::string GenTab::GetTimeSignature(const Bar & bar)
{
    switch (bar.m_timeSymbol)
    {
    case TimeSyNone:
        return "";
    case TimeSyCommon:
        return "C";
    case TimeSyCut:
        return "c";
    case TimeSySingleNumber:
        return "S" + std::to_string(bar.m_beats);
    case TimeSyNote:
        return "";
    case TimeSyDottedNote:
        return "";
    case TimeSyNormal:
        return "S" + std::to_string(bar.m_beats) + "-" + std::to_string(bar.m_beatType);
    }
    return "";
}

std::string GenTab::GetFlagInfo(const std::vector<Chord> & chords, const Chord & our)
{
    std::string result;
    std::string ourFlag;
    
    const int ourIndex = std::distance(chords.data(), &our);
    const int following = ourIndex + 1;
    const int previous = ourIndex - 1;
    
    if (our.m_fermata)
        return "Y";
    
    // TODO TAB thinks 0 flags = crotchet
    if (our.m_noteType >= NoteTypeQuarter)
    {
        ourFlag = std::to_string(our.m_noteType - NoteTypeQuarter);
    }
    else if (our.m_noteType == NoteTypeHalf)
    {
        ourFlag = "w";
    }
    else if (our.m_noteType == NoteTypeWhole)
    {
        ourFlag = "W";
    }
    else if (our.m_noteType == NoteTypeBreve)
    {
        ourFlag = "B";
    }
    else if (our.m_noteType == NoteTypeLong)
    {
        ourFlag = "L";
    }
    
    // first flag in grid, with following
    if (following < static_cast<int>(chords.size()) && (our.m_grid == GridStart) && (chords[following].m_grid != GridStart))
    {
        result = "#" + ourFlag;
    }
    else if (previous >= 0 && (our.m_grid == GridMid || our.m_grid == GridEnd || our.m_noFlag) && (our.m_noteType == chords[previous].m_noteType))
    {
        // middle or end of grid or no flag
        result = "x";
    }
    else
    {
        result = ourFlag;
    }
    
    // dotted
    if (our.m_dotted && result != "x")
    {
        result += ".";
    }
    
    return result;
}

std::string GenTab::GetRightFingering(const Note & note)
{
    switch (note.m_rightFingering)
    {
    case FingerNone:
        return "";
    case FingerFirst:
        return ".";
    case FingerSecond:
        return ":";
    case FingerThird:
        return ";";
    case FingerForth:
        return "";
    case FingerThumb:
        return "|";
    }
    return "";
}
    
std::string GenTab::GetLeftFingering(const Note & note)
{
    switch (note.m_leftFingering)
    {
    case FingerNone:
        return "";
    case FingerFirst:
        return "\\1";
    case FingerSecond:
        return "\\2";
    case FingerThird:
        return "\\3";
    case FingerForth:
        return "\\4";
    case FingerThumb:
        return "";
    }
    return "";
}

std::string GenTab::GetLeftOrnament(const Note & note)
{
    switch (note.m_leftOrnament)
    {
    case OrnNone:
        return "";
    case OrnHash:
        return "#";
    case OrnPlus:
        return "+";
    case OrnCross:
        return "x";
    case OrnLeftDot:
        return "*";
    case OrnBrackets:
        return "Q";
    }
    return "";
}

std::string GenTab::GetRightOrnament(const Note & note)
{
    switch (note.m_rightOrnament)
    {
    case OrnNone:
        return "";
    case OrnHash:
        return "&#";
    case OrnPlus:
        return "&+";
    case OrnCross:
        return "&x";
    case OrnLeftDot:
        return "&*";
    case OrnBrackets:
        return "";
    }
    return "";
}

std::string GenTab::GetFret(const Note & note, const Options& options)
{
    if (options.m_dstTabType == TabFrench)
    {
        // a..p, excluding j
        const std::string letter{static_cast<char>('a' + note.m_fret + (note.m_fret > 8 ? 1 : 0))};
        if (note.m_string >= 11)
        {
            return std::to_string(note.m_string - 7);
        }
        else if (note.m_string >= 8)
        {
            return std::string(note.m_string - 7, '/') + letter;
        }
        else
        {
            return letter;
        }
    }
    else
    {
        const std::string number{(note.m_fret <= 9)
            ? std::to_string(note.m_fret)
            : "N" + std::to_string(note.m_fret / 10) + std::to_string(note.m_fret % 10)};
        
        if (note.m_string >= 8)
        {
            return std::string(note.m_string - 7, '/') + number;
        }
        else
        {
            return number;
        }
    }
}

} // namespace luteconv
