#pragma once

#include <iostream>

#ifdef XNA_LOG
#define LOG(...) std::cout, "[DEBUG] ", __VA_ARGS__, std::endl

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
#else
#define LOG(...)
#endif
