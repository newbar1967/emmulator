#pragma once

#include <QObject>

class HexEditModel : public QObject
{
    Q_OBJECT

public:
    HexEditModel(QObject* parent = nullptr);
};
