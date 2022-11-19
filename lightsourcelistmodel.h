#ifndef LIGHTSOURCELISTMODEL_H
#define LIGHTSOURCELISTMODEL_H

#include <QTimer>
#include <QAbstractItemModel>
#include <memory>
#include <vector>
#include "lightsourcelistview.h"
#include "lightsource.h"

namespace pv {

    class LightSourceListModel : public QAbstractItemModel {
        Q_OBJECT
    public:
        LightSourceListModel(LightSourceListView *view);
        ~LightSourceListModel();

    public slots:
      void ProcessLightSourcePayloads();

    signals:
      void LightSourceListModelUpdated(const pv::LightSourceListModel*);

    public:
        void AppendLightSource(std::shared_ptr<LightSource>);
        void RemoveLightSource(size_t index);

        virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
        virtual QModelIndex parent(const QModelIndex &child) const override;
        virtual int rowCount(const QModelIndex &parent) const override;
        virtual int columnCount(const QModelIndex &parent) const override;
        virtual QVariant data(const QModelIndex &index, int role) const override;

        auto GetLightSourceItems() const -> std::vector<std::shared_ptr<LightSource>>;

    private:
        QTimer *light_source_item_timer_;

        struct LightSourceListItemPayload {
            QModelIndex index;
            LightSource* item;
        };

        QVector<struct LightSourceListItemPayload> light_source_list_payloads_;
        void AddLightSourceListItemPayload(QModelIndex index, LightSource* item);


        std::vector<std::shared_ptr<LightSource>> light_source_items_;
        LightSourceListView* list_view_;
    };

}

#endif // LIGHTSOURCELISTMODEL_H
