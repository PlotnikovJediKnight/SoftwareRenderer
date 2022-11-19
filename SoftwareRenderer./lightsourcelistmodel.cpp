#include "lightsourcelistmodel.h"
#include "lightsourcewidget.h"
using namespace std;

namespace pv {

    LightSourceListModel::LightSourceListModel(LightSourceListView *view) :
        list_view_(view) {

        light_source_item_timer_ = new QTimer(this);
        connect(light_source_item_timer_, SIGNAL(timeout()), this, SLOT(ProcessLightSourcePayloads()));
        light_source_item_timer_->start(20);

        constexpr size_t LIST_VIEW_ITEM_HEIGHT = 160;
        QString style = QString("QListView:item { height: %1 }").arg( LIST_VIEW_ITEM_HEIGHT );
        list_view_->setStyleSheet(style);
    }

    LightSourceListModel::~LightSourceListModel() {
        delete light_source_item_timer_;
    }

    void LightSourceListModel::ProcessLightSourcePayloads() {

        if ( light_source_list_payloads_.length() == 0 )
            return;

          LightSourceListItemPayload payload = light_source_list_payloads_.at(0);
          QModelIndex index = payload.index;
          LightSource *item = payload.item;

          if ( !list_view_->indexWidget(index) &&
               index.isValid() ) {
            QWidget *widget = new LightSourceWidget(item);
            list_view_->setIndexWidget( index, widget );
          }

          light_source_list_payloads_.removeFirst();
    }

    void LightSourceListModel::AppendLightSource(std::shared_ptr<LightSource> lightSourcePtr) {
        size_t newIndex = light_source_items_.size();
        beginInsertRows(QModelIndex(), newIndex, newIndex);

        light_source_items_.push_back( lightSourcePtr );

        endInsertRows();

        emit LightSourceListModelUpdated(this);
    }

    void LightSourceListModel::RemoveLightSource(size_t index) {
        beginRemoveRows(QModelIndex(), index, index);

        auto beginIterator = light_source_items_.begin();
        light_source_items_.erase( beginIterator + index );

        endRemoveRows();

        emit LightSourceListModelUpdated(this);
    }

    QModelIndex LightSourceListModel::index(int row, int column, const QModelIndex &parent) const {
        if (!hasIndex(row, column, parent))
            return QModelIndex();

          if (row < light_source_items_.size()) {
            QModelIndex index = createIndex(row, column, light_source_items_.at(row).get());
            return index;
          }
          else
            return QModelIndex();
    }

    QModelIndex LightSourceListModel::parent(const QModelIndex &child) const {
        return QModelIndex();
    }

    int LightSourceListModel::rowCount(const QModelIndex &parent) const {
        return light_source_items_.size();
    }

    int LightSourceListModel::columnCount(const QModelIndex &parent) const {
        return 1;
    }

    QVariant LightSourceListModel::data(const QModelIndex &index, int role) const {
        if (!index.isValid())
          return QVariant();

        if (role != Qt::DisplayRole)
          return QVariant();

        LightSource *light = static_cast<LightSource*>( index.internalPointer() );

        if ( !list_view_->indexWidget(index) ) {
            const_cast<LightSourceListModel*>(this)->AddLightSourceListItemPayload(index, light);
        }

        return QVariant();
    }

    auto LightSourceListModel::GetLightSourceItems() const
    -> std::vector<std::shared_ptr<LightSource>> {
        return light_source_items_;
    }

    void LightSourceListModel::AddLightSourceListItemPayload(QModelIndex index, LightSource* light) {
        struct LightSourceListItemPayload payload;
        payload.index = index;
        payload.item = light;
        light_source_list_payloads_.append(payload);
    }

}
