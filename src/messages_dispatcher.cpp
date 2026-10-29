#include <QString>
#include <QDataStream>
#include <QTimer>
#include <QMessageBox>
#include <QSignalBlocker>
#include "messages_dispatcher.h"
#include "time_convert.h"
#include "utils.h"

using namespace utils;
using namespace server;
using namespace cfg;
using namespace emm;

MessagesDispatcher::MessagesDispatcher(MainWindow& view,
                       UdpServer& udp_server,
                       Settings& settings,
                       QObject* parent)
    :QObject(parent)
    ,view_(view)
    ,udp_server_(udp_server)
    ,settings_(settings)
    ,count_all_recive_messages_(0)
    ,count_can_recive_messages_(0)
    ,need_print_(false)
    ,work_mode_(workMode::OFF)
    ,previous_mode_string_("Выкл")
    ,log_file_stream_(nullptr)
    ,connect_status_(false)
{
    udp_server_.SetDataReadyCallback([this](const QVector<QByteArray>& data){ReadDeviceMessages(data);});

    settings_.AddCallBack([this](){UpdateSettings();});
    settings_.SetEnabledHardSettingsTab(false);

    SetCountAllReciveMessages(count_all_recive_messages_);
    SetCountCanReciveMessages(count_can_recive_messages_);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){SendMessage(message_type::PING, true);});
    timer->start(1000); // Периодическая проверка связи.
}

MessagesDispatcher::~MessagesDispatcher() {
    SendMessage(message_type::STOP_CAN_MESSAGES);

    // Ждём немного, чтобы сообщение успело обработаться.
    QThread::msleep(100); // 100 мс ожидания.
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 100);
}

void MessagesDispatcher::SetWorkMode(const QString& mode) {
    if (mode == "Выкл") {
        SendMessage(message_type::STOP_CAN_MESSAGES, true);
    } else if (mode == "Эмулятор") {
        SendMessage(message_type::START_EMMULATOR, PrepareDataForStartEmmulatorOrMonitor(), true);
    } else if (mode == "Монитор") {
        SendMessage(message_type::START_MONITOR, PrepareDataForStartEmmulatorOrMonitor(), true);
    }
}

void MessagesDispatcher::SendMessageImpl(quint8 message_type,
                                     const QByteArray& data,
                                     bool response_is_needed,
                                     int timeout_ms)
{
    // Такое сообщение было отправлено и ответа по нему еще нет.
    if (send_message_types_.count(message_type)) {
        return; // Ничего не отправляем.
    }

    // Если ответ надо ждать, запускаем таймер таймаута для него.
    if (response_is_needed) {
        // Добавляем сообщение в список.
        send_message_types_.insert(message_type);
        // Запускаем таймер таймаута для него.
        QTimer::singleShot(timeout_ms, [this, message_type]() {
            // Про прошествии таймаута его обработка.
            TimeoutHandling(message_type);
        });
    }

    QByteArray message = PrepareMessageToSend(message_type, data);
    udp_server_.Write(message);
}

void MessagesDispatcher::ReadDeviceMessages(const QVector<QByteArray>& data) {
    all_can_messages_text_.clear();

    for (auto const& message : data) {
        ParseDeviceMessage(message);
    }

    // Если есть CAN-сообщения для печати, то печатаем их все разом.
   if (!all_can_messages_text_.isEmpty()) {
        if (message_coloring_mode_) {
            // Режим печати с подкраской.
            view_.GetUi()->text_msg->appendHtml(all_can_messages_text_);
        } else {
            // Режим печати без подкраски.
            all_can_messages_text_.remove(all_can_messages_text_.size() - 1, 1); // Удалим '\n' в конце.
            view_.GetUi()->text_msg->appendPlainText(all_can_messages_text_);
        }
        QApplication::processEvents();
   }
}

