#include "shadingmodel.h"
#include <algorithm>
#include <cmath>
using namespace std;

namespace pv {

    inline std::array<uchar, 4>
    operator*(const std::array<uchar, 4>& color, float value){
        return {255,
                static_cast<uchar>(color[1] * value),
                static_cast<uchar>(color[2] * value),
                static_cast<uchar>(color[3] * value)};
    }

    inline std::array<uchar, 4>
    GetAverageMaterialLightColor(std::array<uchar, 4> materialColor, std::array<uchar, 4> lightColor)
    {
        return {255,
                static_cast<uchar>((materialColor[1] + lightColor[1]) / 2),
                static_cast<uchar>((materialColor[2] + lightColor[2]) / 2),
                static_cast<uchar>((materialColor[3] + lightColor[3]) / 2)};
    }

    inline glm::mat3
    GetMatrix3x3(const glm::mat4 &modelView)
    {
        return { {modelView[0][0], modelView[0][1], modelView[0][2]},
                 {modelView[1][0], modelView[1][1], modelView[1][2]},
                 {modelView[2][0], modelView[2][1], modelView[2][2]} };
    }

    std::vector<ShadedPixel>
    PhongShading::GetShadedPixels
    (
            const ViewportPoint& firstPoint, const ViewportPoint& secondPoint, const ViewportPoint& thirdPoint,
            const Polygon& polygon,
            const SceneData& sceneData,
            std::array<uchar, 4> materialColor,
            const std::vector<std::shared_ptr<LightSource>>& lightSources,
            const glm::mat4 &model, const glm::mat4 &view
    ) const {

        normal_interpolation_needed_ = true;
        vector<glm::vec3> vertexNormals = GetPolygonVertexNormals(polygon.normal_indices, sceneData.vertex_normals);
        normal_vectors_ptr_ = &vertexNormals;

        vector<glm::vec3> interpolatedVertexNormals;
        interpolated_normal_vectors_ptr_ = &interpolatedVertexNormals;

        camera_space_interpolation_needed_ = true;
        vector<glm::vec3> cameraSpacePositions = GetPolygonVerticesInCameraSpace(polygon.vertex_indices, sceneData.vertices, model, view);
        camera_space_pos_ptr_ = &cameraSpacePositions;

        vector<glm::vec3> interpolatedCameraSpacePos;
        interpolated_camera_space_pos_ptr_ = &interpolatedCameraSpacePos;

        auto shadedPoints =
                ShadingModel::GetShadedPixels
                (
                    firstPoint, secondPoint, thirdPoint,
                    polygon, sceneData, materialColor, lightSources, model, view
                );

        if (lightSources.size() > 0) {

            size_t shadedPointIdx = 0;
            auto ambientShade = GetAmbientShade(materialColor);

            auto ModelView = view * model;
            auto ModelViewNormal = glm::transpose(glm::inverse(GetMatrix3x3(ModelView)));

            for (auto& shadedPoint : shadedPoints) {
                unsigned long long r = 0, g = 0, b = 0;

                for (const auto& lightSource : lightSources) {
                    auto diffuseShade = GetDiffuseShade(lightSource,
                                                        interpolatedVertexNormals[shadedPointIdx],
                                                        interpolatedCameraSpacePos[shadedPointIdx],
                                                        ModelViewNormal, view);

                    auto specularShade = GetSpecularShade(lightSource,
                                                          interpolatedVertexNormals[shadedPointIdx],
                                                          interpolatedCameraSpacePos[shadedPointIdx],
                                                          ModelViewNormal, view);

                    r += diffuseShade[1] + specularShade[1];
                    g += diffuseShade[2] + specularShade[2];
                    b += diffuseShade[3] + specularShade[3];
                }

                r += ambientShade[1];
                g += ambientShade[2];
                b += ambientShade[3];

                r /= lightSources.size() * 2 + 1;
                g /= lightSources.size() * 2 + 1;
                b /= lightSources.size() * 2 + 1;

                array<uchar, 4> finalShade = {255, static_cast<uchar>(r), static_cast<uchar>(g), static_cast<uchar>(b)};
                shadedPoint.shadeColor = finalShade;

                shadedPointIdx++;
            }

        } else {
            std::array<uchar, 4> blackShade{255, 0, 0, 0};
            for_each(begin(shadedPoints), end(shadedPoints), [&blackShade](ShadedPixel& pixel) { pixel.shadeColor = blackShade; });
        }
        return shadedPoints;

    }

