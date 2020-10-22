#ifndef _PARSERMXL_H_
#define _PARSERMXL_H_

#include "options.h"
#include "piece.h"

#include <string>

namespace luteconv
{

/**
 * Parse MusicXML .mxl file
 */
class ParserMxl
{
public:

    /**
     * Constructor
    */
    ParserMxl() = default;

    /**
     * Destructor
     */
    ~ParserMxl() = default;
    
    /**
     * Parse .mxl file
     *
     * @param[in] options
     * @param[out] piece destination
     */
    void Parse(const Options& options, Piece& piece);
};

} // namespace luteconv

#endif // _PARSERMXL_H_
