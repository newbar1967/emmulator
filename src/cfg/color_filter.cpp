#include <QMessageBox>
#include <QColorDialog>

#include "settings_view.h"
#include "color_filter.h"

namespace cfg {

IdWithParameters ColorFilter::FromUiToList() {
    IdWithParameters color_filter_by_id;
    auto widgests = GetCanIdWidgests();

    for (auto it = widgests.constBegin(); it != widgests.constEnd(); ++it) {
        bool ok;
        auto id = (*it)->GetIdStr().toInt(&ok, 16);
        auto header_type = (*it)->GetTextHeaderType();
        if (header_type == EXTENDED_HEADER) {
            id |= EXTENDED_HEADER_ADD_BIT;
        }

        color_filter_by_id[id] = IdParameters{
            .header_type = header_type,
            .color = (*it)->GetColor()};
    }

    return color_filter_by_id;
}

void ColorFilter::FromListToUi(const IdWithParameters& color_filter_by_id) {
    ClearCanIdWidgests();
    for (const auto& [id, id_parameters] : color_filter_by_id) {
        // Добавляем id-widget в окно фильтра.
        AddIdToWindow(id, id_parameters);
    }
}

QList<CanIdWidget*> ColorFilter::GetCanIdWidgests() {
    return widget_->findChildren<CanIdWidget*>();
}

void ColorFilter::ClearCanIdWidgests() {
    auto widgests = GetCanIdWidgests();
    qDeleteAll(widgests);
    widgests.clear();
    last_сlicked_widget_ = nullptr;
}

void ColorFilter::AddIdToWindow(quint32 id, const IdParameters& id_parameters) {
    // Конвертируем число в форматированный текст.
    auto id_str = QString::number(id & EXTENDED_HEADER_SUB_BIT, 16).toUpper()
                      .rightJustified(cfg::SettingsWindow::CAN_ID_MAX_LENGTH, '0');
    // Добавляем в окно фильтра.
    auto can_id_widget = new CanIdWidget();
    can_id_widget->SetIdStr(id_str);
    can_id_widget->SetColor(id_parameters.color);
    can_id_widget->SetTextHeaderType(id_parameters.header_type);
    layout_->insertWidget(GetCanIdWidgests().size(), can_id_widget);
    connect(can_id_widget, &CanIdWidget::mouseClick, this, [this, can_id_widget]() {
                edit_->setText(can_id_widget->GetIdStr());
                box_->setCurrentText(can_id_widget->GetTextHeaderType());
                last_сlicked_widget_ = can_id_widget;
            });
    connect(can_id_widget, &CanIdWidget::mouseColorDoubleClick, this, [can_id_widget]() {
        QColor color = QColorDialog::getColor(can_id_widget->GetColor(),
                                              QApplication::activeWindow(),
                                              "Выберите цвет"
                                              );
        if (!color.isValid()) {
            return;
        }

        can_id_widget->SetColor(color);
    });

}

void ColorFilter::DeleteAll() {
    // Очищаем окно виджетов фильтра.
    ClearCanIdWidgests();
}

void ColorFilter::DeleteSelectedId() {
    auto widgests = GetCanIdWidgests();

    // Идем по списку виджетов, ищем где установлен селект.
    for (auto it = widgests.constBegin(); it != widgests.constEnd(); ++it) {
        if ((*it)->GetSelected()) {
            layout_->removeWidget(*it); // Удаляем виджет.
            delete *it; // Освобождаем память.
        }
    }

    last_сlicked_widget_ = nullptr;
}

void ColorFilter::UpdateId() {
    if (last_сlicked_widget_ == nullptr) {
        QMessageBox::warning(QApplication::activeWindow(), "Предупреждение",
                             "Сначала кликните по id-фильтру, чтобы отредактировать его!");
        return;
    }

    if (edit_->text().isEmpty()) {
        return;
    }

    // Превращаем текст в число.
    bool ok;
    auto id = edit_->text().toUInt(&ok, 16);
    if (!ok) {
        QMessageBox::warning(QApplication::activeWindow(), "Предупреждение",
                             "Ошибка преобразования id!");
        return;
    }

    // Обрежем ID по стандарту.
    if (box_->currentText() == STANDART_HEADER) {
        id &= STANDART_HEADER_MASK;
    } else {
        id &= EXTENDED_HEADER_MASK;
    }

    auto id_str = QString::number(id, 16).toUpper().rightJustified(cfg::SettingsWindow::CAN_ID_MAX_LENGTH, '0');
    last_сlicked_widget_->SetIdStr(id_str);
    last_сlicked_widget_->SetTextHeaderType(box_->currentText());
}

void ColorFilter::AddId() {
    // Проверим количество виджетов идентификаторов.
    if (GetCanIdWidgests().size() == max_num_id_) {
        QMessageBox::warning(QApplication::activeWindow(), "Предупреждение",
                             "Достигнуто максимальное количество id-фильтров!");
        return;
    }

    if (edit_->text().isEmpty()) {
        return;
    }

    // Превращаем текст в число.
    bool ok;
    auto id = edit_->text().toUInt(&ok, 16);
    if (!ok) {
        if (!ok) {
            QMessageBox::warning(QApplication::activeWindow(), "Предупреждение",
                                 "Ошибка преобразования id!");
            return;
        }
    }

    // Параметры id.
    IdParameters id_parameters {
        .header_type = box_->currentText(),
        .color = QColor(Qt::white)  // Начальный цвет - белый.
    };

    // Обрежем ID по стандарту.
    bool is_extended = false;
    if (id_parameters.header_type == STANDART_HEADER) {
        id &= STANDART_HEADER_MASK;
    } else {
        is_extended = true;
        id &= EXTENDED_HEADER_MASK;
    }

    // Проверяем, содержится ли число в списке.
    auto id_str = QString::number(id, 16).toUpper().rightJustified(cfg::SettingsWindow::CAN_ID_MAX_LENGTH, '0');
    auto header_type_str = box_->currentText();
    if (CheckId(id, is_extended)) {
        QMessageBox::warning(QApplication::activeWindow(), "Предупреждение",
                             "Такой id-фильтр уже есть!\nid = "
                                 + id_str
                                 + ", "
                                 + header_type_str);
        return; // Этот id уже есть, выходим.
    }

    // Добавляем id-widget в окно фильтра.
    AddIdToWindow(id, id_parameters);
}

bool ColorFilter::CheckId(quint32 id, bool is_extended) {
    auto widgests = GetCanIdWidgests();

    // Идем по списку виджетов.
    for (auto it = widgests.constBegin(); it != widgests.constEnd(); ++it) {
        // Превращаем текст в число.
        bool ok;
        auto id_lbl = (*it)->GetIdStr().toInt(&ok, 16);
        auto is_extended_lbl = (*it)->GetTextHeaderType() == EXTENDED_HEADER;
        if (id == id_lbl && is_extended == is_extended_lbl) {
            return true;
        }
    }
    return false;
}

} // namespace cfg

