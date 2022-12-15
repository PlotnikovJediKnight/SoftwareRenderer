#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "headers/gui/display.h"
#include "headers/object_file_parser/objectfileparser.h"
#include "headers/rendering/scenedata.h"
#include "headers/rendering/modelstatus.h"
#include "headers/models/lightsourcelistmodel.h"
#include "headers/texture_reader/texturereader.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    using TextureHolder = std::unique_ptr<pv::Texture>;
    using TextureReaderHolder = std::unique_ptr<pv::TextureReader>;
    using LightSourceListModelHolder = std::unique_ptr<pv::LightSourceListModel>;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_openObjFilePushButton_clicked();

    void on_xRotationRadioButton_clicked();

    void on_noAnimationRadioButton_clicked();

    void on_yRotationRadioButton_clicked();

    void on_zRotationRadioButton_clicked();

    void on_carouselRadioButton_clicked();

    void on_fovySlider_valueChanged(int value);

    void on_nearPlaneDistanceSlider_valueChanged(int value);

    void on_farPlaneDistanceSlider_valueChanged(int value);

    void on_drawWorldAxesCheckBox_stateChanged(int arg1);

    void on_drawPolygonMeshCheckBox_stateChanged(int arg1);

    void on_horizontalSlider_valueChanged(int value);

    void on_guiPageSelection_currentIndexChanged(int index);

    void on_changePenColorButton_clicked();

    void on_changeBrushColorButton_clicked();

    void on_rasterizePolygonsCheckBox_stateChanged(int arg1);

    void on_enableZBufferingCheckBox_stateChanged(int arg1);

    void on_modifyMeshButton_clicked();

    void on_enableBackfaceCullingCheckBox_stateChanged(int arg1);

    void on_noShadingRadioButton_clicked();

    void on_lambertianModelRadioButton_clicked();

    void on_phongModelRadioButton_clicked();

    void on_addLightPushButton_clicked();

    void on_removeLightPushButton_clicked();

private:
    Ui::MainWindow *ui_;
    pv::Display *display_;

    pv::ObjectFileParser obj_file_parser_;
    pv::SceneData scene_data_;

    QString color_label_style_sheet_;
    QString model_status_label_style_sheet_;
    // QWidget interface

    void UpdatePenColorLabelStyleSheet(QColor&);
    void UpdateBrushColorLabelStyleSheet(QColor&);

    void UpdateModelStatus();

    void UpdateMeshStatus();
    void UpdateMeshStatusLabelStyleSheet(int statusIndex);
    void UpdateMeshStatusLabelText(int statusIndex);

    void UpdateNormalStatus();
    void UpdateNormalStatusLabelStyleSheet(int statusIndex);
    void UpdateNormalStatusLabelText(int statusIndex);

    void UpdateTextureStatus();
    void UpdateTextureStatusLabelStyleSheet(int statusIndex);
    void UpdateTextureStatusLabelText(int statusIndex);

    QString GetTextureFilePath();

    static const size_t MESH_STATUS_COUNT = 4;
    std::array<QString, MESH_STATUS_COUNT> mesh_status_strings_;
    std::array<QString, MESH_STATUS_COUNT> mesh_status_string_colors_;

    static const size_t NORMAL_STATUS_COUNT = 3;
    std::array<QString, NORMAL_STATUS_COUNT> normal_status_strings_;
    std::array<QString, NORMAL_STATUS_COUNT> normal_status_string_colors_;

    static const size_t TEXTURE_STATUS_COUNT = 3;
    std::array<QString, TEXTURE_STATUS_COUNT> texture_status_strings_;
    std::array<QString, TEXTURE_STATUS_COUNT> texture_status_string_colors_;

    void InitModelStatusData();

    void InitMeshStatusStrings();
    void InitMeshStatusStringColors();

    void InitNormalStatusStrings();
    void InitNormalStatusStringColors();

    void InitTextureStatusStrings();
    void InitTextureStatusStringColors();

    void SplitQuadsIntoTriangles(std::vector<pv::Polygon>& polygons);

    void DoEnableModifyButton(bool enableModifyButton);
    void DoEnableZBufferingButton(bool enableZBufferingButton);

    void DoEnableBackfaceCullingButton(bool enableBackfaceCullingButton);

    void DoEnableLambertianRadioButton(bool enableLambertianRadioButton);
    void DoEnablePhongRadioButton(bool enablePhongRadioButton);

    void DoEnableDiffuseCheckBox(bool enableDiffuseCheckBox);
    void DoEnableDiffuseGroupBox(bool enableDiffuseGroupBox);

    void DoEnableNormalCheckBox(bool enableNormalCheckBox);
    void DoEnableNormalGroupBox(bool enableNormalGroupBox);

    void DoEnableSpecularCheckBox(bool enableSpecularCheckBox);
    void DoEnableSpecularGroupBox(bool enableSpecularGroupBox);

    pv::MeshStatus GetMeshStatus();
    pv::NormalStatus GetNormalStatus();
    pv::TextureStatus GetTextureStatus();

    LightSourceListModelHolder light_source_list_model_holder_;

    TextureHolder diffuse_texture_;
    TextureHolder normal_texture_;
    TextureHolder specular_texture_;

    TextureReaderHolder texture_reader_;

private slots:
    void UpdatedLightSourceListModelSlot(const pv::LightSourceListModel*);

    void on_diffuseTextureChooseBtn_clicked();

    void on_normalTextureChooseBtn_clicked();

    void on_specularTextureChooseBtn_clicked();

    void on_diffuseTextureCheckBox_stateChanged(int state);

    void on_normalTextureCheckBox_stateChanged(int state);

    void on_specularTextureCheckBox_stateChanged(int state);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
};
#endif // MAINWINDOW_H
