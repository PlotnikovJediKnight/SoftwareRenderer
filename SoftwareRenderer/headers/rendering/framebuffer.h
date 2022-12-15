#ifndef BITMAP_H
#define BITMAP_H

#include <cstddef>
#include <vector>
#include <array>

namespace pv {

    using uchar = unsigned char;

    enum class COLOR_MODEL {
        ARGB32
    };

    class FrameBuffer {
    public:
        FrameBuffer(size_t width, size_t height, COLOR_MODEL clrModel);
        ~FrameBuffer();

        void CopyToUcharArray(uchar* extBuffer) const;
        void DrawPixel(size_t x, size_t y, uchar a, uchar r, uchar g, uchar b);
        void DrawPixel(size_t x, size_t y, const std::array<uchar, 4>&  argb);
        void ZBufferDrawPixel(size_t x, size_t y, double z, const std::array<uchar, 4>&  argb);

        void DrawLine(float x1, float y1, float x2, float y2, const std::array<uchar, 4>&  argb);
        void Clear(uchar shade);

        void EnableZBuffer();
        void ClearZBuffer();

        size_t GetWidth() const;
        size_t GetHeight() const;

        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer(FrameBuffer&&) = delete;

        FrameBuffer& operator=(const FrameBuffer&) = delete;
        FrameBuffer& operator=(FrameBuffer&&) = delete;

    private:
        int components_count_;
        uchar* buffer_pointer_;
        size_t width_;
        size_t height_;

        double* depth_buffer_pointer_;

        static int const inline MODEL_COMPONENTS_COUNT[] = { 4 };
    };

}

#endif // BITMAP_H
