#include "headers/shading/shadingmodel.h"
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

        //=====================================================
        normal_interpolation_needed_ = true;
        vector<glm::vec3> vertexNormals = GetPolygonVertexNormals(polygon.normal_indices, sceneData.vertex_normals);
        normal_vectors_ptr_ = &vertexNormals;

        vector<glm::vec3> interpolatedVertexNormals;
        interpolated_normal_vectors_ptr_ = &interpolatedVertexNormals;
        //=====================================================

        //=====================================================
        camera_space_interpolation_needed_ = true;
        vector<glm::vec3> cameraSpacePositions = GetPolygonVerticesInCameraSpace(polygon.vertex_indices, sceneData.vertices, model, view);
        camera_space_pos_ptr_ = &cameraSpacePositions;

        vector<glm::vec3> interpolatedCameraSpacePos;
        interpolated_camera_space_pos_ptr_ = &interpolatedCameraSpacePos;
        //=====================================================

        //=====================================================
        if (diffuse_texturing_enabled_ ||
            normal_texturing_enabled_  ||
            specular_texturing_enabled_) { texture_coord_interpolation_needed_ = true; }

        vector<glm::vec3> vertexTexturesCoords = GetPolygonVertexTextureCoords(polygon.texture_indices, sceneData.vertex_textures);
        texture_coords_ptr_ = &vertexTexturesCoords;

        vector<glm::vec3> interpolatedTextureCoords;
        interpolated_texture_coords_ptr_ = &interpolatedTextureCoords;
        //=====================================================

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
                    glm::vec3 normal;
                    if (normal_texture_ && normal_texturing_enabled_) {
                        auto &interpTextCoord = interpolatedTextureCoords[shadedPointIdx];
                        normal = GetTextureNormal(interpTextCoord);
                    }
                    else { normal = interpolatedVertexNormals[shadedPointIdx]; }

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

                    float specularShadeCoefficient = 1.0;
                    if (specular_texture_ && specular_texturing_enabled_) {
                        auto &interpTextCoord = interpolatedTextureCoords[shadedPointIdx];
                        specularShadeCoefficient = GetTextureSpecular(interpTextCoord);
                    }
                    auto specularShade =
                            GetSpecularShade(lightSource, lightDirectionView, surfaceNormalView, viewDirection) * specularShadeCoefficient;
                    {
                        r += specularShade[1] / MAX_BYTE_VALUE_COLOR;
                        g += specularShade[2] / MAX_BYTE_VALUE_COLOR;
                        b += specularShade[3] / MAX_BYTE_VALUE_COLOR;
                    }
                }

                array<uchar, 4> finalShade;
                array<uchar, 4> ambientShade;
                if (diffuse_texture_ && diffuse_texturing_enabled_){
                    auto &interpTextCoord = interpolatedTextureCoords[shadedPointIdx];
                    ambientShade = GetTextureAmbientShade(interpTextCoord);
                } else {
                    ambientShade = GetAmbientShade(materialColor);
                }

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
    PhongShading::GetPolygonVertexTextureCoords
    (
            const std::vector<int> &textureIndices,
            const std::vector<glm::vec3> &vertexTextures
    ) const
    {
        vector<glm::vec3> vertexTextureCoords;
        vertexTextureCoords.reserve(3);

        for (int textureIndex : textureIndices){
            vertexTextureCoords.push_back(vertexTextures[textureIndex]);
        }

        return vertexTextureCoords;
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

    glm::vec3 PhongShading::GetTextureNormal(glm::vec3 &interpTextCoord) const {
        float x = interpTextCoord[0]; size_t width = normal_texture_->GetWidth() - 1;
        float y = interpTextCoord[1]; size_t height = normal_texture_->GetHeight() - 1;

        auto texel = normal_texture_->GetTexel(x * width, y * height);
        std::swap(texel.at(0), texel.at(2));

        glm::vec3 texelNormal { texel.at(0), texel.at(1), texel.at(2) };

        return texelNormal;
    }

    float PhongShading::GetTextureSpecular(glm::vec3 &interpTextCoord) const {
        float x = interpTextCoord[0]; size_t width = specular_texture_->GetWidth() - 1;
        float y = interpTextCoord[1]; size_t height = specular_texture_->GetHeight() - 1;

        auto texel = specular_texture_->GetTexel(x * width, y * height);

        return texel[0] / 255.0;
    }

    std::array<uchar, 4>
    PhongShading::GetAmbientShade(std::array<uchar, 4> materialColor) const {
        constexpr double AMBIENT_LIGHT_COEFF = 0.22;
        return materialColor * AMBIENT_LIGHT_COEFF;
    }

    std::array<uchar, 4>
    PhongShading::GetTextureAmbientShade(glm::vec3& interpTextCoord) const {
        constexpr double AMBIENT_LIGHT_COEFF = 0.95;

        float x = interpTextCoord[0]; size_t width = diffuse_texture_->GetWidth() - 1;
        float y = interpTextCoord[1]; size_t height = diffuse_texture_->GetHeight() - 1;

        auto texel = diffuse_texture_->GetTexel(x * width, y * height);
        std::swap(texel.at(0), texel.at(2));

        std::array<uchar, 4> texelColor { 255, texel.at(0), texel.at(1), texel.at(2) };

        return texelColor * AMBIENT_LIGHT_COEFF;
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
        reflectanceDirectionView = glm::normalize(reflectanceDirectionView);

        float saturatedDotProductRV = glm::clamp(glm::dot(reflectanceDirectionView, viewDirection),
                                                 static_cast<float>(0.0),
                                                 static_cast<float>(1.0));

        constexpr double SPECULAR_LIGHT_COEFF = 1.00;

        auto specularShade =
                lightSource->GetLightColor()  * SPECULAR_LIGHT_COEFF *
                pow(saturatedDotProductRV, lightSource->GetSpecularPower());

        return specularShade;

    }

}
