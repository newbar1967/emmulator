#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include <QTimer>
#include "view.h"
#include "udp_server.h"
#include "controller.h"
#include "utils.h"
#include "settings.h"
#include "config.h"
#include "json_storage.h"
#include "messages_dispatcher.h"

namespace {

static const QString CONFIG_FILE_NAME = "config.json";

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Newbar Company");
    QApplication::setApplicationName("Эмулятор ЭБУ (Ethernet <-> CAN)");
    QApplication::setApplicationVersion(utils::GetAppVersionFromResources());
    QThread::currentThread()->setPriority(QThread::HighPriority);
    app.setStyleSheet("QStatusBar::item { border: none; }");

    MainWindow main_window;
    cfg::Settings settings(&main_window);
    cfg::Config config;
    cfg::JsonStorage json_storage(settings, config);
    server::UdpServer udp_server(settings);
    MessagesDispatcher messages_dispather(main_window,
                          udp_server,
                          settings);
    Controller controller(main_window,
                          messages_dispather,
                          settings,
                          json_storage,
                          config);

    // Подключаем обработчик сохранения перед выходом.
    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                    [&json_storage, &controller]() {
                        controller.CaptureAnswerDatasToConfig();
                        auto config_path = json_storage.GetConfigPath();
                        if (!config_path.has_value()
                            || !json_storage.Save(config_path.value() + "/" + CONFIG_FILE_NAME)) {
                            qWarning() << "Ошибка сохранения последней конфигурации";
                        }
                    });

    main_window.show();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    // Загружаем конфигурацию из файла.

    auto config_path = json_storage.GetConfigPath();
    if(!config_path.has_value()
        || !json_storage.Load(config_path.value() + "/" + CONFIG_FILE_NAME)) {
        settings.Update(); // Настройки будут по-умолчанию.
        QMessageBox::information(QApplication::activeWindow(), "Информация",
                                 "Ошибка загрузки последней конфигурации");
    } else {
        // Обновление конфигурации.
        config.Update();
        // Обновление настроек.
        settings.Update();
    }

    return app.exec();
}
