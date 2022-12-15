#include "headers/rendering/framebuffer.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdexcept>

namespace pv {

    FrameBuffer::FrameBuffer(size_t width, size_t height, COLOR_MODEL clrModel) :
        components_count_(MODEL_COMPONENTS_COUNT[static_cast<int>(clrModel)]),
        buffer_pointer_(new uchar[width * height * components_count_]),
        width_(width),
        height_(height),
        depth_buffer_pointer_(nullptr){ }

    FrameBuffer::~FrameBuffer() {
        delete[] buffer_pointer_;
        buffer_pointer_ = nullptr;

        if (depth_buffer_pointer_){
            delete[] depth_buffer_pointer_;
            depth_buffer_pointer_ = nullptr;
        }
    }

    void FrameBuffer::CopyToUcharArray(uchar *extBuffer) const {
        std::copy(buffer_pointer_,
                  buffer_pointer_ + width_ * height_ * components_count_,
                  extBuffer);
    }

    void FrameBuffer::DrawPixel(size_t x, size_t y, uchar a, uchar r, uchar g, uchar b) {
        if (x <= width_ - 1 &&
            y <= height_ - 1)
        {
            size_t index = (x + y * width_) * components_count_;

            buffer_pointer_[index    ] = b;
            buffer_pointer_[index + 1] = g;
            buffer_pointer_[index + 2] = r;
            buffer_pointer_[index + 3] = a;
        }
    }

    void FrameBuffer::DrawPixel(size_t x, size_t y, const std::array<uchar, 4>& argb) {
        if (x <= width_ - 1 &&
            y <= height_ - 1)
        {
            size_t index = (x + y * width_) * components_count_;

            buffer_pointer_[index    ] = argb[3];
            buffer_pointer_[index + 1] = argb[2];
            buffer_pointer_[index + 2] = argb[1];
            buffer_pointer_[index + 3] = argb[0];
        }
    }

    void FrameBuffer::ZBufferDrawPixel(size_t x, size_t y, double z, const std::array<uchar, 4> &argb) {
        if (x <= width_ - 1 &&
            y <= height_ - 1)
        {
            size_t primaryBufferIndex = (x + y * width_) * components_count_;
            size_t depthBufferIndex = (x + y * width_);

            if (z < depth_buffer_pointer_[depthBufferIndex]){
                depth_buffer_pointer_[depthBufferIndex] = z;

                buffer_pointer_[primaryBufferIndex    ] = argb[3];
                buffer_pointer_[primaryBufferIndex + 1] = argb[2];
                buffer_pointer_[primaryBufferIndex + 2] = argb[1];
                buffer_pointer_[primaryBufferIndex + 3] = argb[0];
            }
        }
    }

    void FrameBuffer::DrawLine(float x1, float y1, float x2, float y2, const std::array<uchar, 4> &argb) {
        size_t integerX1 = x1, integerY1 = y1, integerX2 = x2, integerY2 = y2;
        int deltaX = integerX2 - integerX1;
        int deltaY = integerY2 - integerY1;

        size_t rasterizationSteps = abs(deltaX) > abs(deltaY) ? abs(deltaX) : abs(deltaY);
        rasterizationSteps++;

        float x = x1;
        float y = y1;

        float xAccretion = deltaX / static_cast<float>(rasterizationSteps);
        float yAccretion = deltaY / static_cast<float>(rasterizationSteps);

        for (size_t step = 1; step <= rasterizationSteps; ++step){
            x += xAccretion;
            y += yAccretion;
            DrawPixel(round(x), round(y), argb);
        }
    }

    void FrameBuffer::Clear(uchar shade) {
        std::fill(buffer_pointer_,
                  buffer_pointer_ + width_ * height_ * components_count_,
                  shade);

        constexpr int alphaStride = 4;
        const size_t componentsTotal = width_ * height_ * components_count_;

        for (size_t i = 3; i < componentsTotal; i += alphaStride){
            buffer_pointer_[i] = 0xFF;
        }
    }

    void FrameBuffer::ClearZBuffer(){
        if (depth_buffer_pointer_){
            std::fill(depth_buffer_pointer_,
                      depth_buffer_pointer_ + width_ * height_,
                      std::numeric_limits<double>::max());
        } else{
            throw std::runtime_error("Z buffer is not enabled!");
        }
    }

    void FrameBuffer::EnableZBuffer() {
        if (depth_buffer_pointer_ == nullptr){
            depth_buffer_pointer_ = new double[width_ * height_];
        } else{
            throw std::runtime_error("Z buffer already enabled!");
        }
    }

    size_t FrameBuffer::GetWidth() const {
        return width_;
    }

    size_t FrameBuffer::GetHeight() const {
        return height_;
    }
}
