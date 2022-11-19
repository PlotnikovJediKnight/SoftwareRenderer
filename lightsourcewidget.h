#ifndef LIGHTSOURCEWIDGET_H
#define LIGHTSOURCEWIDGET_H

#include <QWidget>
#include <QLabel>
#include "lightsource.h"

QT_BEGIN_NAMESPACE
namespace Ui {
  class LightSourceListItem;
}
QT_END_NAMESPACE

namespace pv {

    class LightSourceWidget : public QWidget {
        Q_OBJECT
    public:
        explicit LightSourceWidget(LightSource* light, QWidget *parent = nullptr);
        ~LightSourceWidget();

      private:
        void UpdateLightColorLabelStyleSheet(QColor& penColor);

      private slots:
        void UpdateLightDegreeLabel(int newValue);
        void UpdateLightSourceDegrees(int newValue);
        void UpdateLightPowerLabel(int newValue);
        void UpdateLightSourcePower(int newValue);
        void OnChoseLightColorButtonAction();

    private:
        Ui::LightSourceListItem *ui_;
        LightSource *light_source_;
        QString color_label_style_sheet_;

    };
}

#endif // LIGHTSOURCEWIDGET_H
