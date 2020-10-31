#include "gentabcode.h"

#include <algorithm>
#include <fstream>
#include <chrono>
#include <iomanip>


namespace luteconv
{

void GenTabCode::Generate(const Options& options, const Piece& piece)
{
    std::fstream dst;
    dst.open(options.m_dstFilename.c_str(), std::fstream::out | std::fstream::trunc);
    if (!dst.is_open())
        throw std::runtime_error(std::string("Error: Can't open ") + options.m_dstFilename);
    
    Generate(options, piece, dst);
}

void GenTabCode::Generate(const Options& options, const Piece& piece, std::ostream& dst)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct std::tm * ptm = std::gmtime(&tt);
    
    // TabCode has no syntax for title, composer etc.  Just put everything in comments.
    dst << "{ Converted to .tab by luteconv " << options.m_version << " }" << std::endl
        << "{ encoding-date " << std::put_time(ptm,"%F") << " }" << std::endl;

    if (!piece.m_copyright.empty())
        dst << "{ " << piece.m_copyright << " }" << std::endl;
    
    if (!piece.m_title.empty())
        dst << "{ " << piece.m_title << " }" << std::endl;

    if (!piece.m_composer.empty())
        dst << "{ " << piece.m_composer << " }" << std::endl;

    for (const auto & credit : piece.m_credits)
    {
        if (!credit.m_left.empty())
            dst << "{ " << credit.m_left << " }" << std::endl;
        if (!credit.m_right.empty())
            dst << "{ " << credit.m_right << " }" << std::endl;
     }

    dst << std::endl;
     
    // body
    int staveNum{1};
    int barNum{1};
    int chordCount{0};
    std::string repForward;
    
    dst << "{ Stave " << staveNum << " }" << std::endl;
    
