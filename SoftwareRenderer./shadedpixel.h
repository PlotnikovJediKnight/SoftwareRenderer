#ifndef SHADEDPIXEL_H
#define SHADEDPIXEL_H

#include <glm/vec3.hpp>
#include <array>
#include <vector>
#include <glm/mat4x4.hpp>

namespace pv{

    using InterpolationPoint = glm::vec<3, double>;
    using uchar = unsigned char;
    using ShadeColor = std::array<uchar, 4>;

    struct ShadedPixel {
        InterpolationPoint interpolatedPoint;
        std::array<uchar, 4> shadeColor;
    };

}

#endif // SHADEDPIXEL_H
