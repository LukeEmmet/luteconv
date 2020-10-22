#ifndef _PARSERFT3_H_
#define _PARSERFT3_H_

#include <iostream>
#include <vector>
#include <cstdint>

#include "piece.h"
#include "options.h"

namespace luteconv
{

/**
 * Parse Fronimo .ft3 file
 * 
 * Fronimo is a closed source Windows lute tablature editor that uses
 * MFC CArchive as its file format.  The contents are undocumented and proprietary.
 * https://sites.google.com/view/fronimo/home
 *
 * Thanks to Luke Emmet for reverse engineering .ft3
 * https://bitbucket.org/loemmet/lutescribe
 *
 * Limitations
 * -----------
 * Single lute part
 * One piece per file
 * Renaissance lute, up to 10 courses
 * French tablature
 * No voice text
 * No mensural notation
 * No ties
 */
class ParserFt3
{
public:

    /**
     * Constructor
    */
    ParserFt3() = default;

    /**
     * Destructor
     */
    ~ParserFt3() = default;
    
    /**
     * Parse .tab file
     *
     * @param[in] options
     * @param[out] piece destination
     */
    void Parse(const Options& options, Piece& piece);
    
private:
    void Gunzip(const std::string& filename, std::vector<uint8_t>& ft3Image);
    
    void ParseHeader(const std::vector<uint8_t>::const_iterator headerBegin,
            const std::vector<uint8_t>::const_iterator headerEnd, Piece& piece);
    
    std::string ExtractRtf(const char* rtfBegin, int strLen);
    
    void ParseBody(const std::vector<uint8_t>::const_iterator bodyBegin,
            const std::vector<uint8_t>::const_iterator bodyEnd, Piece& piece);
    
    void ParseBar(int barNum, const std::vector<uint8_t>::const_iterator barBegin,
            const std::vector<uint8_t>::const_iterator barEnd, Piece& piece);
    
    void ParseTimeSignature(const std::vector<uint8_t>::const_iterator barBegin, Bar& bar);
    
    static bool AtNextNote(uint8_t s, uint8_t f);
    
    static Fingering GetRightFingering(uint16_t extras);
    
    static Fingering GetLeftFingering(uint16_t extras);
    
    static Ornament GetLeftOrnament(uint16_t extras);
    
    static Ornament GetRightOrnament(uint16_t extras);
};

} // namespace luteconv

#endif // _FT32XML_H_