#ifndef _PLATFORM_H_
#define _PLATFORM_H_

namespace luteconv
{

// platform specifics

#if defined(_WIN32) || defined(_WIN64)
const char pathSeparator = '\\';
#else
const char pathSeparator = '/';
#endif

} // namespace luteconv

#endif // _PLATFORM_H_
