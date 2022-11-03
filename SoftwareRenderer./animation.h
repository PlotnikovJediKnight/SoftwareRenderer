#ifndef PV_ANIMATION_H
#define PV_ANIMATION_H

#include <glm/mat4x4.hpp>

namespace pv {

    enum class ANIMATION_TYPE { NO_ANIMATION, X_ROTATION, Y_ROTATION, Z_ROTATION, CAROUSEL };

    class Animation {
    public:
        float GetRadianAngle(float degreeAngle) const;

        Animation() = default;
        virtual glm::mat4 GetModelMatrix() = 0;

        virtual ~Animation() = default;
    };

    class NoAnimation : public Animation {
    public:
        NoAnimation() = default;
        virtual glm::mat4 GetModelMatrix() override;

        virtual ~NoAnimation() = default;
    };

    class XAnimation : public Animation {
    public:
        XAnimation() = default;
        virtual glm::mat4 GetModelMatrix() override;

        virtual ~XAnimation() = default;
    };

    class YAnimation : public Animation {
    public:
        YAnimation() = default;
        virtual glm::mat4 GetModelMatrix() override;

        virtual ~YAnimation() = default;
    };

    class ZAnimation : public Animation {
    public:
        ZAnimation() = default;
        virtual glm::mat4 GetModelMatrix() override;

        virtual ~ZAnimation() = default;
    };

    class CarouselAnimation : public Animation {
    public:
        CarouselAnimation() = default;
        virtual glm::mat4 GetModelMatrix() override;

        virtual ~CarouselAnimation() = default;
    };

} // namespace pv

#endif // PV_ANIMATION_H
