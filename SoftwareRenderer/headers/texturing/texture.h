#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstddef>
#include <vector>

namespace pv {

    using uchar = unsigned char;

    enum class TEXTURE_COLOR_MODEL {
        RGB24, MONO8
    };

    class Texture {
    public:
        Texture(size_t width, size_t height, TEXTURE_COLOR_MODEL textClrModel);
        ~Texture();

        std::vector<uchar> GetTexel(size_t x, size_t y) const;

        void SaveTexture(uchar* textureBuffer);
        size_t GetWidth() const;
        size_t GetHeight() const;

        Texture(const Texture&) = delete;
        Texture(Texture&&) = delete;

        Texture& operator=(const Texture&) = delete;
        Texture& operator=(Texture&&) = delete;

    private:
        int components_count_;
        uchar* buffer_pointer_;
        size_t width_;
        size_t height_;

        static int const inline TEXT_MODEL_COMPONENTS_COUNT[] = { 3, 1 };
    };

}

#endif // BMPREADER_H
