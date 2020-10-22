#ifndef _GENMXL_H_
#define _GENMXL_H_

#include <iostream>

#include "options.h"
#include "piece.h"

namespace luteconv
{

/**
 * Generate .mxl (compressed MusicXML)
 */
class GenMxl
{
public:
    /**
     * Constructor
     */
    GenMxl() = default;

    /**
     * Destructor
     */
    ~GenMxl() = default;
    
    /**
     * Generate .mxl
     * 
     * @param[in] options
     * @param[in] piece
     */
    void Generate(const Options& options, const Piece& piece);
};

} // namespace luteconv

#endif // _GENMXL_H_
