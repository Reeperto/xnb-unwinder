#pragma once

#include <cstdint>
#include <span>

namespace packing
{

std::uint64_t pack_uint(std::span<uint8_t> bytes,
                        std::endian endianess = std::endian::little);

std::int64_t pack_int(std::span<uint8_t> bytes,
                      std::endian endianess = std::endian::little);

} // namespace packing
