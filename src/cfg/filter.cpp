#include <QMessageBox>

#include "settings_view.h"
#include "filter.h"

namespace cfg {

std::unordered_set<quint32> Filter::FromUiToList() {
    std::unordered_set<quint32> filter_by_id;
    auto widgests = GetCanIdWidgests();

    for (auto it = widgests.constBegin(); it != widgests.constEnd(); ++it) {
        bool ok;
        auto id = (*it)->GetIdStr().toInt(&ok, 16);
        filter_by_id.insert(id);
    }

    return filter_by_id;
}

QList<CanIdWidget*> Filter::GetCanIdWidgests() {
    return widget_->findChildren<CanIdWidget*>();
}

void Filter::FromListToUi(const std::unordered_set<quint32>& filter_by_id) {
    ClearCanIdWidgests();
    for (const auto& id : filter_by_id) {
        // Добавляем id в окно фильтра.
        AddIdToWindow(id);
    }
}

void Filter::ClearCanIdWidgests() {
    auto widgests = GetCanIdWidgests();
    qDeleteAll(widgests);
    widgests.clear();
}

void Filter::AddIdToWindow(quint32 id) {
    // Конвертируем число в форматированный текст.
    auto id_str = QString::number(id, 16).toUpper().rightJustified(cfg::SettingsWindow::CAN_ID_MAX_LENGTH, '0');
    // Добавляем в окно фильтра.
    auto can_id_widget = new CanIdWidget();
    can_id_widget->SetColorVisible(false);
    can_id_widget->SetIdStr(id_str);
    layout_->insertWidget(GetCanIdWidgests().size(), can_id_widget);
}

void Filter::DeleteAll() {
    // Очищаем окно виджетов фильтра.
    ClearCanIdWidgests();
}

void Filter::DeleteSelectedId() {
    auto widgests = GetCanIdWidgests();

    // Идем по списку виджетов, ищем где установлен селект.
    for (auto it = widgests.constBegin(); it != widgests.constEnd(); ++it) {
        if ((*it)->GetSelect()) {
            layout_->removeWidget(*it); // Удаляем виджет.
            delete *it; // Освобождаем память.
        }
    }
}

void Filter::AddId() {
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
    AddIdToWindow(id);
}

bool Filter::CheckId(quint32 id) {
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


} // namespace cfg

