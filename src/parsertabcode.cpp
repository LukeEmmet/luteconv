#include "parsertabcode.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>

#include "logger.h"

namespace luteconv
{

void ParserTabCode::Parse(const Options& options, Piece& piece)
{
    std::fstream src;
    src.open(options.m_srcFilename.c_str(), std::fstream::in);
    if (!src.is_open())
        throw std::runtime_error("Error: Can't open " + options.m_srcFilename);
    Parse(src, options, piece);
}
    
void ParserTabCode::Parse(std::istream& src, const Options& options, Piece& piece)
{
    LOGGER << "Parse TabCode";

    // As far as I can see TabCode doesn't have syntax for the title, composer or copyright.
    // Use the filename for the title
    const size_t slash = options.m_srcFilename.find_last_of('/');
    if (slash == std::string::npos)
        piece.m_title = options.m_srcFilename;
    else
        piece.m_title = options.m_srcFilename.substr(slash + 1);

    int lineNo{0};
    Bar bar;
    bool barIsClear{true};
    bool inComment{false};
    
    while (!src.eof())
    {
        std::string line;
        getline(src, line);
        ++lineNo;

        // Tokenize: a token is delimited by whitespace or endl or a comment.
        // Comments are tokenized as they can contain semantic information.
        // (IMHO comments should only contain comments!)
        size_t tokEnd = 0;
        for (size_t tokBegin = 0; tokBegin < line.size(); tokBegin = tokEnd)
        {
            // find start of token
            while (tokBegin < line.size() && (line[tokBegin] == ' ' || line[tokBegin] == '\t'))
                ++tokBegin;
            
            // find end of token
            tokEnd = tokBegin + 1;
            while (tokEnd < line.size() && line[tokEnd] != ' ' && line[tokEnd] != '\t' && line[tokEnd] != '{')
                ++tokEnd;
            
            // deal with comments
            if (inComment)
            {
                if (line[tokEnd - 1] == '}')
                {
                    inComment = false;
                }
                continue;
            }
    
            if (line[tokBegin] == '{')
            {
                // end of system is encoded in a comment {^}!
                if (line.substr(tokBegin, 3) == "{^}" && barIsClear && !piece.m_bars.empty())
                {
                    Bar& prev = piece.m_bars.back();
                    prev.m_eol = true;
                }
                
                if (line[tokEnd - 1] != '}')
                {
                     inComment = true;
                }
                continue;
            }
            
            ParseCodeWord(line.substr(tokBegin, tokEnd - tokBegin), lineNo, bar, barIsClear, piece);
        }
    }
    
    // deal with missing final bar line
    ParseBarLine("|", lineNo, bar, barIsClear, piece);

    // TODO some tabcode files have a commented XML section <rules> which can contain
    // tuning information.  Can't find any documentation for this.  Attempt to parse?
    piece.SetTuning(options);
}

void ParserTabCode::ParseCodeWord(const std::string& tabword, int lineNo, Bar& bar, bool& barIsClear, Piece& piece)
{
    switch (tabword[0])
    {
    case '|':   // barline
        // [[fallthrough]]
    case ':':   // barline
        ParseBarLine(tabword, lineNo, bar, barIsClear, piece);
        break;
    case 'M':
        ParseTimeSignature(tabword, lineNo, bar);
        break;
    default:
        ParseChord(tabword, lineNo, bar, barIsClear);
        break;
    }
}

void ParserTabCode::ParseBarLine(const std::string& tabword, int lineNo, Bar& bar, bool& barIsClear, Piece& piece)
{
    if (tabword == "|")
    {
        bar.m_barStyle = BarStyleRegular;
    }
    else if (tabword == "||")
    {
        bar.m_barStyle = BarStyleLightLight;
    }
    else if (tabword == "|:")
    {
        bar.m_barStyle = BarStyleRegular;
        bar.m_repeat = RepForward;
    }
    else if (tabword == "||:")
    {
        bar.m_barStyle = BarStyleLightLight;
        bar.m_repeat = RepForward;
    }
    else if (tabword == ":|")
    {
        bar.m_barStyle = BarStyleRegular;
        bar.m_repeat = RepBackward;

    }
    else if (tabword == ":||")
    {
        bar.m_barStyle = BarStyleLightLight;
        bar.m_repeat = RepBackward;
    }
    else if (tabword == ":|:")
    {
        bar.m_barStyle = BarStyleRegular;
        bar.m_repeat = RepJanus;
    }
    else if (tabword == ":||:")
    {
        bar.m_barStyle = BarStyleLightLight;
        bar.m_repeat = RepJanus;
    }
    else if (tabword == "|=")
    {
        bar.m_barStyle = BarStyleRegular;
    }
    else if (tabword == "|0")
    {
        bar.m_barStyle = BarStyleRegular;
    }
    else if (tabword == "|=0")
    {
        bar.m_barStyle = BarStyleRegular;
    }
    else
    {
        bar.m_barStyle = BarStyleRegular;
        LOGGER << lineNo << ": \"" << tabword << "\" unknown barline";
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

void ParserTabCode::ParseChord(const std::string& tabword, int lineNo, Bar& bar, bool& barIsClear)
{
    bar.m_chords.emplace_back(); // new chord
    Chord & chord = bar.m_chords.back();
    static Chord previousChord;
    
    // flags
    size_t idx{0};
    const std::string flags{"BWHQESTYZ"};
    const size_t flagPos = flags.find(tabword[idx]);
    if (flagPos != std::string::npos)
    {
        // TabCode documentation has Q = 1 flag, therefore B = NoteTypeWhole, not NoteTypeBreve
        chord.m_noteType = static_cast<NoteType>(NoteTypeWhole + flagPos);
        ++idx;
        
        if (idx < tabword.size() && tabword[idx] == '.')
        {
            chord.m_dotted = true;
            ++idx;
        }
        previousChord = chord; // only need to save the flags data, not the notes.
    }
    else if (tabword[idx] == 'F')
    {
        chord.m_fermata = true;
        chord.m_noteType = NoteTypeHalf;
        ++idx;
    }
    else
    {
        chord.m_noteType = previousChord.m_noteType;
        chord.m_dotted = previousChord.m_dotted;
        chord.m_noFlag = true;
    }
    
    // notes
    
    barIsClear = idx >= tabword.size();

    while (idx < tabword.size())
    {
        if (tabword[idx] >= 'a' && tabword[idx] <= 'p')
        {
            chord.m_notes.emplace_back(); // allocate new note
            Note& note = chord.m_notes.back();

            note.m_fret = tabword[idx] - 'a' - (tabword[idx] <= 'i' ? 0 : 1); // fret i = j
            ++idx;
            
            if (idx < tabword.size() && tabword[idx] >= '0' && tabword[idx] <= '6')
            {
                note.m_string = tabword[idx] - '0';
                ++idx;
            }
            else
            {
                LOGGER << lineNo << ": \"" << tabword << "\" missing string number";
            }
        }
        else if (tabword[idx] == 'X')
        {
            chord.m_notes.emplace_back(); // allocate new note
            Note& note = chord.m_notes.back();

            // diapasons
            ++idx;
            
            // french, string 11 .. 14
            if (idx < tabword.size() && tabword[idx] >= '4' && tabword[idx] <= '7')
            {
                note.m_fret = 0;
                note.m_string = 11 + tabword[idx] - '4';
                ++idx;
            }
            else
            {
                // fret letters
                if (idx < tabword.size() && tabword[idx] >= 'a' && tabword[idx] <= 'p')
                {
                    // fret
                    note.m_fret = tabword[idx] - 'a' - (tabword[idx] <= 'i' ? 0 : 1); // fret i = j
                     ++idx;
                }
                else
                {
                    LOGGER << lineNo << ": \"" << tabword << "\" missing fret letter";
                }

                // /diapason
                int diapason{0};
                if (idx < tabword.size() && tabword[idx] == '/')
                {
                    // diapason
                    do
                    {
                        ++diapason;
                        ++idx;
                    }
                    while (idx < tabword.size() && tabword[idx] == '/');
                }
    
                note.m_string = 7 + diapason;
            }
        }
        else if (tabword[idx] == '(')
        {
            // fingering, ornament or line
            std::string extra;
            extra = '(';
            ++idx;
            while (idx < tabword.size() && tabword[idx] != ')')
            {
                extra += tabword[idx];
                ++idx;
            }
            extra += ')';
            
            if (idx < tabword.size())
                ++idx; // eat )
            
            if (!chord.m_notes.empty())
                ParseExtra(tabword, lineNo, extra, chord.m_notes.back());
        }
        else if (tabword[idx] == '.')
        {
            if (!chord.m_notes.empty())
                chord.m_notes.back().m_rightFingering = FingerFirst;
            ++idx;
        }
        else if (tabword[idx] == ':')
        {
            if (!chord.m_notes.empty())
                chord.m_notes.back().m_rightFingering = FingerSecond;
            ++idx;
        }
        else if (tabword[idx] == '!')
        {
            if (!chord.m_notes.empty())
                chord.m_notes.back().m_rightFingering = FingerThumb;
            ++idx;
        }
        else
        {
            LOGGER << lineNo << ": \"" << tabword << "\" unknown tabword";
            ++idx;
        }
    }
}

void ParserTabCode::ParseExtra(const std::string& tabword, int lineNo, const std::string& extra, Note& note)
{
    // left hand fingering, at any position
    if (extra.substr(0, 5) == "(Fl1:")
    {
        note.m_leftFingering = FingerFirst;
    }
    else if (extra.substr(0, 5) == "(Fl2:")
    {
        note.m_leftFingering = FingerSecond;
    }
    else if (extra.substr(0, 5) == "(Fl3:")
    {
        note.m_leftFingering = FingerThird;
    }
    else if (extra.substr(0, 5) == "(Fl4:")
    {
        note.m_leftFingering = FingerForth;
    }
    // right hand fingering, at any position
    else if (extra.substr(0, 5) == "(Fr.:")
    {
        note.m_rightFingering = FingerFirst;
    }
    else if (extra.substr(0, 6) == "(Fr..:")
    {
        note.m_rightFingering = FingerSecond;
    }
    else if (extra.substr(0, 7) == "(Fr...:")
    {
        note.m_rightFingering = FingerThird;
    }
    else if (extra.substr(0, 5) == "(Fr!:")
    {
        note.m_rightFingering = FingerThumb;
    }
    // # ornament.  Put positions 358 on the right, 12467 on the left
    // 123
    // 4 5
    // 678
    else if ((extra.substr(0, 4) == "(Oe:"))
    {
        if (extra[5] == '3' || extra[5] == '5' || extra[5] == '8')
            note.m_rightOrnament = OrnHash;
        else
            note.m_leftOrnament = OrnHash;
    }
    // x ornament.  Put positions 358 on the right, 12467 on the left
    // 123
    // 4 5
    // 678
    else if ((extra.substr(0, 4) == "(Of:"))
    {
        if (extra[5] == '3' || extra[5] == '5' || extra[5] == '8')
            note.m_rightOrnament = OrnCross;
        else
            note.m_leftOrnament = OrnCross;
    }
    else
    {
        LOGGER << lineNo << ": \"" << tabword << "\" extra \"" << extra << "\" ignored";
    }
}

void ParserTabCode::ParseTimeSignature(const std::string& tabword, int lineNo, Bar& bar)
{
    if (tabword == "M(C)")
    {
        bar.m_timeSymbol = TimeSyCommon;
        bar.m_beats = 4;
        bar.m_beatType = 4;
    }
    else if (tabword == "M(C/)")
    {
        bar.m_timeSymbol = TimeSyCut;
        bar.m_beats = 2;
        bar.m_beatType = 2;
    }
    else if (tabword.size() == 4 && tabword[1] >= '1' && tabword[1] <= '9')
    {
        // M(3)
        bar.m_timeSymbol = TimeSySingleNumber;
        bar.m_beats = tabword[1] - '0';
        bar.m_beatType = 4;
    }
    else if (tabword.size() == 6 && tabword[2] >= '1' && tabword[2] <= '9' && tabword[3] >= '/' && tabword[4] >= '1' && tabword[4] <= '9')
    {
        // M(3/4)
        bar.m_timeSymbol = TimeSyNormal;
        bar.m_beats = tabword[2] - '0';
        bar.m_beatType = tabword[4] - '0';
    }
    else
    {
        LOGGER << lineNo << ": \"" << tabword << "\" unknown time signature";
    }
}

} // namespace luteconv
