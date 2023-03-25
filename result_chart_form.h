#ifndef RESULT_CHART_FORM_H
#define RESULT_CHART_FORM_H

#include <QWidget>

namespace Ui {
class result_chart_form;
}

class result_chart_form : public QWidget
{
    Q_OBJECT

public:
    explicit result_chart_form(QWidget *parent = nullptr);
    ~result_chart_form();

private:
    Ui::result_chart_form *ui;
};

#endif // RESULT_CHART_FORM_H
