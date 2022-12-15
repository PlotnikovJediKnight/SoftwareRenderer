#include "headers/gui/lightsourcewidget.h"
#include "ui_lightsourcelistitem.h"
#include <QColorDialog>

namespace pv {

    LightSourceWidget::LightSourceWidget(LightSource* light, QWidget *parent) :
        QWidget{parent}, light_source_(light) {

        ui_ = new Ui::LightSourceListItem;
        ui_->setupUi( this );

        ui_->lightItemDegreeValue->setText(QString::number(light->GetLightSourcePositionDegrees()));
        ui_->lightItemDegreeSlider->setValue(light->GetLightSourcePositionDegrees());

        ui_->lightItemPowerValue->setText(QString::number(light->GetSpecularPower()));
        ui_->lightItemPowerSlider->setValue(light->GetSpecularPower());

        connect(ui_->lightItemDegreeSlider, SIGNAL(valueChanged(int)), this, SLOT(UpdateLightDegreeLabel(int)));
        connect(ui_->lightItemDegreeSlider, SIGNAL(valueChanged(int)), this, SLOT(UpdateLightSourceDegrees(int)));

        connect(ui_->lightItemPowerSlider, SIGNAL(valueChanged(int)), this, SLOT(UpdateLightPowerLabel(int)));
        connect(ui_->lightItemPowerSlider, SIGNAL(valueChanged(int)), this, SLOT(UpdateLightSourcePower(int)));

        connect(ui_->choseLightColorButton, SIGNAL(clicked()), this, SLOT(OnChoseLightColorButtonAction()));

        color_label_style_sheet_ =
                R"(
                        background-color: rgb(%1, %2, %3);
                        border: 1px solid black
                   )";
    }

    void LightSourceWidget::OnChoseLightColorButtonAction() {
        QColor penColor = QColorDialog::getColor(Qt::white, this, "Choose Light Color");
        if (penColor.isValid()){
            uchar red = penColor.red();
            uchar green = penColor.green();
            uchar blue = penColor.blue();
            light_source_->SetLightColor({255, red, green, blue });
            UpdateLightColorLabelStyleSheet(penColor);
        }
    }

    void LightSourceWidget::UpdateLightColorLabelStyleSheet(QColor& penColor) {
        QString filledInStyleSheet =
                this->color_label_style_sheet_
                .arg(QString::number(penColor.red()),
                     QString::number(penColor.green()),
                     QString::number(penColor.blue()));

        ui_->lightColorLabel->setStyleSheet( filledInStyleSheet );
    }

    LightSourceWidget::~LightSourceWidget() {
        delete ui_;
    }

    void LightSourceWidget::UpdateLightDegreeLabel(int newValue) {
        ui_->lightItemDegreeValue->setText(QString::number(newValue));
    }

    void LightSourceWidget::UpdateLightSourceDegrees(int newValue) {
        light_source_->UpdateLightSourcePosition(newValue);
    }

    void LightSourceWidget::UpdateLightPowerLabel(int newValue) {
        ui_->lightItemPowerValue->setText(QString::number(newValue));
    }

    void LightSourceWidget::UpdateLightSourcePower(int newValue) {
        light_source_->SetSpecularPower(newValue);
    }

}
