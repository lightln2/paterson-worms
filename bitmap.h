#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "util.h"

#pragma pack(push)
#pragma pack(1)

struct BMP_FILE_HEADER {
    uint16_t header;
    uint32_t file_size;
    uint16_t reserved[2];
    uint32_t offset;
};

struct BMP_INFO_HEADER {
    uint32_t header_size;
    uint32_t width, height;
    uint16_t color_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    uint32_t x_pixels_per_meter, y_pixels_per_meter;
    uint32_t pallette_size, important_colors_count;
};

#pragma pack(pop)

struct RGB {
    uint8_t r,g,b;
};

void save_bitmap(const std::string& filename, const std::vector<RGB>& input, uint32_t width, uint32_t height) {
    assert(input.size() == width * height);
    uint32_t size = 3 * width * height;
    uint32_t header_size = sizeof(BMP_FILE_HEADER) + sizeof(BMP_INFO_HEADER);
    BMP_FILE_HEADER fileHeader{0x4D42, header_size + size, 0, 0, header_size};
    BMP_INFO_HEADER infoHeader{sizeof(BMP_INFO_HEADER), width, height, 1, 24, 0, size, 0, 0, 0, 0 };
    std::ofstream out(filename, std::ios::out | std::ios::binary);
    out.write((char*)&fileHeader, sizeof(BMP_FILE_HEADER));
    out.write((char*)&infoHeader, sizeof(BMP_INFO_HEADER));
    char padding_bytes[4] = {0, 0, 0, 0};
    uint32_t padding_size = (4 - width*3%4) % 4;
    for (size_t i = 0; i < height; i++) {
        out.write((const char*)&input[i*width], width*3);
        out.write(padding_bytes, padding_size);
    }
}
