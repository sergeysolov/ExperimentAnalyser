#include "result_chart_form.h"
#include "ui_result_chart_form.h"

result_chart_form::result_chart_form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::result_chart_form), chart(new QChart()), chart_view(new QChartView())
{
    ui->setupUi(this);
    chart->setParent(ui->horizontalFrame);
    chart->legend()->setVisible(true);
    chart->setAnimationOptions(QChart::AllAnimations);
    chart_view->setChart(chart.get());
    chart_view->setFixedSize(ui->horizontalFrame->size().width(), ui->horizontalFrame->size().height());
    chart_view->setRenderHint(QPainter::Antialiasing);
    ui->horizontalLayout->addWidget(chart_view.get());
}

QChart* result_chart_form::get_chart() const
{
    return chart.get();
}

result_chart_form::~result_chart_form()
{
    delete ui;
}

void result_chart_form::closeEvent(QCloseEvent *event)
{
    emit form_closed();
    QWidget::closeEvent(event);
}
