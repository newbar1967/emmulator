#pragma once

#include <QVBoxLayout>
#include <QWidget>
#include <QList>
#include <QLineEdit>
#include <unordered_set>

#include "can_id_widget.h"

namespace cfg {

class Filter final
{
public:
    Filter(QWidget* widget,
           QVBoxLayout* layout,
           QLineEdit* edit,
           quint32 max_num_id)
        : widget_(widget)
        , layout_(layout)
        , edit_(edit)
        , max_num_id_(max_num_id)
    {}

    Filter(Filter& other) = delete;
    Filter& operator=(Filter& other) = delete;
    Filter(Filter&& other) = delete;
    Filter& operator=(Filter&& other) = delete;

    ~Filter() {
        ClearCanIdWidgests();
    }

    std::unordered_set<quint32> FromUiToList();
    void FromListToUi(const std::unordered_set<quint32>& filter_by_id);
    void DeleteAll();
    void DeleteSelectedId();
    void AddId();

private:
    QList<CanIdWidget*> GetCanIdWidgests();
    void ClearCanIdWidgests();
    void AddIdToWindow(quint32 id);
    bool CheckId(quint32 id);

private:
    QWidget* widget_;
    QVBoxLayout* layout_;
    QLineEdit* edit_;
    quint32 max_num_id_; // Максимальное количество
};

} // namespace cfg
