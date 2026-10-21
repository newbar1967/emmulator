#include <QMessageBox>
#include <QColorDialog>

#include "settings_view.h"
#include "color_filter.h"

namespace cfg {

std::unordered_map<quint32, QColor> ColorFilter::FromUiToList() {
    std::unordered_map<quint32, QColor> color_filter_by_id;
    auto widgests = GetCanIdWidgests();

    for (auto it = widgests.constBegin(); it != widgests.constEnd(); ++it) {
        bool ok;
        auto id = (*it)->GetIdStr().toInt(&ok, 16);
        auto color = (*it)->GetColor();
        color_filter_by_id[id] = color;
    }

    return color_filter_by_id;
}

QList<CanIdWidget*> ColorFilter::GetCanIdWidgests() {
    return widget_->findChildren<CanIdWidget*>();
}

void ColorFilter::FromListToUi(const std::unordered_map<quint32, QColor>& color_filter_by_id) {
    ClearCanIdWidgests();
    for (const auto& [id, color] : color_filter_by_id) {
        // Добавляем id в окно фильтра.
        AddIdToWindow(id, color);
    }
}

void ColorFilter::ClearCanIdWidgests() {
    auto widgests = GetCanIdWidgests();
    qDeleteAll(widgests);
    widgests.clear();
}

void ColorFilter::AddIdToWindow(quint32 id, const QColor& color) {
    // Конвертируем число в форматированный текст.
    auto id_str = QString::number(id, 16).toUpper().rightJustified(cfg::SettingsWindow::CAN_ID_MAX_LENGTH, '0');
    // Добавляем в окно фильтра.
    auto can_id_widget = new CanIdWidget();
    can_id_widget->SetIdStr(id_str);
    can_id_widget->SetColor(color);
    layout_->insertWidget(GetCanIdWidgests().size(), can_id_widget);
}

void ColorFilter::DeleteAll() {
    // Очищаем окно виджетов фильтра.
    ClearCanIdWidgests();
}

void ColorFilter::DeleteSelectedId() {
    auto widgests = GetCanIdWidgests();

    // Идем по списку виджетов, ищем где установлен селект.
    for (auto it = widgests.constBegin(); it != widgests.constEnd(); ++it) {
        if ((*it)->GetSelect()) {
            layout_->removeWidget(*it); // Удаляем виджет.
            delete *it; // Освобождаем память.
        }
    }
}

void ColorFilter::AddId() {
    if (edit_->text().isEmpty()) {
        return;
    }

    // Превращаем текст в число.
    bool ok;
    auto id = edit_->text().toInt(&ok, 16);

    // Проверим количество виджетов идентификаторов.
    if (GetCanIdWidgests().size() == max_num_id_) {
        QMessageBox::warning(QApplication::activeWindow(), "Предупреждение",
                             "Достигнуто максимальное количество id-фильтров!");
        return;
    }

    // Проверяем, содержится ли число в списке.
    if (CheckId(id)) {
        QMessageBox::warning(QApplication::activeWindow(), "Предупреждение",
                             "Такой id-фильтр уже есть!");
        return; // Этот id уже есть, выходим.
    }

    // Добавляем id в окно фильтра.
    AddIdToWindow(id, QColor(Qt::white)); // Начальный цвет - белый.
}

bool ColorFilter::CheckId(quint32 id) {
    auto widgests = GetCanIdWidgests();

    // Идем по списку виджетов.
    for (auto it = widgests.constBegin(); it != widgests.constEnd(); ++it) {
        // Превращаем текст в число.
        bool ok;
        auto id_lbl = (*it)->GetIdStr().toInt(&ok, 16);
        if (id == id_lbl) {
            return true;
        }
    }
    return false;
}

void ColorFilter::SetColorSelectedId() {
    if (!CheckSelectedId()) {
        return; // Виджетов с установленный флагом select нет.
    }

    QColor color = QColorDialog::getColor(Qt::white,
                                          QApplication::activeWindow(),
                                          "Выберите цвет"
                                          );

    if (!color.isValid()) {
        return;
    }

    SetColorId(color);
}

void ColorFilter::SetColorId(const QColor& color) {
    auto widgests = GetCanIdWidgests();

    // Идем по списку виджетов.
    for (auto it = widgests.constBegin(); it != widgests.constEnd(); ++it) {
        if ((*it)->GetSelect()) {
            (*it)->SetColor(color); // Устанавливаем цвет выбранным.
        }
    }
}

// Проверяет, есть виджеты с установленным флагом select.
bool ColorFilter::CheckSelectedId() {
    auto widgests = GetCanIdWidgests();

    // Идем по списку виджетов.
    for (auto it = widgests.constBegin(); it != widgests.constEnd(); ++it) {
        if ((*it)->GetSelect()) {
            return true;
        }
    }
    return false;
}

} // namespace cfg

