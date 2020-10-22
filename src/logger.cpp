#include <logger.h>

#include <chrono>
#include <iomanip>
#include <iostream>

namespace luteconv
{

bool Logger::m_verbose{false};

void Logger::SetVerbose(bool verbose)
{
    m_verbose = verbose;
}

bool Logger::Verbose()
{
    return m_verbose;
}

std::ostringstream& Logger::Get()
{
    std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct std::tm * ptm = std::gmtime(&tt);

    os << std::put_time(ptm,"%FT%T") << " ";
    return os;
}

Logger::~Logger()
{
    std::cout << os.str() << std::endl;
}

} // namespace luteconv
