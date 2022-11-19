#ifndef POLYGONEDGE_H
#define POLYGONEDGE_H

#include "viewportpolygonmargins.h"
#include <optional>

namespace pv{

    struct IntersectionPoint{
        float x;
        float y;
    };

    struct PolygonEdge {
        PolygonEdge(float x1, float y1, float x2, float y2);

        std::optional<IntersectionPoint> GetIntersectionWithScanline(float scanlineY) const;

        float x1, y1;
        float x2, y2;

    private:

        bool PointIsWithinLineSegment(double x, double y) const;

        float min_x_;
        float max_x_;

        float min_y_;
        float max_y_;
    };
}

#endif // POLYGONEDGE_H
