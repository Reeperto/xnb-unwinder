#include <cstdint>
#include <span>

// NOTE: When considering the endianess of binary data, the order of
// bytes is what matters. Bit-ordering *is* preserved between
// corresponding bytes. Therefore all that needs to be done to pack a
// uint is to iterate in the right direction.
namespace packing
{

std::uint64_t pack_uint(std::span<uint8_t> bytes, std::endian endianess)
{
    uint64_t result = 0;
    if (endianess == std::endian::little) {
        for (auto byte = bytes.rbegin(); byte != bytes.rend(); ++byte) {
            result <<= 8;
            result |= *byte;
        }
    } else if (endianess == std::endian::big) {
        for (auto byte : bytes) {
            result <<= 8;
            result |= byte;
        }
    }
    return result;
}

std::int64_t pack_int(std::span<uint8_t> bytes,
                      std::endian endianess = std::endian::little)
{
    return (int64_t)pack_uint(bytes, endianess);
}

} // namespace packing
