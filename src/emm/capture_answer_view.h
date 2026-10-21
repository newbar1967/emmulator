#pragma once

#include <QWidget>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QFont>
#include <QLineEdit>

namespace Ui {
class CaptureAnswerView;
}

namespace emm {

class CaptureAnswerView : public QWidget
{
    Q_OBJECT

public:
    explicit CaptureAnswerView(QWidget *parent = nullptr);
    ~CaptureAnswerView();

    Ui::CaptureAnswerView* GetUi() {
        return ui_;
    }

private:
    enum class validatorType {
        HEX,
        INT
    };

    void SetupLineEdit(QLineEdit *lineEdit,
                       validatorType validator_type,
                       int max_length);
    QRegularExpressionValidator* CreateHexValidator();
    QRegularExpressionValidator* CreateIntValidator();

private:
    static constexpr int EXTRA_WIDTH = 12;
    Ui::CaptureAnswerView *ui_;
    QRegularExpressionValidator* hex_validator_;
    QRegularExpressionValidator* int_validator_;
};

} // namespace emm
