#pragma once

#include "buffer.hpp"
#include "readers/reader.hpp"

#include <cstdint>
#include <vector>

namespace readers
{
struct Texture2DReader : Reader
{
    int surface_format; // Could be an enum
    int width;
    int height;
    int mipcount;
    size_t data_size;
    std::vector<uint8_t> bytes;

    Texture2DReader();
    ~Texture2DReader(){};

    virtual void read(Buffer buffer);
    virtual ReaderType type();
};

} // namespace readers