// Чтение сообщения из устройства.
void MessagesDispatcher::ParseDeviceMessage(const QByteArray& data) {
    if (!ParseEthernetMessage(data)) {
        return;
    }

    SetCountAllReciveMessages(++count_all_recive_messages_);

    if (ethernet_message_.status) {
        // Здесь должны быть действия в зависимости от status.
        // Нужно завести поле в StatusBar и печатать там сообщения об ошибке.
        // Например, точно нужно печатать, что сообщение CAN не отправлено.
    }

    // Такое сообщение есть в списке ждущих ответа.
    if (send_message_types_.count(ethernet_message_.type)) {
        // Удаляем его из списка.
        DeleteMessageTypeFromList(ethernet_message_.type);
    }

    // Действия по приходу сообщений.
    switch(ethernet_message_.type) {
        case STOP_CAN_MESSAGES:
            StopCanMessagesProcessing();
            break;
        case START_MONITOR:
            StartMonitorProcessing();
            break;
        case START_EMMULATOR:
            StartEmmulatorProcessing();
            break;
        case CAN_DATA:
            CanDataProcessing(ethernet_message_.payload);
            break;
        case PING:
            PingProcessing(ethernet_message_.payload);
            break;
        case SET_HARD_SETTINGS:
            HardSettingsProcessing();
            break;
    }
}

void MessagesDispatcher::StopCanMessagesProcessing() {
    work_mode_ = workMode::OFF;
    previous_mode_string_ = "Выкл";
}

void MessagesDispatcher::StartMonitorProcessing() {
    work_mode_ = workMode::MONITOR;
    previous_mode_string_ = "Монитор";
    ClrCountCanReciveMessages();
}

void MessagesDispatcher::StartEmmulatorProcessing() {
    work_mode_ = workMode::EMMULATOR;
    previous_mode_string_ = "Эммулятор";
    ClrCountCanReciveMessages();
}

void MessagesDispatcher::CanDataProcessing(const QByteArray& payload) {
    ParseCanMessages(payload);
    if (can_messages_.empty()) {
        return;
    }

    // Обрабатываем все сообщения CAN.
    for (const auto& can_message : can_messages_) {
        // Записываем сообщения в файл, если поток открыт и сообщение проходит фильтр.
        if (log_file_stream_ && FilteringByCanId(can_message.id)) {
            *log_file_stream_ << CanMessageToQString(can_message) << Qt::endl;
            log_file_stream_->flush();
        }

        // Если необходима печать и сообщение проходит фильтр для печати.
        if (need_print_ && FilteringByCanId(can_message.id)) {
            // Собираем все сообщения в один текст.
            if (message_coloring_mode_) {
                // Режим печати с подкраской.
                auto color_back = ColoringByCanId(can_message.id);
                if (color_back.has_value()) {
                    all_can_messages_text_ += QString("<div style='background-color: %1; padding: 2px;'>%2</div>")
                                            .arg(color_back->name(), CanMessageToQString(can_message).toHtmlEscaped());
                } else {
                    all_can_messages_text_ += QString("<div>%1</div>").arg(CanMessageToQString(can_message).toHtmlEscaped());
                }
            } else {
                // Режим печати без подкраски.
                all_can_messages_text_ += CanMessageToQString(can_message) + "\n";
            }
        }

        if (work_mode_ == workMode::EMMULATOR) { // Если режим работы эммулятор.
            EmmulatorWork(can_message); // Эммулятор работает.
        }
    }

    count_can_recive_messages_ += static_cast<int>(can_messages_.size());
    SetCountCanReciveMessages(count_can_recive_messages_);
}

