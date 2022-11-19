#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QColorDialog>
#include <QColor>
#include "mainwindow.h"
#include "qevent.h"
#include "ui_mainwindow.h"
#include "display.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui_(new Ui::MainWindow) {

    ui_->setupUi(this);

    constexpr size_t width = 800;
    constexpr size_t height = 600;

    display_ = new pv::Display(width, height, scene_data_);
    ui_->verticalLayout->addWidget(display_);

    ui_->guiPageSelection->addItem("Matrix Transforms");
    ui_->guiPageSelection->addItem("Drawing");
    ui_->guiPageSelection->addItem("Shading");

    obj_file_parser_.SetDoApplyYZAxesFix(true);

    color_label_style_sheet_ =
            R"(
                    background-color: rgb(%1, %2, %3);
                    border: 1px solid black
               )";

    model_status_label_style_sheet_ =
            R"(
                    background-color: rgb(200, 200, 200);
                    color: rgb(%1);
                    border: 1px solid black;
                    font-weight: bold
               )";

    //Light Source Custom List
    {
        using namespace pv;
        light_source_list_model_holder_ =
                std::make_unique<LightSourceListModel>(ui_->lightSourceListView);

        connect(light_source_list_model_holder_.get(),
                &LightSourceListModel::LightSourceListModelUpdated,
                this,
                &MainWindow::UpdatedLightSourceListModelSlot);

        light_source_list_model_holder_->AppendLightSource(std::make_shared<LightSource>());
        ui_->lightSourceListView->setModel(light_source_list_model_holder_.get());
    }

    InitModelStatusData();
}

MainWindow::~MainWindow()
{
    delete ui_;
    delete display_;
}

void MainWindow::on_openObjFilePushButton_clicked()
{
    QString objFilter = tr("Object Files (*.obj)");
    QString filePath = QFileDialog::getOpenFileName(
                this,
                "Read Object File Data",
                QString(),
                objFilter,
                &objFilter);

    scene_data_ = obj_file_parser_.GetSceneDataFromObjectFile(filePath);
    UpdateModelStatus();
}

void MainWindow::on_noAnimationRadioButton_clicked() {
   display_->DeferAnimationType(pv::ANIMATION_TYPE::NO_ANIMATION);
}

void MainWindow::on_xRotationRadioButton_clicked() {
    display_->DeferAnimationType(pv::ANIMATION_TYPE::X_ROTATION);
}

void MainWindow::on_yRotationRadioButton_clicked() {
    display_->DeferAnimationType(pv::ANIMATION_TYPE::Y_ROTATION);
}

void MainWindow::on_zRotationRadioButton_clicked() {
    display_->DeferAnimationType(pv::ANIMATION_TYPE::Z_ROTATION);
}

void MainWindow::on_carouselRadioButton_clicked() {
    display_->DeferAnimationType(pv::ANIMATION_TYPE::CAROUSEL);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    int keyPressed = event->key();
    switch (keyPressed){
        case  Qt::Key_X:
        display_->DeferXCameraView();
            break;
        case  Qt::Key_Y:
        display_->DeferYCameraView();
            break;
        case  Qt::Key_Z:
        display_->DeferZCameraView();
            break;
    }
}


void MainWindow::on_fovySlider_valueChanged(int value) {
    ui_->fovyDegreesValueLabel->setText(QString::number(value));
    display_->DeferNewFovYAngleValue(static_cast<float>(value));
}


void MainWindow::on_nearPlaneDistanceSlider_valueChanged(int value) {
    int farPlaneDistanceValue = ui_->farPlaneDistanceSlider->value();
    if (value >= farPlaneDistanceValue){
        ui_->nearPlaneDistanceSlider->setValue(farPlaneDistanceValue - 1);
        return;
    }

    ui_->nearPlaneDistanceLabel->setText(QString::number(value));
    display_->DeferNewNearPlaneDistance(static_cast<float>(value));
}


void MainWindow::on_farPlaneDistanceSlider_valueChanged(int value) {
    int nearPlaneDistanceValue = ui_->nearPlaneDistanceSlider->value();
    if (value <= nearPlaneDistanceValue){
        ui_->farPlaneDistanceSlider->setValue(nearPlaneDistanceValue + 1);
        return;
    }

    ui_->farPlaneDistanceLabel->setText(QString::number(value));
    display_->DeferNewFarPlaneDistance(static_cast<float>(value));
}


