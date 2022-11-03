#ifndef PV_LAMBERTIANMODEL_H
#define PV_LAMBERTIANMODEL_H

#include <array>
#include <glm/mat4x4.hpp>
#include <vector>
#include "lightsource.h"
#include "scenedata.h"

namespace pv {

class LambertianModel
{
public:
    using uchar = unsigned char;

    std::array<uchar, 4> GetShadeColor(const Polygon& polygon,
                                       const SceneData& sceneData,
                                       std::array<uchar, 4> materialColor,
                                       const LightSource& lightSource,
                                       glm::mat4 model,
                                       glm::mat4 view);

private:
    std::vector<glm::vec3> GetPolygonVertices(const std::vector<int>& vertexIndices, const std::vector<glm::vec3>& vertices);
    std::vector<glm::vec3> GetPolygonVertexNormals(const std::vector<int>& normalIndices, const std::vector<glm::vec3>& normals);

    glm::mat3 GetMatrix3x3(const glm::mat4& modelView);

    std::array<uchar, 4> GetFinalAverageShade(const std::vector<std::array<uchar, 4>>& shades);
};

} // namespace pv

#endif // PV_LAMBERTIANMODEL_H
