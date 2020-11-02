#ifndef _PARSERMEI_H_
#define _PARSERMEI_H_

#include <pugixml.hpp>
#include "options.h"
#include "piece.h"

namespace luteconv
{

/**
 * Parse .mei file
 */
class ParserMei
{
public:

    /**
     * Constructor
    */
    ParserMei() = default;

    /**
     * Destructor
     */
    ~ParserMei() = default;
    
    /**
     * Parse .mei file image in buffer
     *
     * @param[in] filename .mei
     * @param[in] contents .mei image
     * @param[in] size
     * @param[in] options
     * @param[out] piece destination
     */
    void Parse(const std::string& filename, void* contents, size_t size, const Options& options, Piece& piece);
    
    /**
     * Parse .mei file
     *
     * @param[in] options
     * @param[out] piece destination
     */
    void Parse(const Options& options, Piece& piece);
    
private:
    void Parse(const std::string& filename, pugi::xml_document& doc, pugi::xml_parse_result& result, const Options& options, Piece& piece);
    void ParseTabGrpList(pugi::xml_node& xmlmeasure, pugi::xml_node& xmlparent, Grid grid, Bar& bar);
    void ParseTabGrp(pugi::xml_node& xmlmeasure, pugi::xml_node& xmltabGrp, Grid grid, Bar& bar);
    void ParseNoteList(pugi::xml_node& xmlmeasure, pugi::xml_node& xmlparent, Chord& chord);
    void ParseFingering(pugi::xml_node& xmlmeasure, const std::string& xmlid, Note& note);
    void ParseCourseTuning(pugi::xml_node& xmlcourseTuning, Piece& piece);
    TimeSig ParseTimeSignature(pugi::xml_node& xmlmensur);
};

} // namespace luteconv

#endif // _PARSERMEI_H_
