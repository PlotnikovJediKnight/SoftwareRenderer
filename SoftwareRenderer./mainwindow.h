#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "display.h"
#include "objectfileparser.h"
#include "scenedata.h"
#include "modelstatus.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
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

    void on_lightPositionSlider_valueChanged(int value);

    void on_enableLambertianModelCheckBox_stateChanged(int arg1);

private:
    Ui::MainWindow *ui_;
    pv::Display *display;

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

    static const size_t MESH_STATUS_COUNT = 4;
    std::array<QString, MESH_STATUS_COUNT> mesh_status_strings_;
    std::array<QString, MESH_STATUS_COUNT> mesh_status_string_colors_;

    static const size_t NORMAL_STATUS_COUNT = 3;
    std::array<QString, NORMAL_STATUS_COUNT> normal_status_strings_;
    std::array<QString, NORMAL_STATUS_COUNT> normal_status_string_colors_;

    void InitModelStatusData();

    void InitMeshStatusStrings();
    void InitMeshStatusStringColors();

    void InitNormalStatusStrings();
    void InitNormalStatusStringColors();

    void SplitQuadsIntoTriangles(std::vector<pv::Polygon>& polygons);

    void DoEnableModifyButton(bool enableModifyButton);
    void DoEnableZBufferingButton(bool enableZBufferingButton);

    void DoEnableBackfaceCullingButton(bool enableBackfaceCullingButton);
    void DoEnableLambertianModelButton(bool enableLambertianModelButton);

    pv::MeshStatus GetMeshStatus();
    pv::NormalStatus GetNormalStatus();

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
};
#endif // MAINWINDOW_H