void MessagesDispatcher::PingProcessing(const QByteArray& payload) {
    connect_status_ = true;
    view_.SetConnectStatus(connect_status_);
    settings_.SetEnabledHardSettingsTab(connect_status_);

    // Синхронизация режимов работы в устройстве и в программе.
    if (payload.isEmpty()) {
        return;
    }

    auto work_mode_as_string = view_.GetUi()->mode_cbox->itemText(payload.at(0));
    if (work_mode_as_string.isEmpty()) {
        return;
    }

    if (view_.GetUi()->mode_cbox->currentText() != work_mode_as_string) {
        // Приведем режим в соответствие.
        view_.GetUi()->mode_cbox->setCurrentText(work_mode_as_string);
    }

    // Настройки аппаратуры начинаются с payload[1] — сразу после work_mode.
    // Устройство отдаёт их через memcpy структуры _hard_settings (little-endian).
    constexpr int hard_settings_offset = 1;
    if (payload.size() >= hard_settings_offset + static_cast<int>(sizeof(cfg::HardSettings))) {
        QByteArray hs_data = payload.mid(hard_settings_offset, sizeof(cfg::HardSettings));
        QDataStream stream(&hs_data, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream >> settings_.GetValue().hard_settings.beep_can_need
            >> settings_.GetValue().hard_settings.beep_can_pause;
    }
}

void MessagesDispatcher::HardSettingsProcessing() {
}

// Действия по окончании тайаута для каждой команды.
void MessagesDispatcher::TimeoutHandling(quint8 message_type) {
    // Проверяем, есть ли идентификатор сообщения в списке.
    if (!send_message_types_.count(message_type)) {
        return; // Идентификатор сообщения был удален из списка, просто выходим.
    }
    // Удаляем его из списка здесь.
    DeleteMessageTypeFromList(message_type);

    // Разделение действий по типу сообщения.
    switch(message_type) {
        case STOP_CAN_MESSAGES:
            StopCanMessagesTimeoutHandling();
            break;
        case START_MONITOR:
            StartMonitorTimeoutHandling();
            break;
        case START_EMMULATOR:
            StartEmmulatorTimeoutHandling();
            break;
        case PING:
            PingTimeoutHandling();
            break;
        case SET_HARD_SETTINGS:
            HardSettingsTimeoutHandling();
            break;
    }
}

void MessagesDispatcher::StopCanMessagesTimeoutHandling() {
    WorkModeStatusToInitial();
    QMessageBox::critical(QApplication::activeWindow(), "Ошибка",
                          "Нет ответа от устройства - команда \"Выкл\"");
}

void MessagesDispatcher::StartMonitorTimeoutHandling() {
    WorkModeStatusToInitial();
    QMessageBox::critical(QApplication::activeWindow(), "Ошибка",
                          "Нет ответа от устройства - команда \"Монитор\"");
}

void MessagesDispatcher::StartEmmulatorTimeoutHandling() {
    WorkModeStatusToInitial();
    QMessageBox::critical(QApplication::activeWindow(), "Ошибка",
                          "Нет ответа от устройства - команда \"Еммулятор\"");
}

void MessagesDispatcher::PingTimeoutHandling() {
    connect_status_ = false;
    view_.SetConnectStatus(connect_status_);
    settings_.SetEnabledHardSettingsTab(connect_status_);
}

void MessagesDispatcher::HardSettingsTimeoutHandling() {
    QMessageBox::critical(QApplication::activeWindow(), "Ошибка",
                          "Нет ответа от устройства - команда \"Запись настроек в адаптер\"");
}

// Вернуть состояние ComboBox "mode" в исходное.
void MessagesDispatcher::WorkModeStatusToInitial() {
    // Временно заблокируем сигналы ModeBox.
    QSignalBlocker blocker(view_.GetUi()->mode_cbox);
    view_.GetUi()->mode_cbox->setCurrentText(previous_mode_string_);
}

void MessagesDispatcher::DeleteMessageTypeFromList(quint8 message_type) {
    auto it = send_message_types_.find(message_type);
    send_message_types_.erase(it);
}

bool MessagesDispatcher::ParseEthernetMessage(const QByteArray& data) {
    // Проверим минимальную длину пакета.
    if (data.size() < EthernetPacket::MIN_SIZE) {
        return false;
    }

    QDataStream data_stream(data);
    data_stream.setByteOrder(QDataStream::LittleEndian);

    // Читаем фиксированные поля.
    data_stream >> ethernet_message_.size
                >> ethernet_message_.status
                >> ethernet_message_.type;

    // Проверяем успешность чтения фиксированных полей.
    if (data_stream.status() != QDataStream::Ok) {
        return false;
    }

    // Проверим соответствие длины размеру массива.
    if (ethernet_message_.size + 1 != data.size()) {
        return false;
    }

    // Проверим контрольную сумму.
    quint8 cs = 0;
    for (quint16 i = 0; i + 1 < data.size(); ++i) {
        cs += static_cast<quint8>(data.at(i));
    }
    if (cs != static_cast<quint8>(data.at(data.size() - 1))) { // Последний байт в массиве - cs.
        return false;
    }

    // Извлечем данные, если они есть.
    ethernet_message_.payload = data.mid(EthernetPacket::DATA_BEGIN, data.size() - EthernetPacket::MIN_SIZE);

    return true;
}

void MessagesDispatcher::ParseCanMessages(const QByteArray& data) {
    can_messages_.clear();

    // Проверяем, что длина данных кратна размеру одного CAN-пакета.
    if (data.size() % CanPacket::SIZE != 0) {
        return;
    }

    const int messages_count = data.size() / CanPacket::SIZE;

    for (int i = 0; i < messages_count; ++i) {
        // Извлекаем один CAN-пакет.
        const QByteArray packet_data = data.mid(i * CanPacket::SIZE, CanPacket::SIZE);

        // Используем существующий метод для разбора одного пакета.
        auto message = ParseCanMessage(packet_data);

        // Если разбор успешен, добавляем сообщение в вектор.
        if (message.has_value()) {
            can_messages_.push_back(std::move(message.value()));
        }
    }
}

std::optional<MessagesDispatcher::CanMessage> MessagesDispatcher::ParseCanMessage(const QByteArray& data) {
    if (data.size() != CanPacket::SIZE) {
        return std::nullopt;
    }

    QDataStream data_stream(data);
    data_stream.setByteOrder(QDataStream::LittleEndian);

    // Читаем фиксированные поля.
    CanMessage can_message;
    data_stream >> can_message.number
                >> can_message.time_stamp
                >> can_message.id
                >> can_message.size;

    // Проверяем успешность чтения фиксированных полей.
    if (data_stream.status() != QDataStream::Ok) {
        return std::nullopt;
    }

    can_message.payload = data.mid(CanPacket::DATA_BEGIN, data.size() - CanPacket::HEADER_SIZE);

    return can_message;
}

const QString MessagesDispatcher::CanMessageToQString(const CanMessage& can_message) const {
    QString qs;
    utils::TimeConvert time_convert(time_convert_type_);

    QString header_type;
    if (can_message.id & EXTENDED_HEADER_ADD_BIT) {
        header_type = "EXT";
    } else {
        header_type = "STD";
    }

    qs += QString("%1 %2 %3 %4 %5 ").arg(can_message.number, 6, 10, QChar('0'))
                                 .arg(time_convert.MsToString(can_message.time_stamp))
                                 .arg(can_message.id & EXTENDED_HEADER_SUB_BIT, 8, 16, QChar('0')).toUpper()
                                 .arg(header_type)
                                 .arg(can_message.size, 2, 10, QChar('0'));

    qs += utils::QByteArrayToQString(can_message.payload, can_message.size);

    return qs;
}

const QByteArray MessagesDispatcher::PrepareMessageToSend(quint8 message_type, const QByteArray& data) {
    const int data_size = data.size();
    const quint16 message_len = static_cast<quint16>(data_size + 3);
    quint8 cs = 0;

    QByteArray message;
    message.reserve(data_size + 4);

    // Записываем длину сообщения (2 байта, little-endian).
    message.append(static_cast<char>(message_len)); // Младший байт.
    cs += static_cast<quint8>(message_len);
    message.append(static_cast<char>(message_len >> 8)); // Старший байт.
    cs += static_cast<quint8>(message_len >> 8);

    // Тип сообщения.
    message.append(static_cast<char>(message_type));
    cs += message_type;

    // Копируем данные.
    if (data_size > 0) {
        message.append(data);
        // Обновляем контрольную сумму.
        for (quint8 byte : data) {
            cs += byte;
        }
    }

    // Добавляем контрольную сумму.
    message.append(static_cast<char>(cs));

    return message;
}

// Отправляет сообщение в CAN-шину.
void MessagesDispatcher::SendMessageCan(quint32 can_id, const QByteArray& data) {
    QByteArray result;
    quint16 data_size = data.size();
    // Резервируем место для данных.
    result.reserve(sizeof(can_id) + sizeof(data_size) + data_size);

    QDataStream out(&result, QIODevice::WriteOnly);
    // Задаём порядок байтов.
    out.setByteOrder(QDataStream::LittleEndian);

    // Записываем поля в поток.
    out << can_id << data_size;
    out.writeRawData(data.constData(), data_size);

    // Отправляем устройству.
    SendMessage(message_type::SEND_CAN_MESSAGE, result);
}

QByteArray MessagesDispatcher::PrepareDataForStartEmmulatorOrMonitor() {
    // Добавляем параметры CAN.
    QByteArray result;

    QDataStream out(&result, QIODevice::WriteOnly);
    // Задаём порядок байтов.
    out.setByteOrder(QDataStream::LittleEndian);

    // Записываем параметры CAN в поток.
    out << can_parameters_.baud_rate << static_cast<quint32>(can_parameters_.header_type == EXTENDED_HEADER);

    // Количество id-фильтров в поток.
    out << static_cast<quint32>(hard_filter_by_id_.size());

    // Сами id-фильтры.
    for (const auto& [id, id_parameters] : hard_filter_by_id_) {
        out << id;
    }

    return result;
}

QByteArray MessagesDispatcher::PrepareDataForHardSettings() {
    QByteArray result;

    QDataStream out(&result, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);

    out << hard_settings_.beep_can_need
        << hard_settings_.beep_can_pause;

    return result;
}

void MessagesDispatcher::SetCountAllReciveMessages(quint32 count) {
    view_.CntAllRecMsgLbl()->setText(QString::number(count));
}

void MessagesDispatcher::SetCountCanReciveMessages(quint32 count) {
    view_.CntCanRecMsgLbl()->setText(QString::number(count));
}

void MessagesDispatcher::ClrCountCanReciveMessages() {
    count_can_recive_messages_ = 0;
    SetCountCanReciveMessages(count_can_recive_messages_);
}

std::optional<QColor> MessagesDispatcher::ColoringByCanId(quint32 can_id) {
    auto it = color_by_id_.find(can_id);
    if (it == color_by_id_.end()) {
        return std::nullopt; // Без подкраски.
    }

    return it->second.color;
}

bool MessagesDispatcher::FilteringByCanId(quint32 can_id) {
    if (soft_filter_by_id_.empty()) {
        return true; // Если список пустой, то фильтра нет.
    }
    return soft_filter_by_id_.contains(can_id);
}

// Печать цветного сообщения.
void MessagesDispatcher::PrintMessage(const QString& message, QColor color_back) {
    ReDraw re_draw(view_); // RAII

    if (color_back == Qt::white) {
        view_.GetUi()->text_msg->appendPlainText(message);
        return;
    }

    QTextCharFormat format = view_.GetUi()->text_msg->currentCharFormat();
    format.setBackground(QBrush(color_back));

    view_.GetUi()->text_msg->setCurrentCharFormat(format);
    view_.GetUi()->text_msg->appendPlainText(message);

    format = view_.GetUi()->text_msg->currentCharFormat();
    format.clearBackground();
    view_.GetUi()->text_msg->setCurrentCharFormat(format);
}

void MessagesDispatcher::UpdateSettings() {
    // Обновим настройки.
    time_convert_type_ = settings_.GetValue().time_convert_type;
    can_parameters_ = settings_.GetValue().can_parameters;
    message_coloring_mode_ = settings_.GetValue().message_coloring_mode;
    soft_filter_by_id_ = settings_.GetValue().soft_filter_by_id;
    hard_filter_by_id_ = settings_.GetValue().hard_filter_by_id;
    color_by_id_ = settings_.GetValue().color_by_id;
    hard_settings_ = settings_.GetValue().hard_settings;

    // Информацию по CAN на панель.
    PrintCanInfo(can_parameters_);

    // Записываем настройки в адаптер.
    if (connect_status_) { // Только если ping успешно прошел.
        SendMessage(message_type::SET_HARD_SETTINGS, PrepareDataForHardSettings(), true);
    }
}

void MessagesDispatcher::PrintCanInfo(const CanParameters& can_parameters) {
    QString baud_rate = QString::number(can_parameters.baud_rate / cfg::Settings::BAUD_RATE_COEF);

    view_.SetTextCanInfoLbl("CAN "
                            + baud_rate
                            + " кбод/с, "
                            + can_parameters.header_type);
}

// Работа эммулятора.
void MessagesDispatcher::EmmulatorWork(const CanMessage& can_message) {
    if (can_message.size == 0) {
        return; // Если данных нет, не с чем сравнивать.
    }

    if (can_message.size > 8) {
        qWarning() << "CAN message data size > 8"; // На всякий случай, для справки.
        // Пока, в этой системе размер, сообщения CAN всегда <= 8.
    }

    if (capture_manager_ == nullptr) {
        return; // capture_manager_ не инициализирован.
    }

    // Описание алгоритма.
    // Когда в окне объекта "захвата и ответа" меняются данные захвата,
    // а также начальная позиция и их количество, то формируется строка
    // из id и этих данных и сохраняется в качестве ключа в unordered map (объект capture_manager_).
    // В качестве значения берется указатель на объект "захвата и ответа".
    // Теперь, когда приходит сообщение на шине CAN, то нужно каким-то образом понять,
    // совпадет ли это сообщение с каким-то в этой базе (объект capture_manager_).
    // На этом этапе механизм такой - из пришедшего сообщения также формируется строковый ключ
    // таким-же способом и затем, по этому ключу ищется нужный объект "захвата и ответа".
    // Но есть один момент. Так как нет информации о позиции и количестве данных для захвата,
    // то приходится данные для строкового ключа применять разные по длине, и так пока данные есть.
    // То есть, например, для сообщения с длиной данных = 8, нужно готовить ключ из 8-ми данных + id,
    // потом из из 7-ми данных + id, потом из из 6-ми данных + id и так далее. То есть для сообщения
    // из 8-ми байт максимально потребуется 8 итераций. Если данных будет больше, то итераций потребуется
    // больше. Но данных больше 8-ми пока не ожидается. И в среднем, итераций нужно меньше, совпадение
    // возникнет раньше, так как количество данных для захвата обычно 2 - 6.
    // Для упрощения алгоритма начальная позиция данных для захвата жестко устанавливается в 0 (файл json_storage)
    // строка "capture_answer_data.capture_pos = "0";" и также  на форме поле "capture_position_le" сделано disabled = true.
    // Таким образом, количество итераций для захвата сообщения при любом количестве объектов "захвата и ответа"
    // нужно максимум 8 (а чаще меньше) чем при последоваетльном опросе всех объектов "захвата и ответа", которых может
    // быть очень много (100 и больше).

    // Готовим строку для сравнения.

    // Заполняем массив QByteArray сначала по максимальному размеру.
    QByteArray result;
    QDataStream out(&result, QIODevice::WriteOnly);
    // Задаём порядок байтов.
    out.setByteOrder(QDataStream::LittleEndian);
    // Записываем идентификатор в выходной поток.
    out << can_message.id;
    // Копируем данные payload в выходной поток.
    out.writeRawData(can_message.payload.constData(), can_message.size);
    auto capture_key = QByteArrayToQString(result, utils::stringOption::WITHOUT_SPACES);

    // Текущий механизм поиска предполагает проверки по разной длине строкового ключа.
    CaptureAnswerController* capture_answer_controller = nullptr;
    for (quint32 i = 0; i < can_message.size; ++i) {
        capture_answer_controller = capture_manager_->Find(capture_key);
        if (capture_answer_controller) {
            break;
        }
        capture_key.chop(2); // Укорачиваем на 2 символа в цикле (это один байт в hex).
    }

    if (capture_answer_controller == nullptr) {
        return; // Ключ не найден в списке, выходим.
    }

    auto data = capture_answer_controller->GetPreparedAnswerData();
    if (data == nullptr) {
        return; // Какая-то проблема с данными, выходим.
    }

    if (!capture_answer_controller->GetModel()->GetAnswerId().has_value()) {
        return; // Id не имеет значения.
    }

    // Посылаем данные в CAN.
    SendMessageCan(capture_answer_controller->GetModel()->GetAnswerId().value(), *data);
}
