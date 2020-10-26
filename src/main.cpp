#include "converter.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

/**
 * tab to musicxml
 * 
 * @param[in] argc number of arguments
 * @param[in] argv arguments.
 * @retval 0 => OK
 * @retval 1 => error
 */
int main(int argc, char *argv[]) 
{
    try
    {
        luteconv::Options options;
        options.ProcessArgs(argc, argv);

        luteconv::Converter converter;
        converter.Convert(options);
    }
    catch (const std::exception & e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
