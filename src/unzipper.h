#ifndef _UNZIPPER_H_
#define _UNZIPPER_H_

#include <vector>
#include <string>

namespace luteconv
{

/**
 * Parse .jtz file
 */
class Unzipper
{
public:
    
    /**
     * Constructor
     */
    Unzipper()= default;
    
    /**
     * Destructor
     */
    ~Unzipper() = default;
    
    /**
     * Unzip a file into a memory image
     * 
     * @param[in] filename
     * @param[out] image
     * @param[out] zipFilename - filename extracted from archive
     */
    static void Unzip(const std::string& filename, std::vector<char>& image, std::string& zipFilename);
};

} // namespace luteconv

#endif // _UNZIPPER_H_
