#ifndef _PIECE_H_
#define _PIECE_H_

#include "pitch.h"
#include "options.h"

#include <string>
#include <vector>

namespace luteconv
{

// text alignment
enum Align
{
    AlignLeft,
    AlignRight,
    AlignCenter,
    AlignLeftRight
};

// Rhythm
enum NoteType
{
    //                               Should be   MuseCore            TAB      TabCode
    NoteTypeLong,    //                                              L
    NoteTypeBreve,   //                                              B
    NoteTypeWhole,   // semibreve    0 flags                         W        B
    NoteTypeHalf,    // minim        1 flag      0 flags short stem  w        W
    NoteTypeQuarter, // crotchet     2 flags     0 flags             0 flags  H
    NoteTypeEighth,  // quaver       3 flags     1 flag              1 flags  Q
    NoteType16th,    // semi         4 flags     2 flags             2 flags  E
    NoteType32nd,    // demi         5 flags     3 flags             3 flags  S
    NoteType64th,    // hemi         6 flags     4 flags             4 flags  T
    NoteType128th,   // semihemi     7 flags     5 flags             5 flags  Y
    NoteType256th    // demisemihemi 8 flags     6 flags                      Z
};

enum Fingering
{
    FingerNone,
    FingerFirst,
    FingerSecond,
    FingerThird,
    FingerForth,
    FingerThumb
};

enum Ornament
{
    OrnNone,
    OrnHash,
    OrnPlus,
    OrnCross,
    OrnLeftDot,
    OrnBrackets, // [] around note
};

enum Grid
{
    GridNone,
    GridStart,
    GridMid,
    GridEnd
};

enum BarStyle
{
    BarStyleRegular,
    BarStyleDotted,
    BarStyleDashed,
    BarStyleHeavy,
    BarStyleLightLight,
    BarStyleLightHeavy,
    BarStyleHeavyLight,
    BarStyleHeavyHeavy,
    BarStyleTick,
    BarStyleShort
};

enum Repeat
{
    RepNone,
    RepForward,
    RepBackward,
    RepJanus
};

enum TimeSymbol
{
    TimeSyNone,
    TimeSyCommon,
    TimeSyCut,
    TimeSySingleNumber,
    TimeSyNote,
    TimeSyDottedNote,
    TimeSyNormal
};

class Note
{
public:
    Note() = default;
    ~Note() = default;

    int m_string{0}; // 1...
    int m_fret{0}; // 0...
    
    Fingering m_leftFingering{FingerNone};
    Fingering m_rightFingering{FingerNone};
    Ornament m_leftOrnament{OrnNone};
    Ornament m_rightOrnament{OrnNone};
};

class Chord
{
public:
    Chord() = default;
    ~Chord() = default;

    NoteType m_noteType{NoteTypeQuarter};
    Grid m_grid{GridNone};
    bool m_dotted{false};
    bool m_fermata{false};
    bool m_noFlag{false};
    
    std::vector<Note> m_notes;
};

class TimeSig
{
public:
    TimeSymbol m_timeSymbol{TimeSyNone};
    int m_beats{0};
    int m_beatType{0};
};

class Bar
{
public:
    Bar() = default;
    ~Bar() = default;
    
    void Clear();
    TimeSig m_timeSig;
    BarStyle m_barStyle{BarStyleRegular};
    Repeat m_repeat{RepNone};
    bool m_fermata{false};
    bool m_eol{false};
    
    std::vector<Chord> m_chords;
};

class Credit
{
public:
    Align m_align{AlignLeft};
    std::string m_left;
    std::string m_right;
};

// Internal representation of tablature.  Source formats are first
// converted to class Piece, then class Piece is converted to the
// destination format.  In this manner for n formats we only need
// n parsers and n generators, rather than n^2 direct converters.
class Piece
{
public:
    Piece() = default;
    ~Piece() = default;
    
    /**
     * Set the tuning
     * 
     * @param[in] options
     */
    void SetTuning(const Options& options);
    
    std::string m_title;
    std::string m_composer;
    std::string m_copyright;
    bool m_copyrightEnabled{false};
    std::vector<Credit> m_credits;
    std::vector<Bar> m_bars;
    std::vector<Pitch> m_tuning;
};

} // namespace luteconv

#endif // _PIECE_H_
