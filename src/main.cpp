#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include <QTimer>
#include <QDir>
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
#if !defined(Q_OS_WIN)
    QApplication app(argc, argv);
    const QStringList args = QCoreApplication::arguments();
    if (args.contains("--version")) {
        QTextStream(stdout) << utils::GetAppVersionFromResources() << Qt::endl;
        return 0;
    }
    QApplication::setApplicationVersion(utils::GetAppVersionFromResources());
    app.setWindowIcon(QIcon(":/images/emmulator.png"));
#else
    QApplication app(argc, argv);
    QApplication::setApplicationVersion(utils::GetAppVersionFromResources());
#endif
    QCoreApplication::setOrganizationName("Newbar Company");
    QApplication::setApplicationName("Эмулятор ЭБУ (Ethernet <-> CAN)");
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
                        if (!config_path.has_value()) {
                            qWarning() << "Не удалось получить путь к последней конфигурации";
                            return;
                        }

                        QDir dir(config_path.value());
                        QString full_path = dir.absoluteFilePath(CONFIG_FILE_NAME);

                        if (!json_storage.Save(full_path)) {
                            qWarning() << "Ошибка сохранения последней конфигурации";
                        }
                    });

    main_window.show();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    // Загружаем конфигурацию из файла.
    auto config_path = json_storage.GetConfigPath();
    if(config_path.has_value()) {
        QDir dir(config_path.value());
        QString full_path = dir.absoluteFilePath(CONFIG_FILE_NAME);

        if (json_storage.Load(full_path)) {
            // Обновление конфигурации.
            config.Update();
            // Обновление настроек.
            settings.Update();

            return app.exec();
        }
    }

    settings.Update(); // Настройки будут по-умолчанию.
    QMessageBox::information(QApplication::activeWindow(), "Информация",
                         "Ошибка загрузки последней конфигурации");

    return app.exec();
}