    std::vector<glm::vec3>
    PhongShading::GetPolygonVertexNormals
    (
            const std::vector<int> &normalIndices,
            const std::vector<glm::vec3> &normals
    ) const {

        vector<glm::vec3> vertexNormals;
        vertexNormals.reserve(3);

        for (int normalIndex : normalIndices){
            vertexNormals.push_back(normals[normalIndex]);
        }

        return vertexNormals;

    }

    std::vector<glm::vec3>
    PhongShading::GetPolygonVerticesInCameraSpace
    (
            const std::vector<int> &vertexIndices,
            const std::vector<glm::vec3> &vertices,
            const glm::mat4& model,
            const glm::mat4& view
    ) const
    {
        glm::mat4 MV = view * model;

        vector<glm::vec3> verticesInCameraSpace;
        verticesInCameraSpace.reserve(3);

        for (int vertexIndex : vertexIndices){
            verticesInCameraSpace.push_back(glm::vec3
                                            (
                                                MV * glm::vec4(vertices[vertexIndex], 1)
                                            ));
        }

        return verticesInCameraSpace;
    }

    std::array<uchar, 4>
    PhongShading::GetAmbientShade
    (
            std::array<uchar, 4> materialColor
     ) const {
        constexpr double AMBIENT_LIGHT_COEFF = 0.35;
        return materialColor * AMBIENT_LIGHT_COEFF;
    }

    std::array<uchar, 4>
    PhongShading::GetDiffuseShade
    (
            const std::shared_ptr<LightSource>& lightSource,
            glm::vec3& normal,
            glm::vec3& interpPointView,
            const glm::mat3& modelViewNormal,
            const glm::mat4& view
    ) const {

        auto lightSourcePositionView = view * glm::vec4(lightSource->GetLightSourcePositionWorld(), 1.0);

        glm::vec3 lightDirectionView = lightSourcePositionView - glm::vec4(interpPointView, 1.0);
        glm::vec3 surfaceNormalView = modelViewNormal * normal;

        lightDirectionView = glm::normalize(lightDirectionView);
        surfaceNormalView = glm::normalize(surfaceNormalView);

        constexpr double DIFFUSE_LIGHT_COEFF = 0.95;

        auto diffuseShade =
                lightSource->GetLightColor() * DIFFUSE_LIGHT_COEFF *
                glm::clamp(glm::dot(lightDirectionView, surfaceNormalView),
                   static_cast<float>(0.0),
                   static_cast<float>(1.0));

        return diffuseShade;
    }

    std::array<uchar, 4>
    PhongShading::GetSpecularShade
    (
            const std::shared_ptr<LightSource>& lightSource,
            glm::vec3& normal,
            glm::vec3& interpPointView,
            const glm::mat3& modelViewNormal,
            const glm::mat4& view
    )
    const {

        auto lightSourcePositionView = view * glm::vec4(lightSource->GetLightSourcePositionWorld(), 1.0);

        glm::vec3 lightDirectionView = lightSourcePositionView - glm::vec4(interpPointView, 1.0);
        glm::vec3 surfaceNormalView = modelViewNormal * normal;

        lightDirectionView = glm::normalize(lightDirectionView);
        surfaceNormalView = glm::normalize(surfaceNormalView);

        float dotProductLN = glm::dot(lightDirectionView, surfaceNormalView);

        glm::vec3 reflectanceDirectionView = 2 * dotProductLN * surfaceNormalView - lightDirectionView;
        glm::vec3 viewDirection = -glm::normalize(interpPointView);

        float saturatedDotProductRV = glm::clamp(glm::dot(reflectanceDirectionView, viewDirection),
                                                 static_cast<float>(0.0),
                                                 static_cast<float>(1.0));

        constexpr double SPECULAR_LIGHT_COEFF = 1.00;

        array<uchar, 4> whiteSpecular = {255, 255, 255, 255};

        auto specularShade =
                whiteSpecular * SPECULAR_LIGHT_COEFF *
                pow(saturatedDotProductRV, lightSource->GetReflectancePower());

        return specularShade;

    }

}