    for (const auto & bar : piece.m_bars)
    {
        if (chordCount == 0)
        {
            // first bar on line
            dst <<  "{ Bar " << barNum << " }" << std::endl;
            if (!repForward.empty())
            {
                dst << repForward << std::endl;
                repForward.clear();
            }
            else
            {
                dst << "|" << std::endl;
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
            std::string tabWord{GetFlagInfo(options, bar.m_chords, chord)};
            
            // notes. TabCode uses fret/string pairs so ordering in a tabword shouldn't matter,
            // but assume that it does.
            std::vector<std::string> vert(7);
            
            for (const auto & note : chord.m_notes)
            {
                if (note.m_string < 7)
                {
                    vert[note.m_string - 1] = GetFret(note)
                            + std::to_string(note.m_string)
                            + GetLeftOrnament(note)
                            + GetRightOrnament(note)
                            + GetLeftFingering(note)
                            + GetRightFingering(note);
                }
                else
                {
                    // diapasons don't have fingering or ornaments - is that right?
                    vert[6] = "X" + GetFret(note);
                }
            }
            
            for (const auto & s : vert)
            {
                tabWord += s;
            }
            
            dst << tabWord << std::endl;
        }
        
        // end of current bar
        ++barNum;
        dst <<  "{ Bar " << barNum << " }" << std::endl;
        
        // Stave ending Use herustic:
        // count chords, when the threshold is reached end the stave at the end of
        // the current bar.  Except for last bar.
        const int threshold = 25; // seems to work OK
        const bool lineBreak = barNum < static_cast<int>(piece.m_bars.size()) && chordCount > threshold;
        
        std::string barStyle;
        switch (bar.m_barStyle)
        {
        case BarStyleLightLight:
            barStyle = "||";
            break;
        default:
            barStyle = "|";
        }
        
        switch (bar.m_repeat)
        {
        case RepNone:
            break;
        case RepForward:
            barStyle += ":";
            break;
        case RepBackward:
            barStyle = ":" + barStyle;
            break;
        case RepJanus:
            if (lineBreak)
            {
                // backward repeat here, forward repeat in next bar, next line
                repForward = barStyle + ":";
                barStyle = ":" + barStyle;
            }
            else
            {
                barStyle = ":" + barStyle + ":";
            }
        }
        
        dst << barStyle << std::endl;
        
        if (lineBreak)
        {
            dst << "{^}" << std::endl;
            ++staveNum;
            chordCount = 0;
            dst << "{ Stave " << staveNum << " }" << std::endl;
        }
    }
}
    
std::string GenTabCode::GetTimeSignature(const Bar & bar)
{
    switch (bar.m_timeSig.m_timeSymbol)
    {
    case TimeSyNone:
        return "";
    case TimeSyCommon:
        return "M(C)";
    case TimeSyCut:
        return "M(C/)";
    case TimeSySingleNumber:
        return "M(" + std::to_string(bar.m_timeSig.m_beats) + ")";
    case TimeSyNote:
        return "";
    case TimeSyDottedNote:
        return "";
    case TimeSyNormal:
        return "M(" + std::to_string(bar.m_timeSig.m_beats) + "/" + std::to_string(bar.m_timeSig.m_beatType) + ")";
    }
    return "";
}

std::string GenTabCode::GetFlagInfo(const Options& options, const std::vector<Chord> & chords, const Chord & our)
{
    if (our.m_fermata)
        return "F";
    
    if (our.m_noFlag)
        return "";
    
    // TabCode documentation has Q = 1 flag, therefore B = NoteTypeWhole, not NoteTypeBreve
    NoteType adjusted{static_cast<NoteType>(our.m_noteType - 1 + options.m_flags)};
    adjusted = std::max(NoteTypeBreve, adjusted);
    adjusted = std::min(NoteType256th, adjusted);
    const std::string flags{"BWHQESTYZ"};
    std::string ourFlags;
    ourFlags = flags[adjusted - NoteTypeBreve];
    
    // dotted
    if (our.m_dotted)
    {
        ourFlags += ".";
    }
    
    return ourFlags;
}

std::string GenTabCode::GetRightFingering(const Note & note)
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
        return "(Fr...:7)";
    case FingerForth:
        return "";
    case FingerThumb:
        return "!";
    }
    return "";
}
    
std::string GenTabCode::GetLeftFingering(const Note & note)
{
    switch (note.m_leftFingering)
    {
    case FingerNone:
        return "";
    case FingerFirst:
        return "(Fl1:4)";
    case FingerSecond:
        return "(Fl2:4)";
    case FingerThird:
        return "(Fl3:4)";
    case FingerForth:
        return "(Fl4:4)";
    case FingerThumb:
        return "";
    }
    return "";
}

std::string GenTabCode::GetLeftOrnament(const Note & note)
{
    switch (note.m_leftOrnament)
    {
    case OrnNone:
        return "";
    case OrnHash:
        return "(Oe:4)";
    case OrnPlus:
        return "";
    case OrnCross:
        return "(Of:4)";
    case OrnLeftDot:
        return "";
    case OrnBrackets:
        return "";
    }
    return "";
}

std::string GenTabCode::GetRightOrnament(const Note & note)
{
    switch (note.m_rightOrnament)
    {
    case OrnNone:
        return "";
    case OrnHash:
        return "(Oe:5)";
    case OrnPlus:
        return "";
    case OrnCross:
        return "(Of:5)";
    case OrnLeftDot:
        return "";
    case OrnBrackets:
        return "";
    }
    return "";
}

std::string GenTabCode::GetFret(const Note & note)
{
    // a..p, excluding j
    const std::string letter{static_cast<char>('a' + note.m_fret + (note.m_fret > 8 ? 1 : 0))};
    if (note.m_string >= 11)
    {
        return std::to_string(note.m_string - 7);
    }
    else if (note.m_string >= 8)
    {
        return letter + std::string(note.m_string - 7, '/');
    }
    else
    {
        return letter;
    }
}

} // namespace luteconv
