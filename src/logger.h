#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <sstream>

namespace luteconv
{

#define LOGGER \
if (!Logger::Verbose()) ; \
else Logger().Get()

//  A very very simple logger
class Logger
{
public:
    
    /**
     * Constructor
     */
    Logger() = default;
    
    /**
     * Destructor
     */
    ~Logger();

    /**
     * Get the logger
     */
    std::ostringstream& Get();
    
    /**
     * Set verbose logging
     * 
     * @param[in] verbode
     */
   static void SetVerbose(bool verbose);
   
   /**
    * Get verbosity
    * 
    * @return true <=> verbose
    */
   static bool Verbose();

private:
   std::ostringstream os;
   static bool m_verbose;
};

} // namespace luteconv

#endif // _LOGGER_H_

