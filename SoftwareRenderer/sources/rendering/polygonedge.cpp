#include "headers/rendering/polygonedge.h"
#include <cmath>
using namespace std;

namespace pv{

PolygonEdge::PolygonEdge(float x1, float y1, float x2, float y2)
    : x1(x1), y1(y1), x2(x2), y2(y2) {

    min_x_ = (x1 < x2) ? x1 : x2;
    max_x_ = (x1 > x2) ? x1 : x2;

    min_y_ = (y1 < y2) ? y1 : y2;
    max_y_ = (y1 > y2) ? y1 : y2;
}

std::optional<IntersectionPoint>
PolygonEdge::GetIntersectionWithScanline(float scanlineY) const {

    if (std::abs(y2 - y1) == 0.0){
        return std::nullopt;
    }

    if (std::abs(x2 - x1) == 0.0){
        if (PointIsWithinLineSegment(x2, scanlineY)){
            return std::optional<IntersectionPoint>{{x2, scanlineY}};
        } else {
            return std::nullopt;
        }
    }

    const double k = (y2 - y1) / (x2 - x1);
    const double b = ((y1*x2)-(y2*x1)) / (x2 - x1);

    const double x = (scanlineY - b) / k;

    if (PointIsWithinLineSegment(x, scanlineY)){
        return std::optional<IntersectionPoint>{{static_cast<float>(x), scanlineY}};
    }

    return std::nullopt;
}

bool PolygonEdge::PointIsWithinLineSegment(double x, double y) const {
    constexpr double eps = 0.001;

    return (min_x_ - eps <= x && x <= max_x_ + eps) &&
           (min_y_ - eps <= y && y <= max_y_ + eps);
}

}
