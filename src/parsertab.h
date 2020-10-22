#ifndef _PARSERTAB_H_
#define _PARSERTAB_H_

#include <iostream>

#include "piece.h"
#include "options.h"

namespace luteconv
{

/**
 * Parse Tab .tab file
 */
class ParserTab
{
public:

    /**
     * Constructor
    */
    ParserTab() = default;

    /**
     * Destructor
     */
    ~ParserTab() = default;
    
    /**
     * Parse .tab file
     *
     * @param[in] options
     * @param[out] piece destination
     */
    void Parse(const Options& options, Piece& piece);
    
    /**
     * Parse .tab file
     *
     * @param[in] src .tab stream
     * @param[in] options
     * @param[out] piece destination
     */
    void Parse(std::istream& srcFile, const Options& options, Piece& piece);
    
private:
    void ParseBarLine(const std::string& line, Bar& bar, bool& barIsClear, Piece& piece);
    void ParseChord(const std::string& line, int lineNo, Bar& bar, bool& barIsClear, int topString);
    void ParseTimeSignature(const std::string& line, Bar& bar);
    std::string CleanTabString(const std::string& src);
    
};

} // namespace luteconv

#endif // _TAB2XML_H_