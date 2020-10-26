#include "parserft3.h"

#include <zlib.h>

#include <stdexcept>
#include <algorithm>
#include <regex>
#include <iterator>
#include <array>

namespace luteconv
{

void ParserFt3::Parse(const Options& options, Piece& piece)
{
    // .ft3 files are (usually) gzipped.  A curious choice of compression for a Windows program.
    std::vector<uint8_t> ft3Image;
    Gunzip(options.m_srcFilename, ft3Image);
    
    const std::string cpiece{"CPiece"};
    auto headerBegin = std::search(ft3Image.cbegin(), ft3Image.cend(), cpiece.cbegin(), cpiece.cend());
    if (headerBegin == ft3Image.cend())
        throw std::runtime_error(std::string("Error: CPiece not found"));
    
    const std::string cbar{"CBar"};
    auto headerEnd = std::search(headerBegin, ft3Image.cend(), cbar.cbegin(), cbar.cend());
    if (headerEnd == ft3Image.cend())
        throw std::runtime_error(std::string("Error: CBar not found"));
    
    auto bodyBegin = headerEnd + cbar.size();

    ParseHeader(headerBegin, headerEnd, piece);
    ParseBody(bodyBegin, ft3Image.cend(), piece);
    piece.SetTuning(options);
}

void ParserFt3::Gunzip(const std::string& filename, std::vector<uint8_t>& ft3Image)
{
    gzFile ft3File = gzopen(filename.c_str(), "rb");
    if (!ft3File)
        throw std::runtime_error("Error: Can't open " + filename);
    
    // decompress and read all the file into memory
    const unsigned int blockSize = 4096;
    unsigned int ft3ImageSize{0};
    ft3Image.resize(blockSize);
    
    for (;;)
    {
        const int nRead = gzread(ft3File, ft3Image.data() + ft3ImageSize, blockSize);
        if (nRead < 0)
        {
            gzclose(ft3File);
            throw std::runtime_error(std::string("Error: Reading ") + filename);
        }
        
        ft3ImageSize += static_cast<unsigned int>(nRead);
        if (nRead < static_cast<int>(blockSize))
            break;
    
        ft3Image.resize(ft3ImageSize + blockSize);
    }
    
    ft3Image.resize(ft3ImageSize);
    
    gzclose(ft3File);
}

void ParserFt3::ParseHeader(const std::vector<uint8_t>::const_iterator headerBegin,
        const std::vector<uint8_t>::const_iterator headerEnd, Piece& piece)
{
    std::vector<uint8_t>::const_iterator ptr = headerBegin + 14;
    int strLen = *ptr++;
    piece.m_title = ExtractRtf(reinterpret_cast<const char *>(&*ptr), strLen);
    
    ptr += strLen;
    strLen = *ptr++;
    const std::string author = ExtractRtf(reinterpret_cast<const char *>(&*ptr), strLen);
    if (!author.empty())
    {
        Credit credit;
        credit.m_align = AlignLeft;
        credit.m_left = author;
        piece.m_credits.push_back(credit);
    }

    ptr += strLen;
    strLen = *ptr++;
    piece.m_composer = ExtractRtf(reinterpret_cast<const char *>(&*ptr), strLen);
}

// Extract the text from RTF, this is a crude regex that works well
// enough in practice but could be fooled by more complex RTF.
std::string ParserFt3::ExtractRtf(const char * rtfBegin,
        int strLen)
{
    std::string rtf{rtfBegin, rtfBegin + strLen};
    
    // c++ regex doesn't have the equivalent of perl's /s to enable matching \n and \r
    rtf.erase(std::remove(rtf.begin(), rtf.end(), '\n'), rtf.end());
    rtf.erase(std::remove(rtf.begin(), rtf.end(), '\r'), rtf.end());
    
    const std::regex re{R"(\{(\\[\w\-;]+|\{[^\}\{]*|\}|[\r\n]*)*\s*([^\\\{]*)(\\[\w\-;]+|\{[^\}\{]*|\}|[\r\n]*)*\})"};
    std::cmatch results;
    if (std::regex_match(rtf.c_str(), results, re))
    {
        return results[2];
    }
    else
    {
        return rtf;
    }
}

void ParserFt3::ParseBody(const std::vector<uint8_t>::const_iterator bodyBegin,
        const std::vector<uint8_t>::const_iterator bodyEnd, Piece& piece)
{
    const std::array<uint8_t, 2> x03x80{0x03, 0x80};
    auto barBegin = bodyBegin;
    
    for (int barNum = 1; ; ++barNum)
    {
        auto barEnd = std::search(barBegin, bodyEnd, x03x80.cbegin(), x03x80.cend());
        ParseBar(barNum, barBegin, barEnd, piece);
        barBegin = barEnd;
        
        if (barBegin == bodyEnd)
            break;

        barBegin += x03x80.size();
    }
}

void ParserFt3::ParseBar(int barNum, const std::vector<uint8_t>::const_iterator barBegin,
        const std::vector<uint8_t>::const_iterator barEnd, Piece& piece)
{
    Bar bar;
    ParseTimeSignature(barBegin, bar);
    
    auto ptr = barBegin + 32;
    
    // chords
    for (;;)
    {
        while (std::distance(ptr, barEnd) >= 9 && !AtNextNote(ptr[4], ptr[5]))
        {
            ++ptr;
        }
        
        if (std::distance(ptr, barEnd) < 9)
            break;
        
        Chord chord;
        static_assert(NoteTypeQuarter == 4, "");
        chord.m_noteType = static_cast<NoteType>(ptr[0] + 2); // musecore has crotchet with 0 flags
        if (ptr[1] & 0x02)
            chord.m_grid = GridStart;
        else if (ptr[1] & 0x04)
            chord.m_grid = GridMid;
        else if (ptr[1] & 0x08)
            chord.m_grid = GridEnd;
        chord.m_dotted = !!(ptr[1] & 0x10);
        ptr += 4;
        
        // notes
        while (AtNextNote(ptr[0], ptr[1]))
        {
            Note note;
            
            if (ptr[0] < 8)
            {
                note.m_string = ptr[0] - 1; // string 2..7 => 1..6
                note.m_fret = ptr[1] - 0x30; // fret a..p
            }
            else if (ptr[0] == 8)
            {
                 // use the note flag slot to determine what kind of string or fret we have here...
                 if (ptr[4] == 0x00)
                 {
                     // fretted 7th course, letter will be given
                     note.m_string = 7;
                     note.m_fret = ptr[1] - 0x61;
                 }
                 else if (ptr[4] == 0x20)
                 {
                     // open diapason string below 7th (00 = 7, 01 = 8 etc)
                     note.m_string = ptr[1] - 0x30 + 7;
                     note.m_fret = 0;
                 }
                 else if (ptr[4] == 0x48)
                 {
                     // means a fretted 8th - letter will be given
                     note.m_string = 8;
                     note.m_fret = ptr[1] - 0x61;
                 }
            }
            
            const uint16_t extras = ptr[3] << 8 | ptr[2];
            note.m_rightFingering = GetRightFingering(extras);
            note.m_leftFingering = GetLeftFingering(extras);
            note.m_rightOrnament = GetRightOrnament(extras);
            note.m_leftOrnament = GetLeftOrnament(extras);
            
            ptr += 5;
            
            chord.m_notes.push_back(note);
        }
        bar.m_chords.push_back(chord);
    }
    
    piece.m_bars.push_back(bar);
}

void ParserFt3::ParseTimeSignature(const std::vector<uint8_t>::const_iterator barBegin, Bar& bar)
{
    const uint8_t timeSignature = barBegin[0] & 0x7f;

    // time signature
    if (timeSignature == 0x01)
    {
        bar.m_timeSymbol = TimeSyCommon;
        bar.m_beats = 4;
        bar.m_beatType = 4;
    }
    else if (timeSignature == 0x02)
    {
        bar.m_timeSymbol = TimeSyCut;
        bar.m_beats = 2;
        bar.m_beatType = 2;
    }
    else if (timeSignature == 0x03)
    {
        bar.m_timeSymbol = TimeSySingleNumber;
        bar.m_beats = 3;
        bar.m_beatType = 4;
    }
    else if (timeSignature == 0x06)
    {
        bar.m_timeSymbol = TimeSyNormal;
        bar.m_beats = barBegin[9];
        bar.m_beatType = barBegin[8];
    }
    else
    {
        bar.m_timeSymbol = TimeSyNone;
        bar.m_beats = 4;
        bar.m_beatType = 4;
    }
}

// detects if we are at another note in the current chord
bool ParserFt3::AtNextNote(uint8_t s, uint8_t f)
{
    const bool onFret = (f >= 0x30) && (f <= 0x3E); // up to fret p
    const bool onDiapason = (f >= 0x61) && (f <= 0x66); // up to fret f on diapasons
    
    const bool onString = (s >= 0x02) && (s <= 0x08); 

    return onString && (onFret || onDiapason);
}

Fingering ParserFt3::GetRightFingering(uint16_t extras)
{
    // right hand fingering
    if (extras & 0x0002) // thumbstroke
    {
        return FingerThumb;
    }
    else if (extras & 0x0004) // . underneath
    {
        return FingerFirst;
    }
    else if (extras & 0x0008) // .. underneath
    {
        return FingerSecond;
    }
    
    // TODO tab supports ... with ";", does Fronimo?
    
    return FingerNone;
}
    
    
Fingering ParserFt3::GetLeftFingering(uint16_t extras)
{
    // left hand fingering
    if (extras & 0x0020) // left finger1
    {
        return FingerFirst;
    }
    else if (extras & 0x0040) // left finger2
    {
        return FingerSecond;
    }
    else if (extras & 0x0080) // left finger3
    {
        return FingerThird;
    }
    else if (extras & 0x0100) // left finger 4
    {
        return FingerForth;
    }
    
    return FingerNone;
}

Ornament ParserFt3::GetLeftOrnament(uint16_t extras)
{
    // left ornaments
    const uint16_t ornament = extras & 0xfe00;
    if (ornament == 0x0400) // # ornament
    {
        return OrnHash;;
    }
    else if (ornament == 0x0800) // + ornament
    {
        return OrnPlus;
    }
    else if (ornament == 0x4a00) // single . on left
    {
        return OrnLeftDot;
    }
    else if (ornament == 0x0c00) // x ornament
    {
        return OrnCross;
    }
    else if (extras == 0x3400) // sq brackets on both sides e.g. [a]
    {
        return OrnBrackets;
    }
    
    return OrnNone;
}

Ornament ParserFt3::GetRightOrnament(uint16_t extras)
{
    // right ornaments
    const uint16_t ornament = extras & 0xfe00;
    if (ornament == 0x0600) // # ornament
    {
        return OrnHash;
    }
    
    // TODO other rhs ornaments when I know the values
    
    return OrnNone;
}

} // namespace luteconv