void MainWindow::on_drawWorldAxesCheckBox_stateChanged(int stateValue) {
    switch (stateValue){
        case Qt::Unchecked:
        display_->DeferDrawWorldAxis(false);
            break;
        case Qt::Checked:
        display_->DeferDrawWorldAxis(true);
            break;
        default:
        display_->DeferDrawWorldAxis(false);
    }
}


void MainWindow::on_drawPolygonMeshCheckBox_stateChanged(int stateValue) {
    switch (stateValue){
        case Qt::Unchecked:
        display_->DeferDrawPolygonMesh(false);
            break;
        case Qt::Checked:
        display_->DeferDrawPolygonMesh(true);
            break;
        default:
        display_->DeferDrawPolygonMesh(false);
    }
}


void MainWindow::on_horizontalSlider_valueChanged(int value) {
    ui_->orbitCameraDistanceLabel->setText(QString::number(value));
    display_->DeferNewOrbitCameraDistance(static_cast<float>(value));
}


void MainWindow::on_guiPageSelection_currentIndexChanged(int index) {
    ui_->stackedWidget->setCurrentIndex(index);
    this->setFocus();
}


void MainWindow::on_changePenColorButton_clicked() {
    QColor penColor = QColorDialog::getColor(Qt::white, this, "Choose Pen Color");
    if (penColor.isValid()){
        display_->DeferNewPenColor(penColor);
        UpdatePenColorLabelStyleSheet(penColor);
    }
}

void MainWindow::UpdatePenColorLabelStyleSheet(QColor& penColor) {
    QString filledInStyleSheet =
            this->color_label_style_sheet_
            .arg(QString::number(penColor.red()),
                 QString::number(penColor.green()),
                 QString::number(penColor.blue()));

    ui_->penColorLabel->setStyleSheet( filledInStyleSheet );
}

void MainWindow::on_changeBrushColorButton_clicked() {
    QColor brushColor = QColorDialog::getColor(Qt::white, this, "Choose Brush Color");
    if (brushColor.isValid()){
        display_->DeferNewBrushColor(brushColor);
        UpdateBrushColorLabelStyleSheet(brushColor);
    }
}

void MainWindow::UpdateBrushColorLabelStyleSheet(QColor& brushColor) {
    QString filledInStyleSheet =
            this->color_label_style_sheet_
            .arg(QString::number(brushColor.red()),
                 QString::number(brushColor.green()),
                 QString::number(brushColor.blue()));

    ui_->brushColorLabel->setStyleSheet( filledInStyleSheet );
}

void MainWindow::UpdateModelStatus() {
    UpdateMeshStatus();
    UpdateNormalStatus();
}

void MainWindow::UpdateMeshStatus() {
    using namespace pv;
    MeshStatus meshStatus = GetMeshStatus();

    int statusIndex = static_cast<int>(meshStatus) - static_cast<int>(MeshStatus::NO_MODEL);

    UpdateMeshStatusLabelStyleSheet(statusIndex);
    UpdateMeshStatusLabelText(statusIndex);

    switch (meshStatus){
    case MeshStatus::NO_MODEL:{
        DoEnableModifyButton(false);
        DoEnableZBufferingButton(false);
        ui_->enableZBufferingCheckBox->setChecked(false);
        break;
    }
    case MeshStatus::TRIANGLES_ONLY:{
        DoEnableModifyButton(false);
        DoEnableZBufferingButton(true);
        break;
    }
    case MeshStatus::CONVERTIBLE_TO_TRIANGLES:{
        DoEnableModifyButton(true);
        DoEnableZBufferingButton(false);
        ui_->enableZBufferingCheckBox->setChecked(false);
        break;
    }
    case MeshStatus::NON_CONVERTIBLE_TO_TRIANGLES:{
        DoEnableModifyButton(false);
        DoEnableZBufferingButton(false);
        ui_->enableZBufferingCheckBox->setChecked(false);
        break;
    }
    }
}

void MainWindow::UpdateMeshStatusLabelStyleSheet(int statusIndex) {
    ui_->meshStatusLabel->setStyleSheet(
                    model_status_label_style_sheet_.arg(mesh_status_string_colors_[statusIndex])
                );
}

void MainWindow::UpdateMeshStatusLabelText(int statusIndex) {
    ui_->meshStatusLabel->setText(mesh_status_strings_[statusIndex]);
}

