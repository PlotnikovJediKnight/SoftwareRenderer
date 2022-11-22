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

    inline uchar GetByteColorComponentValue(float componentValue) {
        if (componentValue > 1.0) {
            componentValue = 1.0;
        }

        if (componentValue < 0.0) {
            componentValue = 0.0;
        }

        return min(static_cast<uchar>(255),
                   static_cast<uchar>(componentValue * 255));
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

            auto ModelView = view * model;
            auto ModelViewNormal = glm::transpose(glm::inverse(GetMatrix3x3(ModelView)));

            constexpr double MAX_BYTE_VALUE_COLOR = 255.0;

            for (auto& shadedPoint : shadedPoints) {
                float r = 0, g = 0, b = 0;

                for (const auto& lightSource : lightSources) {
                    auto &interpPointView = interpolatedCameraSpacePos[shadedPointIdx];
                    auto &normal = interpolatedVertexNormals[shadedPointIdx];
                    auto lightSourcePositionView = view * glm::vec4(lightSource->GetLightSourcePositionWorld(), 1.0);

                    glm::vec3 lightDirectionView = lightSourcePositionView - glm::vec4(interpPointView, 1.0);
                    glm::vec3 surfaceNormalView = ModelViewNormal * normal;

                    lightDirectionView = glm::normalize(lightDirectionView);
                    surfaceNormalView = glm::normalize(surfaceNormalView);

                    glm::vec3 viewDirection = -glm::normalize(interpPointView);

                    auto diffuseShade = GetDiffuseShade(lightSource, lightDirectionView, surfaceNormalView);
                    {
                        r += diffuseShade[1] / MAX_BYTE_VALUE_COLOR;
                        g += diffuseShade[2] / MAX_BYTE_VALUE_COLOR;
                        b += diffuseShade[3] / MAX_BYTE_VALUE_COLOR;
                    }

                    auto specularShade = GetSpecularShade(lightSource, lightDirectionView, surfaceNormalView, viewDirection);
                    {
                        r += specularShade[1] / MAX_BYTE_VALUE_COLOR;
                        g += specularShade[2] / MAX_BYTE_VALUE_COLOR;
                        b += specularShade[3] / MAX_BYTE_VALUE_COLOR;
                    }
                }

                array<uchar, 4> finalShade;
                auto ambientShade = GetAmbientShade(materialColor, lightSources);
                {
                    r += ambientShade[1] / MAX_BYTE_VALUE_COLOR;
                    g += ambientShade[2] / MAX_BYTE_VALUE_COLOR;
                    b += ambientShade[3] / MAX_BYTE_VALUE_COLOR;
                }

                finalShade = {255, GetByteColorComponentValue(r), GetByteColorComponentValue(g), GetByteColorComponentValue(b) };
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
            std::array<uchar, 4> materialColor,
             const std::vector<std::shared_ptr<LightSource>>& lightSources
     ) const {
        constexpr double AMBIENT_LIGHT_COEFF = 0.22;
        return materialColor * AMBIENT_LIGHT_COEFF;
    }

    std::array<uchar, 4>
    PhongShading::GetDiffuseShade
    (
            const std::shared_ptr<LightSource>& lightSource,
            glm::vec3& lightDirectionView,
            glm::vec3& surfaceNormalView
    ) const {

        constexpr double DIFFUSE_LIGHT_COEFF = 0.35;

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
            glm::vec3& lightDirectionView,
            glm::vec3& surfaceNormalView,
            glm::vec3& viewDirection
    )
    const {

        float dotProductLN = glm::dot(lightDirectionView, surfaceNormalView);
        glm::vec3 reflectanceDirectionView = 2 * dotProductLN * surfaceNormalView - lightDirectionView;

        float saturatedDotProductRV = glm::clamp(glm::dot(reflectanceDirectionView, viewDirection),
                                                 static_cast<float>(0.0),
                                                 static_cast<float>(1.0));

        constexpr double SPECULAR_LIGHT_COEFF = 1.00;

        auto specularShade =
                lightSource->GetLightColor()  * SPECULAR_LIGHT_COEFF *
                pow(saturatedDotProductRV, lightSource->GetReflectancePower());

        return specularShade;

    }

}
