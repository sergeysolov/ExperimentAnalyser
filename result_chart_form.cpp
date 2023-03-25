#include "result_chart_form.h"
#include "ui_result_chart_form.h"

result_chart_form::result_chart_form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::result_chart_form)
{
    ui->setupUi(this);
}

result_chart_form::~result_chart_form()
{
    delete ui;
}
