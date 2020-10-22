#ifndef _PARSERTABCODE_H_
#define _PARSERTABCODE_H_

#include <iostream>

#include "piece.h"
#include "options.h"

namespace luteconv
{

/**
 * Parse TabCode .tc file
 * 
 * http://doc.gold.ac.uk/isms/ecolm/?page=TabCode
 */
class ParserTabCode
{
public:

    /**
     * Constructor
    */
    ParserTabCode() = default;

    /**
     * Destructor
     */
    ~ParserTabCode() = default;
    
    /**
     * Parse .tc file
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
    void ParseCodeWord(const std::string& tabword, int lineNo, Bar& bar, bool& barIsClear, Piece& piece);
    void ParseExtra(const std::string& tabword, int lineNo, const std::string& extra, Note& note);
    void ParseBarLine(const std::string& tabword, int lineNo, Bar& bar, bool& barIsClear, Piece& piece);
    void ParseChord(const std::string& tabword, int lineNo, Bar& bar, bool& barIsClear);
    void ParseTimeSignature(const std::string& tabword, int lineNo, Bar& bar);
};

} // namespace luteconv

#endif // _TAB2XML_H_