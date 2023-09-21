#pragma once

#include "buffer.hpp"

#include <cstdint>
#include <vector>

namespace readers
{
struct Texture2D
{
    int surface_format; // Could be an enum
    int width;
    int height;
    int mipcount;
    size_t data_size;
    std::vector<uint8_t> bytes;
};

Texture2D read_texture2d(Buffer buffer);

} // namespace readers
