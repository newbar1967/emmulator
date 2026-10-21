QT += core gui widgets network
QT += widgets

CONFIG += c++20

VERSION = 1.0.0.1

INCLUDEPATH += \
            $$PWD/src \
            $$PWD/src/emm \
            $$PWD/src/server \
            $$PWD/src/utils \
            $$PWD/src/qlineedit_child \
            $$PWD/src/cfg \
            $$PWD/src/hex_widgets \
            $$PWD/src/my_widgets

SOURCES += \
    src/cfg/color_filter.cpp \
    src/cfg/config.cpp \
    src/cfg/filter.cpp \
    src/cfg/settings.cpp \
    src/cfg/settings_view.cpp \
    src/controller.cpp \
    src/emm/capture_manager.cpp \
    src/emm/capture_answer_controller.cpp \
    src/emm/capture_answer_model.cpp \
    src/emm/capture_answer_view.cpp \
    src/cfg/json_storage.cpp \
    src/hex_widgets/hex_edit_controller.cpp \
    src/hex_widgets/hex_edit_model.cpp \
    src/hex_widgets/hex_edit_view.cpp \
    src/main.cpp \
    src/messages_dispatcher.cpp \
    src/my_widgets/bits_widget.cpp \
    src/server/udp_buffer.cpp \
    src/server/udp_server.cpp \
    src/server/udp_server_worker.cpp \
    src/utils/time_convert.cpp \
    src/utils/utils.cpp \
    src/view.cpp

HEADERS += \
    src/cfg/color_filter.h \
    src/cfg/config.h \
    src/cfg/filter.h \
    src/cfg/settings.h \
    src/cfg/settings_view.h \
    src/controller.h \
    src/messages_dispatcher.h \
    src/my_widgets/can_id_widget.h \
    src/my_widgets/color_square.h \
    src/server/udp_buffer.h \
    src/server/udp_server_worker.h \
    src/utils/elapsed_time.h \
    src/emm/capture_manager.h \
    src/emm/capture_answer_controller.h \
    src/emm/capture_answer_model.h \
    src/emm/capture_answer_view.h \
    src/cfg/json_storage.h \
    src/cfg/storage.h \
    src/hex_widgets/hex_edit_controller.h \
    src/hex_widgets/hex_edit_model.h \
    src/hex_widgets/hex_edit_view.h \
    src/my_widgets/bits_widget.h \
    src/server/udp_server.h \
    src/utils/string_converter.h \
    src/utils/time_convert.h \
    src/utils/utils.h \
    src/view.h

FORMS += \
    src/cfg/can_id_widget.ui \
    src/cfg/settings_view.ui \
    src/emm/capture_answer_view.ui \
    src/view.ui

RC_FILE = emmulator_resource.rc

RESOURCES += \
    images.qrc


