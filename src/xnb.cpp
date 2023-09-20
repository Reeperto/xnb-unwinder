#include "xnb.h"

#include "lzx.h"
#include "util.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <format>
#include <fstream>
#include <iterator>
#include <vector>

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
        LOG("Data is compressed. Begin decompressing");
        decompress_lzx();
    }

    reader_count = buffer.read_7_bit_int();
    LOG("Reader count: ", reader_count);

    // Get all the type readers.
    for (int i = 0; i < reader_count; ++i) {
        std::string type = buffer.read_string();
        int version = buffer.read_i32();
        LOG("Reader:\n", "\t\t", "name: ", type, "\n", "\t\t",
            "version: ", version);
    }

    shared_resource_count = buffer.read_7_bit_int();
    LOG("Shared Resource Count: ", shared_resource_count);

    int surface_format = buffer.read_i32();
    int width = buffer.read_u32();
    int height = buffer.read_u32();
    int mipcount = buffer.read_u32();

    LOG("Surface Format: ", surface_format);
    LOG("Width: ", width);
    LOG("Height: ", height);
    LOG("Mip count: ", mipcount);

    auto data_size = buffer.read_u32();
    auto data = buffer.read(data_size);

    std::vector<uint8_t> image_data(data_size, 0);

    for (int chunk_start = 0; chunk_start < data_size - 4;
         chunk_start += 4) {
        image_data[chunk_start] = data[chunk_start + 1];
        image_data[chunk_start + 1] = data[chunk_start + 2];
        image_data[chunk_start + 2] = data[chunk_start + 3];
        image_data[chunk_start + 3] = data[chunk_start];
    }

    stbi_write_png("out.png", width, height, 4, data.data(), 4 * width);
}

void Xnb::read_header()
{
    if (buffer.read_raw_string(3) != "XNB") {
        valid = false;
        return;
    }

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
void Xnb::decompress_lzx()
{
    size_t compressed_todo = filesize - XNB_COMPRESSED_HEADER_SIZE;

    LOG("File size: ", filesize,
        ", Decompresed size: ", decompressed_filesize);

    std::vector<uint8_t> decompressed_data(decompressed_filesize, 0);

    auto lzx = LZXinit(16);

    size_t out_pos = 0;

    uint8_t hi;
    uint8_t lo;

    int block_size;
    int frame_size;

    while (buffer.cursor - XNB_COMPRESSED_HEADER_SIZE < compressed_todo) {

        hi = buffer.read_byte();
        lo = buffer.read_byte();

        block_size = (hi << 8) | lo;
        frame_size = 0x8000;

        if (hi == 0xFF) {
            hi = lo;
            lo = buffer.read_byte();
            frame_size = (hi << 8) | lo;
            hi = buffer.read_byte();
            lo = buffer.read_byte();
            block_size = (hi << 8) | lo;
        }

        if (block_size == 0 || frame_size == 0) {
            break;
        }

        LOG("Block Size: ", block_size, ", Frame Size: ", frame_size);

        LZXdecompress(lzx, buffer.read(block_size).data(),
                      decompressed_data.data() + out_pos, block_size,
                      frame_size);

        out_pos += frame_size;
    }

    std::copy(decompressed_data.begin(), decompressed_data.end(),
              buffer.data.begin() + XNB_COMPRESSED_HEADER_SIZE);

    LZXteardown(lzx);
    buffer.cursor = XNB_COMPRESSED_HEADER_SIZE;

    LOG("Finished decompressing");
}