void MainWindow::UpdateNormalStatus() {
    using namespace pv;
    NormalStatus normalStatus = GetNormalStatus();

    int statusIndex = static_cast<int>(normalStatus) - static_cast<int>(NormalStatus::NO_MODEL);

    UpdateNormalStatusLabelStyleSheet(statusIndex);
    UpdateNormalStatusLabelText(statusIndex);

    switch (normalStatus){
    case NormalStatus::NO_MODEL:{
        DoEnableLambertianRadioButton(false);
        DoEnablePhongRadioButton(false);
        ui_->lambertianModelRadioButton->setChecked(false);
        ui_->phongModelRadioButton->setChecked(false);
        break;
    }
    case NormalStatus::NORMALS_PROVIDED:{
        DoEnableLambertianRadioButton(true);
        DoEnablePhongRadioButton(true);
        break;
    }
    case NormalStatus::NO_NORMALS_PROVIDED:{
        DoEnableLambertianRadioButton(false);
        DoEnablePhongRadioButton(false);
        ui_->lambertianModelRadioButton->setChecked(false);
        ui_->phongModelRadioButton->setChecked(false);
        break;
    }
    }
}

void MainWindow::UpdateNormalStatusLabelStyleSheet(int statusIndex) {
    ui_->normalStatusLabel->setStyleSheet(
                    model_status_label_style_sheet_.arg(normal_status_string_colors_[statusIndex])
                );
}

void MainWindow::UpdateNormalStatusLabelText(int statusIndex) {
    ui_->normalStatusLabel->setText(normal_status_strings_[statusIndex]);
}

void MainWindow::InitModelStatusData(){
    {
        InitMeshStatusStrings();
        InitMeshStatusStringColors();
        ui_->meshStatusLabel->setText(mesh_status_strings_[0]);
    }
    {
        InitNormalStatusStrings();
        InitNormalStatusStringColors();
        ui_->normalStatusLabel->setText(normal_status_strings_[0]);
    }
}

void MainWindow::InitMeshStatusStrings() {
    mesh_status_strings_[0] = "No model chosen yet";
    mesh_status_strings_[1] = "Model is made up of triangles only";
    mesh_status_strings_[2] = "Model can be modified to be made up of triangles only";
    mesh_status_strings_[3] = "Model cannot be modified to be made up of triangles only";
}

void MainWindow::InitMeshStatusStringColors() {
    mesh_status_string_colors_[0] = "0, 0, 0";
    mesh_status_string_colors_[1] = "0, 100, 0";
    mesh_status_string_colors_[2] = "239, 245, 69";
    mesh_status_string_colors_[3] = "255, 0, 0";
}

void MainWindow::InitNormalStatusStrings() {
    normal_status_strings_[0] = "No model chosen yet";
    normal_status_strings_[1] = "Normals provided for this model";
    normal_status_strings_[2] = "No normals provided for this model";
}

void MainWindow::InitNormalStatusStringColors() {
    normal_status_string_colors_[0] = "0, 0, 0";
    normal_status_string_colors_[1] = "0, 100, 0";
    normal_status_string_colors_[2] = "255, 0, 0";
}

void MainWindow::SplitQuadsIntoTriangles(vector<pv::Polygon> &polygons) {
    vector<pv::Polygon> newTriangles;

    for (auto& polygon : polygons){
        if (polygon.vertex_indices.size() == 4){
            pv::Polygon newTriangle;

            {
                int lastVertexIndex = polygon.vertex_indices.back();
                polygon.vertex_indices.pop_back();

                copy(polygon.vertex_indices.begin(),
                         polygon.vertex_indices.end(),
                         back_inserter(newTriangle.vertex_indices));

                newTriangle.vertex_indices.at(1) = lastVertexIndex;
                std::swap(newTriangle.vertex_indices.at(0), newTriangle.vertex_indices.at(2));
            }

            if (polygon.texture_indices.size() == 4){
                int lastTextureIndex = polygon.texture_indices.back();
                polygon.texture_indices.pop_back();

                copy(polygon.texture_indices.begin(),
                     polygon.texture_indices.end(),
                     back_inserter(newTriangle.texture_indices));

                newTriangle.texture_indices.at(1) = lastTextureIndex;
                std::swap(newTriangle.texture_indices.at(0), newTriangle.texture_indices.at(2));
            }

            if (polygon.normal_indices.size() == 4){
                int lastNormalIndex = polygon.normal_indices.back();
                polygon.normal_indices.pop_back();

                copy(polygon.normal_indices.begin(),
                     polygon.normal_indices.end(),
                     back_inserter(newTriangle.normal_indices));

                newTriangle.normal_indices.at(1) = lastNormalIndex;
                std::swap(newTriangle.normal_indices.at(0), newTriangle.normal_indices.at(2));
            }

            newTriangles.push_back(newTriangle);
        }
    }

    polygons.reserve(polygons.size() + newTriangles.size());
    copy(newTriangles.begin(), newTriangles.end(), back_inserter(polygons));
}

