#pragma once
#include <stdint.h>

struct riff_header {
    uint8_t     riff_mark[4];
    uint32_t    file_size;
    uint8_t     file_type[4];
    uint8_t     format_marker[4];
    uint32_t    format_len;
    uint16_t    format_type;
    uint16_t    channels;
    uint32_t    sample_rate;
    uint32_t    srxbpsxch;
    uint16_t    bpsxch;
    uint16_t    bits_per_sample;
    uint8_t     data_marker[4];
    uint32_t    data_size;
};
