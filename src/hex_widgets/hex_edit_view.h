#pragma once

#include <QObject>
#include <QLineEdit>
#include <QString>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QProxyStyle>

class HexEditView : public QLineEdit
{
    Q_OBJECT

public:
    HexEditView(QWidget* parent = nullptr);

signals:
    void KeyPress(const int key, const QString& text);
    void KeyRelease(const int key);
    void MouseRelease();

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};
