#ifndef LIGHTSOURCELISTVIEW_H
#define LIGHTSOURCELISTVIEW_H

#include <QListView>

namespace pv {

    class LightSourceListView : public QListView {
    public:
        LightSourceListView(QWidget *parent=nullptr);
    };

}

#endif // LIGHTSOURCELISTVIEW_H
