#include "shadingmodel.h"
#include <algorithm>

using namespace std;

namespace pv {

    std::vector<ShadedPixel>
    NoShading::GetShadedPixels
    (
            const ViewportPoint& firstPoint, const ViewportPoint& secondPoint, const ViewportPoint& thirdPoint,
            const Polygon& polygon,
            const SceneData& sceneData,
            std::array<uchar, 4> materialColor,
            const std::vector<std::shared_ptr<LightSource>>& lightSources,
            const glm::mat4 &model, const glm::mat4 &view
    ) const {

        auto shadedPoints =
                ShadingModel::GetShadedPixels
                (
                    firstPoint, secondPoint, thirdPoint,
                    polygon, sceneData, materialColor, lightSources, model, view
                );

        for_each(begin(shadedPoints), end(shadedPoints), [&materialColor](ShadedPixel& pixel) { pixel.shadeColor = materialColor; });

        return shadedPoints;
    }

}
