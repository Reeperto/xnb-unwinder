#include "readers/texture2d.hpp"

#include "util.hpp"

namespace readers
{
Texture2D read_texture2d(Buffer buffer)
{
    Texture2D texture;

    texture.surface_format = buffer.read_i32();
    texture.width = buffer.read_u32();
    texture.height = buffer.read_u32();
    texture.mipcount = buffer.read_u32();
    texture.data_size = buffer.read_u32();
    texture.bytes = buffer.copy_out(texture.data_size);

    DEBUG("Surface Format: ", texture.surface_format);
    DEBUG("Width: ", texture.width);
    DEBUG("Height: ", texture.height);
    DEBUG("Mip count: ", texture.mipcount);

    return texture;
}
} // namespace readers
