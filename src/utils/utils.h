#pragma once

#include <QRegularExpression>
#include <QByteArray>
#include <QString>
#include <QTextLayout>
#include <QLineEdit>

namespace utils {

enum class stringOption {
    WITH_SPACES,
    WITHOUT_SPACES
};

const QString GetAppVersionFromResources();
const QString QByteArrayToQString(const QByteArray& data,
                                  stringOption option = stringOption::WITH_SPACES);
const QString QByteArrayToQString(const QByteArray& data, int size);
const QByteArray QStringToQByteArray(const QString& str);
void SetLineEditTextFormat(QLineEdit* lineEdit, const QList<QTextLayout::FormatRange>& formats);
void ClearLineEditTextFormat(QLineEdit* lineEdit);
bool CheckTextBytesFormat(const QString& str);
bool IsHexString(const QString& str);
bool IsIntString(const QString& str);
bool IsDoubleString(const QString& str);
QByteArray CreateQByteArray(std::initializer_list<quint8> bytes);

} // namespace utils
