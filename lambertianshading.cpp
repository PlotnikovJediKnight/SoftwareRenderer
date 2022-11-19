#include "shadingmodel.h"
#include <algorithm>
using namespace std;

namespace pv {

    inline std::array<uchar, 4>
    operator*(const std::array<uchar, 4>& color, float value){
        return {255,
                static_cast<uchar>(color[1] * value),
                static_cast<uchar>(color[2] * value),
                static_cast<uchar>(color[3] * value)};
    }

    std::vector<ShadedPixel>
    LambertianShading::GetShadedPixels
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

        std::array<uchar, 4> finalShade{255, 0, 0, 0};

        if (lightSources.size() > 0) {
            unsigned long long r = 0, g = 0, b = 0;

            for (const auto& lightSource : lightSources){
                auto iShade = GetShadeColor(polygon, sceneData, materialColor, *lightSource, model, view);

                r += iShade[1];
                g += iShade[2];
                b += iShade[3];
            }

            r /= lightSources.size(); finalShade[1] = r;
            g /= lightSources.size(); finalShade[2] = g;
            b /= lightSources.size(); finalShade[3] = b;
        }

        for_each(begin(shadedPoints), end(shadedPoints), [&finalShade](ShadedPixel& pixel) { pixel.shadeColor = finalShade; });
        return shadedPoints;
    }

    std::array<uchar, 4>
    LambertianShading::GetShadeColor(
            const Polygon &polygon,
            const SceneData &sceneData,
            std::array<uchar, 4> materialColor,
            const LightSource &lightSource,
            const glm::mat4 &model,
            const glm::mat4 &view) const
    {
        vector<glm::vec3> vertices = GetPolygonVertices(polygon.vertex_indices, sceneData.vertices);
        vector<glm::vec3> vertexNormals = GetPolygonVertexNormals(polygon.normal_indices, sceneData.vertex_normals);

        vector<glm::vec4> vertices4; vertices4.reserve(3);
        for (auto vertex : vertices){
            vertices4.push_back(glm::vec4{vertex, 1.0});
        }

        auto ModelView = view * model;
        for_each(vertices4.begin(),
                 vertices4.end(),
                 [&ModelView](glm::vec4& vertex){
                        vertex = ModelView * vertex;
        });

        auto ModelViewNormals = glm::transpose(glm::inverse(GetMatrix3x3(ModelView)));
        for_each(vertexNormals.begin(),
                 vertexNormals.end(),
                 [&ModelViewNormals](glm::vec3& normal){
                        normal = ModelViewNormals * normal;
        });

        auto lightSourcePositionView = view * glm::vec4(lightSource.GetLightSourcePositionWorld(), 1.0);

        vector<std::array<uchar, 4>> shades; shades.reserve(3);
        size_t normalIndex = 0;
        for (const auto& triangleVertex : vertices4){
            glm::vec3 lightDirection = lightSourcePositionView - triangleVertex;

            lightDirection = glm::normalize(lightDirection);
            glm::vec3 normalVector = glm::normalize(vertexNormals[normalIndex]);

            auto shadeAtVertex = GetAverageMaterialLightColor(materialColor, lightSource.GetLightColor()) *
                    glm::clamp(glm::dot(lightDirection, normalVector),
                       static_cast<float>(0.0),
                       static_cast<float>(1.0));
            shades.push_back(shadeAtVertex);

            ++normalIndex;
        }

        return GetFinalAverageShade(shades);
    }

    std::vector<glm::vec3>
    LambertianShading::GetPolygonVertices(
            const std::vector<int> &vertexIndices,
            const std::vector<glm::vec3> &vertices) const
    {
        vector<glm::vec3> toReturnVertices;
        toReturnVertices.reserve(3);

        for (int vertexIndex : vertexIndices){
            toReturnVertices.push_back(vertices[vertexIndex]);
        }

        return toReturnVertices;
    }

    std::vector<glm::vec3>
    LambertianShading::GetPolygonVertexNormals(
            const std::vector<int> &normalIndices,
            const std::vector<glm::vec3> &normals) const
    {
        vector<glm::vec3> vertexNormals;
        vertexNormals.reserve(3);

        for (int normalIndex : normalIndices){
            vertexNormals.push_back(normals[normalIndex]);
        }

        return vertexNormals;
    }

    glm::mat3
    LambertianShading::GetMatrix3x3(const glm::mat4 &modelView) const
    {
        return { {modelView[0][0], modelView[0][1], modelView[0][2]},
                 {modelView[1][0], modelView[1][1], modelView[1][2]},
            {modelView[2][0], modelView[2][1], modelView[2][2]} };
    }

    std::array<uchar, 4>
    LambertianShading::GetAverageMaterialLightColor(std::array<uchar, 4> materialColor, std::array<uchar, 4> lightColor)
    const
    {
        return {255,
                static_cast<uchar>((materialColor[1] + lightColor[1]) / 2),
                static_cast<uchar>((materialColor[2] + lightColor[2]) / 2),
                static_cast<uchar>((materialColor[3] + lightColor[3]) / 2)};
    }

    std::array<uchar, 4>
    LambertianShading::GetFinalAverageShade(const std::vector<std::array<uchar, 4> > &shades) const
    {
        unsigned long long r = 0, g = 0, b = 0;
        for (size_t shadeIdx = 0; shadeIdx < 3; ++shadeIdx){
            r += shades[shadeIdx][1];
            g += shades[shadeIdx][2];
            b += shades[shadeIdx][3];
        }

        return {255,
                static_cast<uchar>(r / 3),
                static_cast<uchar>(g / 3),
                static_cast<uchar>(b / 3) };
    }

}
