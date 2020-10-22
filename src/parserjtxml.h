#ifndef _PARSERJTXML_H_
#define _PARSERJTXML_H_

#include <pugixml.hpp>

#include <string>

#include "options.h"
#include "piece.h"

namespace luteconv
{

/**
 * Parse Fronimo .jtxml file
 */
class ParserJtxml
{
public:

    /**
     * Constructor
    */
    ParserJtxml() = default;

    /**
     * Destructor
     */
    ~ParserJtxml() = default;
    
    /**
     * Parse .jtxml file image in buffer
     *
     * @param[in] filename .jtxml
     * @param[in] contents .jtxml image
     * @param[in] size
     * @param[in] options
     * @param[out] piece destination
     */
    void Parse(const std::string& filename, void* contents, size_t size, const Options& options, Piece& piece);
    
    /**
     * Parse .jtxml file
     *
     * @param[in] options
     * @param[out] piece destination
     */
    void Parse(const Options& options, Piece& piece);
    
private:
    void Parse(const std::string& filename, pugi::xml_document& doc, pugi::xml_parse_result& result, const Options& options, Piece& piece);
    void ParseTuning(pugi::xml_node& xmlsection, Piece& piece);
    void ParseEvent(pugi::xml_node& xmlevent, Piece& piece);
    void ParseFlag(int flagNum, Chord& chord);
};

} // namespace luteconv

#endif // _PARSERJTXML_H_
