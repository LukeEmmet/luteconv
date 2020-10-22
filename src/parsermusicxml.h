#ifndef _PARSERMUSICXML_H_
#define _PARSERMUSICXML_H_

#include <pugixml.hpp>
#include "options.h"
#include "piece.h"

namespace luteconv
{

/**
 * Parse .musicxml file
 */
class ParserMusicXml
{
public:

    /**
     * Constructor
    */
    ParserMusicXml() = default;

    /**
     * Destructor
     */
    ~ParserMusicXml() = default;
    
    /**
     * Parse .musicxml file image in buffer
     *
     * @param[in] filename .musicxml
     * @param[in] contents .musicxml image
     * @param[in] size
     * @param[in] options
     * @param[out] piece destination
     */
    void Parse(const std::string& filename, void* contents, size_t size, const Options& options, Piece& piece);
    
    /**
     * Parse .musicxml file
     *
     * @param[in] options
     * @param[out] piece destination
     */
    void Parse(const Options& options, Piece& piece);
    
private:
    void Parse(const std::string& filename, pugi::xml_document& doc, pugi::xml_parse_result& result, const Options& options, Piece& piece);
    void ParseStaffTuning(pugi::xml_node& xmlpart, Piece& piece);
    void ParseBarline(pugi::xml_node& xmlmeasure, Piece& piece);
    void ParseTimeSignature(pugi::xml_node& xmlmeasure, Piece& piece);
    void ParseCredit(pugi::xml_node& xmlscorePartwise, Piece& piece);
};




} // namespace luteconv

#endif // _PARSERMUSICXML_H_
