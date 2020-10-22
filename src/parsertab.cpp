#include "parsertab.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>

#include "logger.h"

namespace luteconv
{

void ParserTab::Parse(const Options& options, Piece& piece)
{
    std::fstream src;
    src.open(options.m_srcFilename.c_str(), std::fstream::in);
    if (!src.is_open())
        throw std::runtime_error("Error: Can't open " + options.m_srcFilename);
    Parse(src, options, piece);
}
    
void ParserTab::Parse(std::istream& src, const Options& options, Piece& piece)
{
    int lineNo{0};
    Bar bar;
    bool barIsClear{true};
    int section{0};
    int target{std::stoi(options.m_index)};
    TabType tabType = options.m_srcTabType;
    int topString{tabType == TabItalian ? 6 : 1};
    
    while (!src.eof())
    {
        std::string line;
        getline(src, line);
        ++lineNo;
        
        // remove trailing spaces
        line.erase(std::find_if(line.rbegin(), line.rend(), std::bind1st(std::not_equal_to<char>(), ' ')).base(), line.end());
        
        // assume sections are separated by end of page
        if (line == "p")
        {
            ++section;
            if (section > target)
                break;
            else
                continue;
        }
        
        if (section != target)
            continue;
        
        if (line.empty())
        {
            if (barIsClear && !piece.m_bars.empty())
            {
                // end of line at end of bar
                Bar& prev = piece.m_bars.back();
                prev.m_eol = true;
            }
            continue;   // (blank line)  - break line here - you must specify line
                        // breaks yourself!
        }
        
        if (line[0] == 'e') // end of document - tab will work but will complain without it.
            break;
        
        if (line == "-s")
        {
            // italian 7 course tablature
            if (tabType == TabUnknown || tabType == TabItalian)
            {
                tabType = TabItalian;
                topString = 7;
            }
            continue;
        }
        
        if (line == "-i" || line == "-O" || line == "$numstyle=italian")
        {
            // italian or spanish tablature: assume italian
            if (tabType == TabUnknown)
            {
                tabType = TabItalian;
                topString = 6;
            }
            continue;
        }
        
        if (line == "-milan")
        {
            // spanish tablature
            if (options.m_srcTabType == TabUnknown && (tabType == TabUnknown || tabType == TabItalian))
            {
                tabType = TabSpanish;
                topString = 1;
            }
            continue;
        }
        
        const std::string tuning{"-tuning "};
        if (line.substr(0, tuning.size()) == tuning)
        {
            try
            {
                Pitch::SetTuningTab(line.substr(tuning.size()).c_str(), piece.m_tuning);
            }
            catch (...)
            {
                LOGGER << lineNo << ": \"" << line << "\" -tuning syntax error, ignored";
                piece.m_tuning.clear();
            }
            continue;
        }
        
        if (line.substr(0, 8) == "$scribe=")
        {
            piece.m_copyright = line.substr(8);
            
            // Prefix "Copyright <year>", unless already there
            if (piece.m_copyright.find("Copyright") == std::string::npos)
            {
                std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                struct std::tm * ptm = std::gmtime(&tt);
                std::ostringstream ss;
                ss << "Copyright " << std::put_time(ptm,"%Y") << " " << piece.m_copyright;
                piece.m_copyright = ss.str();
            }
            continue;
        }
        
        if (line == "-G")
        {
            piece.m_copyrightEnabled = true;
            continue;
        }
        
        switch (line[0])
        {
        case '{':   // { words }     - one of more lines of text (unjustified, not wrapped)
        {
            if (piece.m_bars.empty())
            {
                Credit credit;
                const size_t slash = line.find('/');
                if (line.substr(1, 4) == "\\CL/")
                {
                    credit.m_align = AlignCenter;
                    credit.m_left = CleanTabString(line.substr(5, line.size() - 6));
                }
                else if (slash != line.std::string::npos)
                {
                    const std::string left = CleanTabString(line.substr(1, slash - 1));
                    const std::string right = CleanTabString(line.substr(slash + 1, line.size() - slash - 2));
                    if (!left.empty() && !right.empty())
                    {
                        credit.m_align = AlignLeftRight;
                        credit.m_left = left;
                        credit.m_right = right;
                    }
                    else if (!left.empty() && right.empty())
                    {
                        credit.m_align = AlignLeft;
                        credit.m_left = left;
                    }
                    else if (left.empty() && !right.empty())
                    {
                        credit.m_align = AlignRight;
                        credit.m_right = right;
                    }
                }
                else
                {
                    credit.m_align = AlignLeft;
                    credit.m_left = CleanTabString(line.substr(1, line.size() - 2));
                }
                
                if (!credit.m_left.empty() || !credit.m_right.empty())
                {
                    piece.m_credits.push_back(credit);
                }
            }
            break;
        }
        case '%':   // comment
            break;
        case 'b':   // barline
            // [[fallthrough]]
        case '.':   // column of 5 dots (for a repeat) as in .bb.
                    // with the "-count_dots" this will increment the bar count
                    // unless followed by an "X"
            ParseBarLine(line, bar, barIsClear, piece);
            break;
        case 'B':   // a very thick barline (when followed by newline or !)
                    // a breve (when followed by notes)
            if (line.size() == 1 || line[1] == '!')
                ParseBarLine(line, bar, barIsClear, piece);
            else
                ParseChord(line, lineNo, bar, barIsClear, topString);
            break;
        case 'C':   // a big C
            // [[fallthrough]]
        case 'c':   // cut time in both staves if necessary
            // [[fallthrough]]
        case 'S':   // a time signature - ie S3-4 or SC
                    // a dash, as 12-8 draws the signature in the music
                    // rather than the tablature.
                    // S2| or S3| draws a line throught the number
                    // a single digit followed by Q is highlighted
                    // SO and So do Narvaez style time sig
                    // S0  (that is a zero) gives  the O for perfect time
            ParseTimeSignature(line, bar);
            break;
        case '#':   // first note of a grid followed by number of lines in grid
            // [[fallthrough]]
        case '0':
            // [[fallthrough]]
        case '1':
            // [[fallthrough]]
        case '2':
            // [[fallthrough]]
        case '3':
            // [[fallthrough]]
        case '4':
            // [[fallthrough]]
        case '5':   // a note with that many flags
            // [[fallthrough]]
        case 'W':   // a whole note  0
            // [[fallthrough]]
        case 'w':   // a half note
            // [[fallthrough]]
        case 'L':   // longa
            // [[fallthrough]]
        case 'x':   // same number of flags as the last one
            ParseChord(line, lineNo, bar, barIsClear, topString);
            break;
        case 't':   // first note of a triplet, followed by the
                    // followed by the number of lines there would be
                    // if there were lines - eg. t3 a,x b, xb,
                    // this places a 3 above the second note.
                    // a t before a barline is a tie across the barline
            break;
        case 'Y':   // fermata
            // [[fallthrough]]
        case 'y':   // another fermata
                    // a Yb or yb draws a fermata above a barline
                    // Y. YW Yw YB also work'
            if (line.size() == 1 || line[1] == '.' || line[1] == 'b' || (line.size() == 2 && line[1] == 'B'))
            {
                bar.m_fermata = true;
                ParseBarLine(line.substr(1), bar, barIsClear, piece);
            }
            else
            {
                // if the Y replaces the flag count then substitute 0
                const char flags[] = {"012345wWBL"};
                const char * const flagsEnd = flags + sizeof(flags);
                if (std::find(flags, flagsEnd, line[1]) != flagsEnd)
                    ParseChord(line.substr(1), lineNo, bar, barIsClear, topString);
                else
                    ParseChord("0" + line.substr(1), lineNo, bar, barIsClear, topString);
                bar.m_chords.back().m_fermata = true;
            }
            break;
        
        default:
            LOGGER << lineNo << ": \"" << line << "\" ignored";
        }
    }
    
    // deal with missing final bar line
    ParseBarLine("B", bar, barIsClear, piece);

    piece.SetTuning(options);
}

std::string ParserTab::CleanTabString(const std::string& src)
{
    std::string dst;
    
    for (size_t i = 0; i < src.size(); ++i)
    {
        if (src.size() - i >= 3 && src[i] == '^' && isdigit(src[i + 1]) && isdigit(src[i + 2]))
        {
            i += 3; // skip font
        }
        else if (src[i] != '\\')
        {
            dst += src[i];
        }
    }
    return dst;
}

void ParserTab::ParseBarLine(const std::string& line, Bar& bar, bool& barIsClear, Piece& piece)
{
    if (line.substr(0, 4) == ".bb." || line.substr(0, 5) == ".b.b.")
    {
        bar.m_barStyle = BarStyleLightLight;
        bar.m_repeat = RepJanus;
    }
    else if (line.substr(0, 3) == ".b.")
    {
        bar.m_barStyle = BarStyleRegular;
        bar.m_repeat = RepJanus;
    }
    else if (line.substr(0, 3) == ".bb")
    {
        bar.m_barStyle = BarStyleLightLight;
        bar.m_repeat = RepBackward;
    }
    else if (line.substr(0, 3) == "bb.")
    {
        bar.m_barStyle = BarStyleLightLight;
        bar.m_repeat = RepForward;
    }
    else if (line.substr(0, 3) == "b.b")
    {
        bar.m_barStyle = BarStyleLightLight;
        bar.m_repeat = RepJanus;
    }
    else if (line.substr(0, 2) == ".b")
    {
        bar.m_barStyle = BarStyleRegular;
        bar.m_repeat = RepBackward;
    }
    else if (line.substr(0, 2) == "b.")
    {
        bar.m_barStyle = BarStyleRegular;
        bar.m_repeat = RepForward;
    }
    else if (line.substr(0, 2) == "bb")
    {
        bar.m_barStyle = BarStyleLightLight;
    }
    else if (line.substr(0, 1) == "B")
    {
        bar.m_barStyle = BarStyleHeavy;
    }
    else if (line.substr(0, 1) == "b")
    {
        bar.m_barStyle = BarStyleRegular;
    }
    else if (line == ".")
    {
        bar.m_repeat = barIsClear ? RepForward : RepBackward;
    }
    
    if (barIsClear && !piece.m_bars.empty())
    {
        // two adjacent bar lines, combine
        Bar& prev = piece.m_bars.back();
        if (prev.m_repeat == RepBackward && bar.m_repeat == RepForward)
        {
            prev.m_repeat = RepJanus;
        }
        
        if (!prev.m_eol)
        {
            if (prev.m_barStyle == BarStyleRegular && bar.m_barStyle == BarStyleRegular)
            {
                prev.m_barStyle = BarStyleLightLight;
            }
            else if (prev.m_barStyle == BarStyleRegular && bar.m_barStyle == BarStyleHeavy)
            {
                prev.m_barStyle = BarStyleLightHeavy;
            }
            else if (prev.m_barStyle == BarStyleHeavy && bar.m_barStyle == BarStyleRegular)
            {
                prev.m_barStyle = BarStyleHeavyLight;
            }
            else if (prev.m_barStyle == BarStyleHeavy && bar.m_barStyle == BarStyleHeavy)
            {
                prev.m_barStyle = BarStyleHeavyHeavy;
            }
        }
    }
    else if (!barIsClear)
    {
        piece.m_bars.push_back(bar);
    }
    
    bar.Clear();
    barIsClear = true;
}

void ParserTab::ParseChord(const std::string& line, int lineNo, Bar& bar, bool& barIsClear, int topString)
{
    bar.m_chords.emplace_back(); // new chord
    Chord & chord = bar.m_chords.back();
    static Chord previousChord;
    
    // flags
    size_t idx{0};
    if (line[idx] == 'x')
    {
        // same number of flags as the last one
        // get flags from previous chord
        chord.m_noteType = previousChord.m_noteType;
        chord.m_dotted = previousChord.m_dotted;
        if (previousChord.m_grid == GridNone)
            chord.m_noFlag = true;
        else
            chord.m_grid = GridMid;
        ++idx;
    }
    else
    {
        if (line[idx] == '#')
        {
            // first note of a grid
            chord.m_grid = GridStart;
            ++idx;
            if (idx >= line.size())
                return;
        }
        
        if (line[idx] >= '0' && line[idx] <= '5')
        {
            // a note with that many flags
            // TODO TAB thinks 0 flag is a crotchet.
            chord.m_noteType = static_cast<NoteType>(NoteTypeQuarter + line[idx] - '0');
            ++idx;
        }
        else if (line[idx] == 'w')
        {
            // a half note |
            //             0
            chord.m_noteType = NoteTypeHalf;
            ++idx;
        }
        else if (line[idx] == 'W')
        {
            // a whole note 0
            chord.m_noteType = NoteTypeWhole;
            ++idx;
        }
        else if (line[idx] == 'B')
        {
            // a breve (when followed by notes)
            chord.m_noteType = NoteTypeBreve;
            ++idx;
        }
        else if (line[idx] == 'L')
        {
            // longa
            chord.m_noteType = NoteTypeLong;
            ++idx;
        }
        else
        {
            // ignore
            bar.m_chords.pop_back();
            return;
        }
        
        if (idx < line.size() && line[idx] == '!')
        {
            // spacing, ignore
            ++idx;
        }
        
        if (idx < line.size() && line[idx] == '.')
        {
            // a dotted flag
            chord.m_dotted = true;
            ++idx;
        }
        
        // # can appear before or after the flag
        if (idx < line.size() && line[idx] == '#')
        {
            // first note of a grid
            chord.m_grid = GridStart;
            ++idx;
        }
     }
    
    previousChord = chord; // only need to save the flags data, not the notes.
        
    if (idx < line.size() && line[idx] == '-')
    {
        // a place marker, technically should be the default second character
        // but usually can be left out - it helps when you want to use a
        // *, # or - ornament on the character on the top line.
        ++idx;
    }
    
    // notes
    const int stringInc{topString == 1 ? +1 : -1}; // up for french, down for italian
    int string{topString - stringInc};

    barIsClear = false;
    
    while (idx < line.size())
    {
        string += stringInc;
        
        if (line[idx] == ' ' || line[idx] == '-')
        {
            // empty character position
            ++idx;
            continue;
        }
        
        chord.m_notes.emplace_back(); // allocate new note
        Note& note = chord.m_notes.back();
        
        // optional left ornament or left fingering followed by letter or diapason
        while (idx < line.size())
        {
            // left fingering
            if (line[idx] == '\\')
            {
                ++idx;
                if (idx < line.size())
                {
                    if (line[idx] >= '1' && line[idx] <= '4')
                    {
                        // left hand fingerings placed before the next letter.
                        note.m_leftFingering = static_cast<Fingering>(FingerNone + line[idx] - '0');
                        ++idx;
                        continue;
                    }
                }
            }
            else
            {
            
                // left ornament
                const std::string leftOrnaments{"#+x*Q"};
                const size_t leftOrnPos = leftOrnaments.find(line[idx]);
                if (leftOrnPos != std::string::npos)
                {
                    note.m_leftOrnament = static_cast<Ornament>(leftOrnPos + OrnHash);
                    ++idx;
                    continue;
                }
            }
            
            // french, string 11 .. 14
            if (idx < line.size() && topString == 1 && string >= 7 && line[idx] >= '4' && line[idx] <= '7')
            {
                note.m_fret = 0;
                note.m_string = 11 + line[idx] - '4';
                ++idx;
                break;
            }

            // /diapason
            int diapason{0};
            if (string >= 7 && line[idx] == '/')
            {
                // diapason
                do
                {
                    ++diapason;
                    ++idx;
                }
                while (idx < line.size() && line[idx] == '/');
            }

            // fret letters
            if (idx < line.size() && line[idx] >= 'a' && line[idx] <= 'p')
            {
                // fret
                note.m_fret = line[idx] - 'a' - (line[idx] <= 'i' ? 0 : 1); // fret i = j
                note.m_string = (diapason >= 1) ? 7 + diapason : string;
                ++idx;
                break;
            }
            
            // upper case fret letters: F G H I give characters like M Board used
            if (idx < line.size() && line[idx] >= 'F' && line[idx] <= 'I')
            {
                // fret
                note.m_fret = line[idx] - 'A';
                note.m_string = (diapason >= 1) ? 7 + diapason : string;
                ++idx;
                break;
            }
             
            // fret numbers 0..9
            if (idx < line.size() && isdigit(line[idx]))
            {
                // fret
                note.m_fret = line[idx] - '0';
                note.m_string = (diapason >= 1) ? 7 + diapason : string;
                ++idx;
                break;
            }
            
            // fret number >= 10
            if (idx < line.size() - 3 && line[idx] == 'N' && isdigit(line[idx + 1]) && isdigit(line[idx + 2]))
            {
                // fret
                note.m_fret = (line[idx + 1] - '0') * 10 + (line[idx + 2] - '0');
                note.m_string = (diapason >= 1) ? 7 + diapason : string;
                idx += 3;
                break;
            }
            
            // fret number roman X
            if (idx < line.size() - 2 && line[idx] == '!' && line[idx + 1] == 'x')
            {
                // fret
                note.m_fret = 10;
                note.m_string = (diapason >= 1) ? 7 + diapason : string;
                idx += 2;
                break;
            }
            
            if (idx < line.size() && line[idx] == '&')
            {
                // postfix operator but no note at character position
                string -= stringInc;
                break;
            }
            
            if (idx < line.size() - 1 && (line[idx] == '!' || (line[idx] == '\\' && line[idx] == '\\')))
            {
                // escape operator or \\ one backslash in note position
                idx += 2;
                break;
            }
            
            if (idx < line.size() && line[idx] == '\"')
                ++idx; // ignore prefix operator and the prefix
                
            // ignore other prefixes
            LOGGER << lineNo << ": \"" << line << "\" ignoring prefix \"" << line[idx] << "\"";
            ++idx;
            break;
        }
    
        // optional right fingering and ornament
        
        while (idx < line.size())
        {
            if (line[idx] == '.')
            {
                note.m_rightFingering = FingerFirst;
                string += stringInc; // right hand fingering replaces note on next line
            }
            else if (line[idx] == ':')
            {
                note.m_rightFingering = FingerSecond;
                string += stringInc; // right hand fingering replaces note on next line
            }
            else if (line[idx] == ';')
            {
                note.m_rightFingering = FingerThird;
                string += stringInc; // right hand fingering replaces note on next line
            }
            else if (line[idx] == '|')
            {
                note.m_rightFingering = FingerThumb;
                string += stringInc; // right hand fingering replaces note on next line
            }
            else if (line[idx] == '&')
            {
                ++idx;
                if (idx >= line.size())
                    break;
                
                const std::string rightOrnaments{"#+x*"};
                const size_t rightOrnPos = rightOrnaments.find(line[idx]);
                if (rightOrnPos != std::string::npos)
                {
                    note.m_rightOrnament = static_cast<Ornament>(rightOrnPos + OrnHash);
                }
                else
                {
                    LOGGER<< lineNo << ": \"" << line << "\" ignoring postfix \"&" << line[idx] << "\"";
                }
            }
            else
            {
                break;
            }
            
            ++idx;
        }
        
        // note not needed
        if (note.m_string == 0)
            chord.m_notes.pop_back();
    }
}

void ParserTab::ParseTimeSignature(const std::string& line, Bar& bar)
{
    if (line == "C" || line == "SC")
    {
        bar.m_timeSymbol = TimeSyCommon;
        bar.m_beats = 4;
        bar.m_beatType = 4;
    }
    else if (line == "c" || line == "Sc")
    {
        bar.m_timeSymbol = TimeSyCut;
        bar.m_beats = 2;
        bar.m_beatType = 2;
    }
    else if (line.size() == 2 && line[1] >= '1' && line[1] <= '9')
    {
        bar.m_timeSymbol = TimeSySingleNumber;
        bar.m_beats = line[1] - '0';
        bar.m_beatType = 4;
    }
    else if (line.size() == 3 && line[1] >= '1' && line[1] <= '9' && line[2] >= '1' && line[2] <= '9')
    {
        bar.m_timeSymbol = TimeSyNormal;
        bar.m_beats = line[1] - '0';
        bar.m_beatType = line[2] - '0';
    }
}

} // namespace luteconv
