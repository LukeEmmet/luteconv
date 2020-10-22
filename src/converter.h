#ifndef _CONVERTER_H_
#define _CONVERTER_H_

#include "options.h"

namespace luteconv
{

/**
 * Convert lute tablature formats
 */
class Converter
{
public:
    /**
     * Constructor
     */
    Converter() = default;
    
    /**
     * Destructor
     */
    ~Converter() = default;
    
    /**
     * Covert lute tablature from src to destination format
     * 
     * @param[in] options
     */
    void Convert(const Options& options);
};


} // namespace luteconv

#endif // _CONVERTER_H_