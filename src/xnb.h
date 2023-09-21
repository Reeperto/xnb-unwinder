#include "buffer.h"

#include <_types/_uint8_t.h>
#include <cassert>
#include <string>

struct Xnb
{
    typedef enum
    {
        None,
        LZX,
        LX4
    } CompressionType;

    Buffer buffer;

    bool valid = false;

    char target = 0;
    uint8_t format_version = 0;

    bool hidef = false;
    bool compressed = false;

    CompressionType compression_type = CompressionType::None;
    size_t filesize = 0;
    size_t decompressed_filesize = 0;

    int reader_count = 0;
    int shared_resource_count = 0;

    Xnb(std::string path);

    void read_header();
    Buffer decompress_lzx();
};
