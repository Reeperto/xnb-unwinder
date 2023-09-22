#include "readers/texture2d.hpp"

#include "reader.hpp"
#include "util.hpp"

namespace readers
{
Texture2DReader::Texture2DReader()
    : surface_format(0), width(0), height(0), mipcount(0), data_size(0),
      bytes{}
{
}

ReaderType Texture2DReader::type() { return Texture2D; }

void Texture2DReader::read(Buffer buffer)
{
    surface_format = buffer.read_i32();
    width = buffer.read_u32();
    height = buffer.read_u32();
    mipcount = buffer.read_u32();
    data_size = buffer.read_u32();
    bytes = buffer.copy_out(data_size);

    DEBUG("Surface Format: ", surface_format);
    DEBUG("Width: ", width);
    DEBUG("Height: ", height);
    DEBUG("Mip count: ", mipcount);
}
} // namespace readers
