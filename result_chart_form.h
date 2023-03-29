#ifndef RESULT_CHART_FORM_H
#define RESULT_CHART_FORM_H

#include <memory>

#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QtCharts>
#include <QtCharts/QValueAxis>

namespace Ui {
class result_chart_form;
}

class result_chart_form : public QWidget
{
    Q_OBJECT
signals:
    void form_closed();

public:
    explicit result_chart_form(QWidget *parent = nullptr);
    QChart* get_chart() const;
    ~result_chart_form();

private:
    Ui::result_chart_form *ui;
    std::unique_ptr<QChart> chart;
    std::unique_ptr<QChartView> chart_view;
    void closeEvent(QCloseEvent *event) override;
};

#endif // RESULT_CHART_FORM_H
