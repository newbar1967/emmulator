#pragma once

#include <QVBoxLayout>
#include <QWidget>
#include <QList>
#include <QLineEdit>
#include <unordered_map>

#include "can_id_widget.h"

namespace cfg {

class ColorFilter final
{
public:
    ColorFilter(QWidget* widget,
           QVBoxLayout* layout,
           QLineEdit* edit,
           quint32 max_num_id)
        : widget_(widget)
        , layout_(layout)
        , edit_(edit)
        , max_num_id_(max_num_id)
    {}

    ColorFilter(ColorFilter& other) = delete;
    ColorFilter& operator=(ColorFilter& other) = delete;
    ColorFilter(ColorFilter&& other) = delete;
    ColorFilter& operator=(ColorFilter&& other) = delete;

    ~ColorFilter() {
        ClearCanIdWidgests();
    }

    std::unordered_map<quint32, QColor> FromUiToList();
    void FromListToUi(const std::unordered_map<quint32, QColor>& color_filter_by_id);
    void DeleteAll();
    void DeleteSelectedId();
    void AddId();
    void SetColorSelectedId();

private:
    QList<CanIdWidget*> GetCanIdWidgests();
    void ClearCanIdWidgests();
    void AddIdToWindow(quint32 id, const QColor& color);
    bool CheckId(quint32 id);
    void SetColorId(const QColor& color);
    bool CheckSelectedId();

private:
    QWidget* widget_;
    QVBoxLayout* layout_;
    QLineEdit* edit_;
    quint32 max_num_id_; // Максимальное количество
};

} // namespace cfg
