#include "buffer.h"

#include "packing.h"
#include "util.h"

#include <cstdint>
#include <ranges>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

Buffer::Buffer() : cursor(0) {}
Buffer::Buffer(std::vector<uint8_t> bytes) : data(bytes), cursor(0) {}

std::uint8_t Buffer::read_byte() { return data[cursor++]; }
std::uint8_t Buffer::peek_byte() { return data[cursor]; }

std::span<uint8_t> Buffer::read(size_t len)
{
    std::span slice{data.data() + cursor, cursor + len};
    cursor += len;
    return slice;
}

std::span<uint8_t> Buffer::peek(size_t len)
{
    return std::span(data.data() + cursor, cursor + len);
}

void Buffer::seek(int bytes) { cursor += bytes; }

std::uint32_t Buffer::read_u32(std::endian endianess)
{
    return packing::pack_uint(read(4), endianess);
}

std::uint32_t Buffer::read_u16(std::endian endianess)
{
    return packing::pack_uint(read(2), endianess);
}

std::uint32_t Buffer::peek_u16(std::endian endianess)
{
    return packing::pack_uint(peek(2), endianess);
}

std::int32_t Buffer::read_i32(std::endian endianess)
{
    return packing::pack_int(read(4), endianess);
}

std::int32_t Buffer::read_7_bit_int()
{
    int32_t result = 0;
    int32_t bitsread = 0;
    int32_t value;

    do {
        value = read_byte();
        result |= (value & 0x7F) << bitsread;
        bitsread += 7;
    } while (value & 0x80);

    return result;
}

std::string Buffer::read_raw_string(size_t len)
{
    auto bytes = Buffer::read(len);
    return std::string(bytes.begin(), bytes.end());
}

std::string Buffer::read_string()
{
    // FIX: This is a hack that may easily break on othe files. Figure out
    // why it needs a +1
    return read_raw_string(read_7_bit_int() + 1);
}
