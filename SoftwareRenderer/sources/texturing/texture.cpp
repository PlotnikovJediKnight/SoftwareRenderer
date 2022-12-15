#include "headers/texturing/texture.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdexcept>

using namespace std;

namespace pv {

    Texture::Texture(size_t width, size_t height, TEXTURE_COLOR_MODEL textClrModel) :
        components_count_(TEXT_MODEL_COMPONENTS_COUNT[static_cast<int>(textClrModel)]),
        buffer_pointer_(new uchar[width * height * components_count_]),
        width_(width),
        height_(height) { }

    Texture::~Texture() {
        delete[] buffer_pointer_;
        buffer_pointer_ = nullptr;
    }

    vector<uchar> Texture::GetTexel(size_t x, size_t y) const {
        if (x <= width_ - 1 &&
            y <= height_ - 1)
        {
            size_t index = (x + y * width_) * components_count_;

            return { buffer_pointer_ + index, buffer_pointer_ + index + components_count_ };
        }

        throw std::runtime_error("Wrong Texel Coordinates!");
    }

    void Texture::SaveTexture(uchar *textureBuffer) {
        memcpy(buffer_pointer_, textureBuffer, width_ * height_ * components_count_);
    }

    size_t Texture::GetWidth() const {
        return width_;
    }

    size_t Texture::GetHeight() const {
        return height_;
    }
}
