#ifndef PV_OBJECTFILEPARSER_H
#define PV_OBJECTFILEPARSER_H

#include <QString>
#include "scenedata.h"

namespace pv {

class ObjectFileParser
{
    public:
        ObjectFileParser();

        ObjectFileParser(const ObjectFileParser&) = delete;
        ObjectFileParser(ObjectFileParser&&) = delete;

        ObjectFileParser& operator=(const ObjectFileParser&) = delete;
        ObjectFileParser& operator=(ObjectFileParser&&) = delete;

        SceneData GetSceneDataFromObjectFile(QString filePath);

        void SetDoApplyYZAxesFix(bool applyFix);

    private:
        glm::vec3 GetVertexData(QString);
        glm::vec3 GetTextureData(QString);
        glm::vec3 GetNormalData(QString);

        Polygon GetPolygonData(QString);

        Polygon GetPolygonDataOneDash(QStringList);
        Polygon GetPolygonDataTwoDashes(QStringList);
        Polygon GetPolygonDataDoubleDash(QStringList);

        void DoApplyYZAxesFix(SceneData& sceneData);

        bool do_apply_yz_axes_fix_;
};

} // namespace pv

#endif // PV_OBJECTFILEPARSER_H
