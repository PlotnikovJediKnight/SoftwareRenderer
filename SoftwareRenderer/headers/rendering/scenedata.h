#ifndef PV_SCENEDATA_H
#define PV_SCENEDATA_H

#include <vector>
#include <glm/vec3.hpp>

namespace pv {

    struct Polygon {
        std::vector<int> vertex_indices;
        std::vector<int> texture_indices;
        std::vector<int> normal_indices;
    };

    struct SceneData {
    public:
        SceneData();

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> vertex_textures;
        std::vector<glm::vec3> vertex_normals;
        std::vector<Polygon> polygons;
    };

} // namespace pv

#endif // PV_SCENEDATA_H
