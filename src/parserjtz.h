#ifndef _PARSERJTZ_H_
#define _PARSERJTZ_H_

#include "options.h"
#include "piece.h"

#include <string>

namespace luteconv
{

/**
 * Parse Fronimo .jtz file
 */
class ParserJtz
{
public:

    /**
     * Constructor
    */
    ParserJtz() = default;

    /**
     * Destructor
     */
    ~ParserJtz() = default;
    
    /**
     * Parse .jtz file
     *
     * @param[in] options
     * @param[out] piece destination
     */
    void Parse(const Options& options, Piece& piece);
};

} // namespace luteconv

#endif // _PARSERJTZ_H_