void MainWindow::DoEnableModifyButton(bool enableModifyButton) {
    ui_->modifyMeshButton->setEnabled(enableModifyButton);
}

void MainWindow::DoEnableZBufferingButton(bool enableZBufferingButton) {
    ui_->enableZBufferingCheckBox->setEnabled(enableZBufferingButton);
}

void MainWindow::DoEnableBackfaceCullingButton(bool enableBackfaceCullingButton) {
    ui_->enableBackfaceCullingCheckBox->setEnabled(enableBackfaceCullingButton);
}

void MainWindow::DoEnableLambertianRadioButton(bool enableLambertianRadioButton) {
    ui_->lambertianModelRadioButton->setEnabled(enableLambertianRadioButton);
}

void MainWindow::DoEnablePhongRadioButton(bool enablePhongRadioButton) {
    ui_->phongModelRadioButton->setEnabled(enablePhongRadioButton);
}

pv::MeshStatus MainWindow::GetMeshStatus() {
    using namespace pv;

    if (scene_data_.vertices.size() == 0 ||
        scene_data_.polygons.size() == 0) { return  MeshStatus::NO_MODEL; }

    MeshStatus meshStatus = MeshStatus::TRIANGLES_ONLY;

    for (const auto& polygon : scene_data_.polygons){
        if (polygon.vertex_indices.size() == 4){
            meshStatus = MeshStatus::CONVERTIBLE_TO_TRIANGLES;
        }
        if (polygon.vertex_indices.size() > 4){
            return MeshStatus::NON_CONVERTIBLE_TO_TRIANGLES;
        }
    }

    return meshStatus;
}

pv::NormalStatus MainWindow::GetNormalStatus() {
    using namespace pv;

    if (scene_data_.vertices.size() == 0 ||
        scene_data_.polygons.size() == 0) { return  NormalStatus::NO_MODEL; }

    if (scene_data_.vertex_normals.size() == 0){
        return NormalStatus::NO_NORMALS_PROVIDED;
    }

    return NormalStatus::NORMALS_PROVIDED;
}

void MainWindow::UpdatedLightSourceListModelSlot(const pv::LightSourceListModel *model) {
    display_->DeferUpdatedLightSourceListModel(model);
}

void MainWindow::on_rasterizePolygonsCheckBox_stateChanged(int stateValue) {
    switch (stateValue){
        case Qt::Unchecked:
        display_->DeferRasterizePolygons(false);
            break;
        case Qt::Checked:
        display_->DeferRasterizePolygons(true);
            break;
        default:
        display_->DeferRasterizePolygons(false);
    }
}


void MainWindow::on_enableZBufferingCheckBox_stateChanged(int stateValue) {
    switch (stateValue){
        case Qt::Unchecked:{
            display_->DeferEnableZBuffering(false);
            break;
        }
        case Qt::Checked:{
            display_->DeferEnableZBuffering(true);
            break;
        }

        default:
        display_->DeferEnableZBuffering(false);
    }
}


void MainWindow::on_modifyMeshButton_clicked() {
    SplitQuadsIntoTriangles(scene_data_.polygons);
    UpdateMeshStatus();
}


void MainWindow::on_enableBackfaceCullingCheckBox_stateChanged(int stateValue) {
    switch (stateValue){
        case Qt::Unchecked:{
            display_->DeferEnableBackfaceCulling(false);
            break;
        }
        case Qt::Checked:{
            display_->DeferEnableBackfaceCulling(true);
            break;
        }

        default:
        display_->DeferEnableBackfaceCulling(false);
    }
}

void MainWindow::on_noShadingRadioButton_clicked() {
    display_->DeferShadingType(pv::SHADING_MODEL::NO_SHADING);
}


void MainWindow::on_lambertianModelRadioButton_clicked() {
    display_->DeferShadingType(pv::SHADING_MODEL::LAMBERTIAN_SHADING);
}


void MainWindow::on_phongModelRadioButton_clicked() {
    display_->DeferShadingType(pv::SHADING_MODEL::PHONG_SHADING);
}

void MainWindow::on_addLightPushButton_clicked() {
    using namespace pv;
    light_source_list_model_holder_->AppendLightSource(std::make_shared<LightSource>());
}


void MainWindow::on_removeLightPushButton_clicked() {
    using namespace pv;
    auto currentIndex = ui_->lightSourceListView->currentIndex();
    if (currentIndex.isValid()){
        light_source_list_model_holder_->RemoveLightSource(currentIndex.row());
    }
}
