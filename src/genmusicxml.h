#ifndef _GENMUSICXML_H_
#define _GENMUSICXML_H_

#include <iostream>

#include "xmlwriter.h"
#include "piece.h"
#include "options.h"

namespace luteconv
{

/**
 * Generate .musicxml
 */
class GenMusicXml
{
public:
    /**
     * Constructor
     */
    GenMusicXml() = default;

    /**
     * Destructor
     */
    ~GenMusicXml() = default;
    
    /**
     * Generate .musicxml to file specified in options
     * 
     * @param[in] options
     * @param[in] piece
     */
    void Generate(const Options& options, const Piece& piece);
    
    /**
     * Generate .musicxml
     * 
     * @param[in] options
     * @param[in] piece
     * @param[out] dst destination
     */
    void Generate(const Options& options, const Piece& piece, std::ostream& dst);
    
private:
    XMLElement* Measure(const Piece& src, const Bar& bar, int n, bool& repForward, const Options& options);
    XMLElement* FirstMeasureAttributes(const Piece& src, const Bar& bar, const Options& options);
    void AddTimeSignature(XMLElement* attributes, const Bar& bar);
    int Duration(NoteType noteType);
};



} // namespace luteconv

#endif // _GENMUSICXML_H_
