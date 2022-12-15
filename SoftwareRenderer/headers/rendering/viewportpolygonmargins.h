#ifndef VIEWPORTPOLYGONMARGINS_H
#define VIEWPORTPOLYGONMARGINS_H

namespace pv{

struct ViewportPolygonMargins
{
    ViewportPolygonMargins() = delete;
    ViewportPolygonMargins(float minY, float maxY);

    float minScanlineY;
    float maxScanlineY;
};

}

#endif // VIEWPORTPOLYGONMARGINS_H
