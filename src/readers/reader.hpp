#pragma once

#include <buffer.hpp>

namespace readers
{
enum ReaderType
{
    Texture2D
};

struct Reader
{
    Reader(){};
    virtual ~Reader(){};
    virtual ReaderType type() = 0;
    virtual void read(Buffer buffer) = 0;
};
} // namespace readers
