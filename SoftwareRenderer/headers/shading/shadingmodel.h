#ifndef PV_SHADINGMODEL_H
#define PV_SHADINGMODEL_H

#include <glm/vec3.hpp>
#include <array>
#include <vector>
#include <glm/mat4x4.hpp>
#include <memory>
#include "headers/shading/lightsource.h"
#include "headers/rendering/scenedata.h"
#include "headers/rendering/polygonedge.h"
#include "headers/rendering/viewportpolygonmargins.h"
#include "headers/shading/shadedpixel.h"
#include "headers/texturing/texture.h"

namespace pv {

    enum class SHADING_MODEL { NO_SHADING, LAMBERTIAN_SHADING, PHONG_SHADING };

    using TextureHolder = std::unique_ptr<Texture>;
    using ViewportPoint = glm::vec4;
    using InterpolationPoint = glm::vec<3, double>;
    using uchar = unsigned char;

    class ShadingModel {
    public:
        ShadingModel();
        virtual ~ShadingModel() = default;

        //------
        virtual std::vector<ShadedPixel>
        GetShadedPixels
        (
                const ViewportPoint&, const ViewportPoint&, const ViewportPoint&,
                const Polygon&,
                const SceneData&,
                std::array<uchar, 4>,
                const std::vector<std::shared_ptr<LightSource>>&,
                const glm::mat4&, const glm::mat4&
        ) const = 0;
        //------

        void SetDiffuseTexturingEnabled(bool diffuseEnabled);
        void SetNormalTexturingEnabled(bool normalEnabled);
        void SetSpecularTexturingEnabled(bool specularEnabled);

        void SetDiffuseTexture(TextureHolder diffuseTexture);
        void SetNormalTexture(TextureHolder normalTexture);
        void SetSpecularTexture(TextureHolder specularTexture);

    protected:
        mutable bool normal_interpolation_needed_;
        mutable std::vector<glm::vec3>* normal_vectors_ptr_;
        mutable std::vector<glm::vec3>* interpolated_normal_vectors_ptr_;

        mutable bool camera_space_interpolation_needed_;
        mutable std::vector<glm::vec3>* camera_space_pos_ptr_;
        mutable std::vector<glm::vec3>* interpolated_camera_space_pos_ptr_;

        mutable bool texture_coord_interpolation_needed_;
        mutable std::vector<glm::vec3>* texture_coords_ptr_;
        mutable std::vector<glm::vec3>* interpolated_texture_coords_ptr_;


        bool diffuse_texturing_enabled_;
        bool normal_texturing_enabled_;
        bool specular_texturing_enabled_;

        TextureHolder diffuse_texture_;
        TextureHolder normal_texture_;
        TextureHolder specular_texture_;

    private:
        std::vector<InterpolationPoint> GetTriangleInterpolationPoints(const ViewportPoint& firstPoint, const ViewportPoint& secondPoint, const ViewportPoint& thirdPoint) const;
        ViewportPolygonMargins GetViewportTriangleMargins(const ViewportPoint& firstPoint, const ViewportPoint& secondPoint, const ViewportPoint& thirdPoint) const;
        std::vector<PolygonEdge> GetTriangleEdges(const ViewportPoint& firstPoint, const ViewportPoint& secondPoint, const ViewportPoint& thirdPoint) const;
        std::vector<IntersectionPoint> GetIntersectionPoints(const std::vector<PolygonEdge>& polygonEdges, float currScanlineY) const;
        void TryFixThreePointsIntersectionCase(std::vector<IntersectionPoint>&) const;
    };

    class NoShading : public ShadingModel {
    public:
        NoShading() = default;
        virtual ~NoShading() = default;

        //------
        virtual std::vector<ShadedPixel>
        GetShadedPixels
        (
                const ViewportPoint&, const ViewportPoint&, const ViewportPoint&,
                const Polygon&,
                const SceneData&,
                std::array<uchar, 4>,
                const std::vector<std::shared_ptr<LightSource>>&,
                const glm::mat4&, const glm::mat4&
        ) const override;
        //------
    };


    class LambertianShading : public ShadingModel {
    public:
        LambertianShading() = default;
        virtual ~LambertianShading() = default;

        //------
        virtual std::vector<ShadedPixel>
        GetShadedPixels
        (
                const ViewportPoint&, const ViewportPoint&, const ViewportPoint&,
                const Polygon&,
                const SceneData&,
                std::array<uchar, 4>,
                const std::vector<std::shared_ptr<LightSource>>&,
                const glm::mat4&, const glm::mat4&
        ) const override;
        //------

    private:
        std::array<uchar, 4> GetShadeColor(const Polygon& polygon,
                                           const SceneData& sceneData,
                                           std::array<uchar, 4> materialColor,
                                           const LightSource& lightSource,
                                           const glm::mat4 &model,
                                           const glm::mat4 &view) const;

        std::vector<glm::vec3> GetPolygonVertices(const std::vector<int>& vertexIndices, const std::vector<glm::vec3>& vertices) const;
        std::vector<glm::vec3> GetPolygonVertexNormals(const std::vector<int>& normalIndices, const std::vector<glm::vec3>& normals) const;

        glm::mat3 GetMatrix3x3(const glm::mat4& modelView) const;

        std::array<uchar, 4> GetAverageMaterialLightColor(std::array<uchar, 4> materialColor, std::array<uchar, 4> lightColor) const;

        std::array<uchar, 4> GetFinalAverageShade(const std::vector<std::array<uchar, 4>>& shades) const;
    };


    class PhongShading : public ShadingModel {
    public:
        PhongShading() = default;
        virtual ~PhongShading() = default;

        //------
        virtual std::vector<ShadedPixel>
        GetShadedPixels
        (
                const ViewportPoint&, const ViewportPoint&, const ViewportPoint&,
                const Polygon&,
                const SceneData&,
                std::array<uchar, 4>,
                const std::vector<std::shared_ptr<LightSource>>&,
                const glm::mat4&, const glm::mat4&
        ) const override;
        //------

    private:

        std::vector<glm::vec3> GetPolygonVertexNormals(const std::vector<int> &normalIndices,
                                                       const std::vector<glm::vec3> &normals) const;

        std::vector<glm::vec3> GetPolygonVertexTextureCoords(const std::vector<int> &textureIndices,
                                                             const std::vector<glm::vec3> &vertexTextures) const;

        std::vector<glm::vec3> GetPolygonVerticesInCameraSpace(const std::vector<int> &vertexIndices,
                                                               const std::vector<glm::vec3> &vertices,
                                                               const glm::mat4& model,
                                                               const glm::mat4& view) const;

        glm::vec3 GetTextureNormal(glm::vec3&) const;
        float GetTextureSpecular(glm::vec3&) const;

        std::array<uchar, 4> GetAmbientShade(std::array<uchar, 4>) const;
        std::array<uchar, 4> GetTextureAmbientShade(glm::vec3&) const;

        std::array<uchar, 4> GetDiffuseShade(const std::shared_ptr<LightSource>&, glm::vec3&, glm::vec3&) const;
        std::array<uchar, 4> GetSpecularShade(const std::shared_ptr<LightSource>&, glm::vec3&, glm::vec3&, glm::vec3&) const;
    };

} // namespace pv

#endif // PV_SHADINGMODEL_H
