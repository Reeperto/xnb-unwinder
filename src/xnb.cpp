#include "xnb.hpp"

#include "lzx.h"
#include "readers/texture2d.hpp"
#include "util.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <format>
#include <fstream>
#include <iterator>
#include <ostream>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const uint8_t HIDEF_MASK = 0x1;
const uint8_t COMPRESSED_LZX_MASK = 0x80;
const uint8_t COMPRESSED_LZ4_MASK = 0x80;

const size_t XNB_COMPRESSED_HEADER_SIZE = 14;

Xnb::Xnb(std::string path)
{
    std::ifstream instream(path, std::ios::in | std::ios::binary);
    std::vector<uint8_t> out((std::istreambuf_iterator<char>(instream)),
                             std::istreambuf_iterator<char>());
    buffer = Buffer(out);

    read_header();

    if (compressed) {
        INFO("Data is compressed with LZX. Decompressing");
        buffer = decompress_lzx();
        INFO("Data is uncompressed");
    }

    reader_count = buffer.read_7_bit_int();
    INFO("Reader count: ", reader_count);

    std::vector<readers::Reader *> reader_list(reader_count);

    // Get all the type readers.
    for (int i = 0; i < reader_count; ++i) {
        std::string type = buffer.read_string();
        int version = buffer.read_i32();
        DEBUG("Reader: ", type);

        if (type.starts_with(
                "Microsoft.Xna.Framework.Content.Texture2DReader")) {
            reader_list.push_back(new readers::Texture2DReader);
        }
    }

    shared_resource_count = buffer.read_7_bit_int();
    DEBUG("Shared Resource Count: ", shared_resource_count);

    // XXX: This was a big pain. The content data is polymorphic, so in
    // order to determine what data lies first, a 7 bit int is used to
    // index into the list of readers constructed above. Then the
    // respective reader is used to actually read the data. Hence no need
    // for any hacky +1 issues.

    int read_index = buffer.read_7_bit_int();
    DEBUG("Read index: ", read_index);

    auto reader = reader_list[read_index];
    reader->read(buffer);

    auto texture = (readers::Texture2DReader *)reader;

    // Valid and working. Weird visual artificats are in the actual pixel
    // data itself.
    stbi_write_png("out.png", texture->width, texture->height, 4,
                   texture->bytes.data(), 4 * texture->width);
}

void Xnb::read_header()
{
    if (buffer.read_raw_string(3) != "XNB") {
        valid = false;
        return;
    }

    INFO("File is valid XNB");

    target = static_cast<char>(buffer.read_byte());
    format_version = static_cast<int>(buffer.read_byte());

    uint8_t flags = buffer.read_byte();

    hidef = flags & HIDEF_MASK;

    if (flags & COMPRESSED_LZX_MASK) {
        compressed = true;
        compression_type = CompressionType::LZX;
    } else if (flags & COMPRESSED_LZ4_MASK) {
        compressed = true;
        compression_type = CompressionType::LX4;
    }

    filesize = buffer.read_u32();

    if (compressed) {
        decompressed_filesize = buffer.read_u32();
    }
}

/*
 * If an XNB file is compressed with LZX compression, it requires some
 * special handling. The contents of the compressed section following the
 * header is not contiguously compressed LZX data. Instead, it
 * is a collection of chunks. Each chunk has an associated frame size. The
 * frame size represents the size of the chunk when uncompressed. Before
 * each chunk are some indicator bytes. They come in two forms:
 *
 * 	1.)
 * 	0xFF [2 Bytes] [2 Bytes]
 * 	^       ^         ^
 * 	flag   int16    int16
 *
 * 	2.)
 * 	[2 Bytes]
 * 	    ^
 * 	  int16
 *
 * In the first case, the flag indicates that both the chunk and frame size
 * are going to be defined. The first int16 is the frame size and the
 * second int16 is the chunk size.
 *
 * In the second case, the int16 represents the chunk size. The frame size
 * is then assumed to be 32 kb or 0x8000.
 *
 */
Buffer Xnb::decompress_lzx()
{
    size_t compressed_todo = filesize - XNB_COMPRESSED_HEADER_SIZE;

    DEBUG("File size: ", filesize,
          ", Decompresed size: ", decompressed_filesize);

    auto compressed_data = buffer.peek(compressed_todo);

    buffer.cursor = XNB_COMPRESSED_HEADER_SIZE;

    std::vector<uint8_t> decompressed_data(decompressed_filesize, 0);

    auto lzx = LZXinit(16);

    size_t out_pos = 0;
    size_t pos = 0;

    uint8_t hi;
    uint8_t lo;

    int block_size;
    int frame_size;

    while (pos < compressed_todo) {

        hi = compressed_data[pos++];
        lo = compressed_data[pos++];

        block_size = (hi << 8) | lo;
        frame_size = 0x8000;

        if (hi == 0xFF) {
            hi = lo;
            lo = compressed_data[pos++];
            frame_size = (hi << 8) | lo;
            hi = compressed_data[pos++];
            lo = compressed_data[pos++];
            block_size = (hi << 8) | lo;
        }

        if (block_size == 0 || frame_size == 0) {
            break;
        }

        DEBUG("Block Size: ", block_size, ", Frame Size: ", frame_size);

        LZXdecompress(lzx, compressed_data.data() + pos,
                      decompressed_data.data() + out_pos, block_size,
                      frame_size);

        out_pos += frame_size;
        pos += block_size;
    }

    LZXteardown(lzx);
    return Buffer(decompressed_data);
}
