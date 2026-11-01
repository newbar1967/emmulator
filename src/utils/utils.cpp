#include <QCoreApplication>
#include <QLayout>
#include <QFileInfo>
#include <algorithm>
#include "utils.h"

#ifdef Q_OS_WIN
    #include <windows.h>
    #pragma comment(lib, "version.lib")
#endif

namespace utils {

const QString GetAppVersionFromResources() {
#if !defined(Q_OS_WIN)
    QString version_str = "1.0.0.1";
#elif
    QString version_str = "0.0.0.0";
    auto app_filename = QCoreApplication::applicationFilePath().toStdWString();

    UINT size = 0;
    DWORD verHandle = 0;
    LPBYTE lpBuffer = NULL;
    DWORD verSize = GetFileVersionInfoSize(app_filename.data(), &verHandle);
    if (verSize != NULL) {
        LPSTR verData = new char[verSize];
        if (GetFileVersionInfo(app_filename.data(), verHandle, verSize, verData)) {
            if (VerQueryValue(verData, L"\\", (VOID FAR * FAR*) & lpBuffer, &size)) {
                if (size) {
                    VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
                    if (verInfo->dwSignature == 0xfeef04bd) {
                        // Doesn't matter if you are on 32 bit or 64 bit,
                        // DWORD is always 32 bits, so first two revision numbers
                        // come from dwFileVersionMS, last two come from dwFileVersionLS
                        version_str = QString("%1.%2.%3.%4").
                        arg((verInfo->dwFileVersionMS >> 16) & 0xffff).
                        arg((verInfo->dwFileVersionMS >> 0) & 0xffff).
                        arg((verInfo->dwFileVersionLS >> 16) & 0xffff).
                        arg((verInfo->dwFileVersionLS >> 0) & 0xffff);
                    }
                }
            }
        }

      delete [] verData;
    }
#endif

    return version_str;
}

// QByteArray в строку (с пробелами или без).
const QString QByteArrayToQString(const QByteArray& data, stringOption option) {
    QString str_arg;
    option == stringOption::WITH_SPACES ? str_arg = "%1 " : str_arg = "%1";

    QString qs;
    for(int i = 0; i < data.size(); ++i) {
        qs += QString(str_arg).arg(static_cast<uint8_t>(data.at(i)), 2, 16, QChar('0')).toUpper();
    }

    return qs;
}

// QByteArray в строку по количеству байт.
const QString QByteArrayToQString(const QByteArray& data, int size) {
    int count = std::min<int>(data.size(), size);

    QString qs;

    int i = 0;
    for(; i + 1 < count; ++i) {
        qs += QString("%1 ").arg(static_cast<uint8_t>(data.at(i)), 2, 16, QChar('0')).toUpper();
    }
    qs += QString("%1").arg(static_cast<uint8_t>(data.at(i)), 2, 16, QChar('0')).toUpper();

    return qs;
}

// Разбивает строку по пробелам и преобразовывает в QByteArray
const QByteArray QStringToQByteArray(const QString& str) {
    QStringList strList = str.split(QRegularExpression("\\s+"));
    QByteArray byte_array;
    for (auto it = strList.constBegin(); it != strList.constEnd(); ++it) {
        bool ok;
        const uint8_t tmp = it->toUInt(&ok, 16);
        if (ok) {
            byte_array.push_back(tmp);
        }
    }
    return byte_array;
}

// Для применения особого оформления к части строки.
void SetLineEditTextFormat(QLineEdit* lineEdit, const QList<QTextLayout::FormatRange>& formats) {
    if(!lineEdit) {
        return;
    }

    QList<QInputMethodEvent::Attribute> attributes;
    for(const QTextLayout::FormatRange& fr : formats) {
        QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;
        int start = fr.start - lineEdit->cursorPosition();
        int length = fr.length;
        QVariant value = fr.format;
        attributes.append(QInputMethodEvent::Attribute(type, start, length, value));
    }
    QInputMethodEvent event(QString(), attributes);
    QCoreApplication::sendEvent(lineEdit, &event);
}

// Отменяет особое оформление части строки.
void ClearLineEditTextFormat(QLineEdit* lineEdit) {
    SetLineEditTextFormat(lineEdit, QList<QTextLayout::FormatRange>());
}

// Проверяет входную строку на наш Hex-формат.
bool CheckTextBytesFormat(const QString& str) {
    if (str.isEmpty()) {
        return false;
    }

    QString tmp_str = str.simplified();

    QStringList strList = tmp_str.split(QRegularExpression("\\s+"));
    for (auto it = strList.constBegin(); it != strList.constEnd(); ++it) {
        if (it->size() > 2) {
            return false;
        }

        bool ok;
        const uint32_t tmp = it->toUInt(&ok, 16);
        if (!ok || tmp > 255) {
            return false;
        }
    }

    return true;
}

// Строка может содержать только символы 0-9, A-F.
bool IsHexString(const QString& str) {
    if (str.isEmpty()) {
        return false;
    }

    for (const QChar& c : str) {
        ushort code = c.unicode();
        if (!((code >= '0' && code <= '9')
              || (code >= 'A' && code <= 'F')
              || (code >= 'a' && code <= 'f')
              )) {
            return false;
        }
    }
    return true;
}

// Строка может содержать только символы 0-9.
bool IsIntString(const QString& str) {
    if (str.isEmpty()) {
        return false;
    }

    for (const QChar& c : str) {
        ushort code = c.unicode();
        if (!(code >= '0' && code <= '9')) {
            return false;
        }
    }
    return true;
}

// Строка может содержать только символы 0-9 и одну точку (double).
bool IsDoubleString(const QString& str) {
    if (str.isEmpty()) {
        return false;
    }

    bool has_point = false;
    bool has_digit = false;

    for (int i = 0; i < str.length(); ++i) {
        ushort code = str[i].unicode();

        if (code >= '0' && code <= '9') {
            has_digit = true;
        }
        else if (code == '.') {
            if (has_point) {
                return false; // Вторая точка — ошибка.
            }
            has_point = true;
        }
        else {
            return false; // Любой другой символ — ошибка.
        }
    }

    return has_digit; // Должна быть хотя бы одна цифра.
}

QByteArray CreateQByteArray(std::initializer_list<quint8> bytes) {
    QByteArray result;
    for (quint8 byte : bytes) {
        result.append(static_cast<char>(byte));
    }
    return result;
}

QWidget* FindTopLevelWidget(QLayout* layout) {
    QObject* current = layout;
    while (current) {
        QWidget* asWidget = qobject_cast<QWidget*>(current);
        if (asWidget && asWidget->isWindow()) {
            return asWidget; // Нашли окно верхнего уровня
        }
        current = current->parent();
    }
    return nullptr;
}

} // namespace utils
