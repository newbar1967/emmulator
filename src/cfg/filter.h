#pragma once

#include <QVBoxLayout>
#include <QWidget>
#include <QList>
#include <QLineEdit>
#include <QComboBox>
#include <QObject>

#include "can_id_widget.h"
#include "common_cfg.h"

namespace cfg {

class Filter final : public QObject {
    Q_OBJECT

public:
    Filter(QWidget* widget,
           QVBoxLayout* layout,
           QLineEdit* edit,
           QComboBox* box,
           quint32 max_num_id)
        : widget_(widget)
        , layout_(layout)
        , edit_(edit)
        , box_(box)
        , max_num_id_(max_num_id)
        , last_сlicked_widget_(nullptr)
    {}

    Filter(Filter& other) = delete;
    Filter& operator=(Filter& other) = delete;
    Filter(Filter&& other) = delete;
    Filter& operator=(Filter&& other) = delete;

    ~Filter() {
        ClearCanIdWidgests();
    }

    IdWithParameters FromUiToList();
    void FromListToUi(const IdWithParameters& filter_by_id);
    void DeleteAll();
    void DeleteSelectedId();
    void AddId();
    void UpdateId();

private:
    QList<CanIdWidget*> GetCanIdWidgests();
    void ClearCanIdWidgests();
    void AddIdToWindow(quint32 id, const IdParameters& id_parameters);
    bool CheckId(quint32 id, bool is_extended);

private:
    QWidget* widget_;
    QVBoxLayout* layout_;
    QLineEdit* edit_;
    QComboBox* box_;
    quint32 max_num_id_; // Максимальное количество ID.
    CanIdWidget* last_сlicked_widget_;  // Последний виджет, по которому кликали.
};

} // namespace cfg
