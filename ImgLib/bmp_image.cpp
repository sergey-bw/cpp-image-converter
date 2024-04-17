#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <cstdint>
#include <fstream>

using namespace std;

namespace img_lib {

static const uint16_t BMP_SIG = 0x4D42; // "BP"

PACKED_STRUCT_BEGIN BitmapFileHeader{
    uint16_t signature = BMP_SIG;
    uint32_t reserved_1;
    uint32_t reserved_2 = 0;
    uint32_t offset_data = 54;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader{
    uint32_t info_header_size = 40;
    int32_t image_width;
    int32_t image_height;
    uint16_t planes = 1;
    uint16_t bit_per_pixel = 24;
    uint32_t compression = 0;
    uint32_t image_size;
    int32_t x_permission = 11811;
    int32_t y_permission = 11811;
    uint32_t collor_used = 0;
    uint32_t collor_count = 0x1000000;
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);

    const int32_t w = image.GetWidth();
    const int32_t h = image.GetHeight();
    const int32_t padding = GetBMPStride(w);
    const uint32_t img_size = padding * h;

    BitmapFileHeader file_header;
    file_header.reserved_1 = img_size + file_header.offset_data;
    out.write((const char*)&file_header, sizeof(BitmapFileHeader));
    
    BitmapInfoHeader info_header;
    info_header.image_width = w;
    info_header.image_height = h;
    info_header.image_size = img_size;
    out.write((const char*)&info_header, sizeof(BitmapInfoHeader));
    
    std::vector<char> buff(padding);

    for (int32_t y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int32_t x = 0; x < w; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(buff.data(), padding);
    }
    
    return out.good();
}

Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);

    BitmapFileHeader file_header;
    if (!ifs.read((char*)&file_header, sizeof(BitmapFileHeader))
        && file_header.signature != BMP_SIG) {
        return {};
    }

    BitmapInfoHeader info_header;
    if (!ifs.read((char*)&info_header, sizeof(BitmapInfoHeader))) {
        return {};
    };

    const int32_t w = info_header.image_width;
    const int32_t h = info_header.image_height;
    const uint32_t padding = static_cast<uint32_t>(GetBMPStride(w));
    if (w < 0 || h < 0) {
        return {};
    }

    Image result(w, h, Color::Black());
    std::vector<char> buff(padding);
    for (int32_t y = h - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), padding);
        for (int32_t x = 0; x < w; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib