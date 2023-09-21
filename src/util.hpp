#pragma once

#include <iostream>

#define INFO(...) std::cout, " [INFO] ", __VA_ARGS__, std::endl

#ifdef XNA_LOG
#define DEBUG(...) std::cout, "[DEBUG] ", __VA_ARGS__, std::endl
#else
#define DEBUG(...)
#endif

template <typename T>
std::ostream &operator,(std::ostream &out, const T &t)
{
    out << t;
    return out;
}

// overloaded version to handle all those special std::endl and others...
inline std::ostream &operator,(std::ostream &out,
                        std::ostream &(*f)(std::ostream &))
{
    out << f;
    return out;
}
