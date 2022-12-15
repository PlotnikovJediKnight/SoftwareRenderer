#include "headers/object_file_parser/objectfileparser.h"
#include <QFile>
#include <QTextStream>
#include <glm/mat3x3.hpp>

namespace pv {


ObjectFileParser::ObjectFileParser() : do_apply_yz_axes_fix_(false) {}

SceneData ObjectFileParser::GetSceneDataFromObjectFile(QString filePath) {
        SceneData sceneData;

        QFile inputFile(filePath);
        inputFile.open(QIODevice::ReadOnly);
        QTextStream in(&inputFile);

        {
            while (!in.atEnd()){
                QString line = in.readLine();
                if (line.size() != 0) {
                    char firstChar = line[0].toLatin1();

                    if (firstChar == 'v'){
                            char secondChar = line[1].toLatin1();

                            switch (secondChar){
                                case ' ':
                                sceneData.vertices.push_back(GetVertexData(line));
                                    break;
                                case 't':
                                sceneData.vertex_textures.push_back(GetTextureData(line));
                                    break;
                                case 'n':
                                sceneData.vertex_normals.push_back(GetNormalData(line));
                                    break;
                            }

                    }
                        else
                    if (firstChar == 'f')
                    {
                        sceneData.polygons.push_back(GetPolygonData(line));
                    }
                }
            }
        }
        inputFile.close();

        if (do_apply_yz_axes_fix_){
            this->DoApplyYZAxesFix(sceneData);
        }

        return sceneData;
    }

    void ObjectFileParser::SetDoApplyYZAxesFix(bool applyFix) {
        do_apply_yz_axes_fix_ = applyFix;
    }

    glm::vec3 ObjectFileParser::GetVertexData(QString line) {
        QStringList list = line.split(" ", Qt::SkipEmptyParts);

        float x = list.at(1).toFloat();
        float y = list.at(2).toFloat();
        float z = list.at(3).toFloat();

        return {x, y, z};
    }

    glm::vec3 ObjectFileParser::GetTextureData(QString line) {
        QStringList list = line.split(" ", Qt::SkipEmptyParts);

        float u = list.at(1).toFloat();
        float v = 0, w = 0;

        if (list.size() >= 3){
            v = list.at(2).toFloat();
        }

        if (list.size() == 4){
            w = list.at(3).toFloat();
        }

        return {u, v, w};
    }

    glm::vec3 ObjectFileParser::GetNormalData(QString line) {
        QStringList list = line.split(" ", Qt::SkipEmptyParts);

        float i = list.at(1).toFloat();
        float j = list.at(2).toFloat();
        float k = list.at(3).toFloat();

        return {i, j, k};
    }

    Polygon ObjectFileParser::GetPolygonData(QString line) {
        Polygon toReturn;
        QStringList list = line.split(" ", Qt::SkipEmptyParts);
        qsizetype doubleDashCount = list[1].count("//");

        switch (doubleDashCount){
            case 1:{
            toReturn = GetPolygonDataDoubleDash(list);
                break;
            }
            default:{
                qsizetype dashCount = list[1].count("/");
                if (dashCount == 1)
                    toReturn = GetPolygonDataOneDash(list);
                else
                    toReturn = GetPolygonDataTwoDashes(list);
            }
        }

        return toReturn;
    }

    Polygon ObjectFileParser::GetPolygonDataOneDash(QStringList list) {
        Polygon toReturn;
        {
            for (qsizetype itemIndex = 1; itemIndex < list.size(); ++itemIndex){
                QStringList indices = list[itemIndex].split("/", Qt::SkipEmptyParts);

                int vertexIndex = indices[0].toInt(); if (vertexIndex < 0) throw std::runtime_error("One dash - Negative vertex index!");
                vertexIndex--;
                toReturn.vertex_indices.push_back(vertexIndex);

                int textureIndex = indices[1].toInt(); if (textureIndex < 0) throw std::runtime_error("One dash - Negative texture index!");
                textureIndex--;
                toReturn.texture_indices.push_back(textureIndex);
            }
        }
        return toReturn;
    }

    Polygon ObjectFileParser::GetPolygonDataTwoDashes(QStringList list) {
        Polygon toReturn;
        {
            for (qsizetype itemIndex = 1; itemIndex < list.size(); ++itemIndex){
                QStringList indices = list[itemIndex].split("/", Qt::SkipEmptyParts);

                int vertexIndex = indices[0].toInt(); if (vertexIndex < 0) throw std::runtime_error("Two dashes - Negative vertex index!");
                vertexIndex--;
                toReturn.vertex_indices.push_back(vertexIndex);

                int textureIndex = indices[1].toInt(); if (textureIndex < 0) throw std::runtime_error("Two dashes - Negative texture index!");
                textureIndex--;
                toReturn.texture_indices.push_back(textureIndex);

                int normalIndex = indices[2].toInt(); if (normalIndex < 0) throw std::runtime_error("Two dashes - Negative normal index!");
                normalIndex--;
                toReturn.normal_indices.push_back(normalIndex);
            }
        }
        return toReturn;
    }

    Polygon ObjectFileParser::GetPolygonDataDoubleDash(QStringList list) {
        Polygon toReturn;
        {
            for (qsizetype itemIndex = 1; itemIndex < list.size(); ++itemIndex){
                QStringList indices = list[itemIndex].split("//", Qt::SkipEmptyParts);

                int vertexIndex = indices[0].toInt(); if (vertexIndex < 0) throw std::runtime_error("One dash - Negative vertex index!");
                vertexIndex--;
                toReturn.vertex_indices.push_back(vertexIndex);

                int normalIndex = indices[1].toInt(); if (normalIndex < 0) throw std::runtime_error("Two dashes - Negative normal index!");
                normalIndex--;
                toReturn.normal_indices.push_back(normalIndex);
            }
        }
        return toReturn;
    }

    float GetRadianAngle(float degreeAngle) {
        return degreeAngle / 180.0 * M_PI;
    }

    glm::mat3 GetYZAxesFix() {
        constexpr float rotationAngleInDegrees = 90.0;

        float rotationAngleInRadians = GetRadianAngle(rotationAngleInDegrees);

        float cosineValue = cos(rotationAngleInRadians);
        float sineValue = sin(rotationAngleInRadians);

        return glm::mat3(
                        {1, 0, 0},
                        {0, cosineValue, sineValue},
                        {0,  -sineValue, cosineValue});
    }

    void ObjectFileParser::DoApplyYZAxesFix(SceneData &sceneData) {
        auto fixMatrix = GetYZAxesFix();

        for (auto& pointGlmVector : sceneData.vertices){
            auto pointFixed = fixMatrix * pointGlmVector;
            pointGlmVector = pointFixed;
        }

        for (auto& normalGlmVector : sceneData.vertex_normals){
            auto normalFixed = fixMatrix * normalGlmVector;
            normalGlmVector = normalFixed;
        }
    }


} // namespace pv
