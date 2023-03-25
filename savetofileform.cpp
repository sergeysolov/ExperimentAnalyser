#include "savetofileform.h"
#include "ui_savetofileform.h"
#include "QMessageBox"

SaveToFileForm::SaveToFileForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SaveToFileForm)
{
    ui->setupUi(this);
    ui->secondsRadioButton->setChecked(true);
    ui->time_checkBox->setChecked(true);
    ui->y_checkBox->setChecked(true);

    ui->comma_radioButton->setChecked(true);
}

SaveToFileForm::~SaveToFileForm()
{
    delete ui;
}

int SaveToFileForm::extract_time_measurement() const
{
    if (ui->secondsRadioButton->isChecked())
        return 0;
    if (ui->minutesRadioButton->isChecked())
        return 1;
    if (ui->hoursRadioButton->isChecked())
        return 2;
    if(ui->daysRadioButton->isChecked())
        return 3;
    return 4;
}

void SaveToFileForm::closeEvent(QCloseEvent *event)
{
    emit form_closed();
    QWidget::closeEvent(event);
}

void SaveToFileForm::on_save_pushButton_clicked()
{
    SaveSettings settings;
    bool at_least_one = false;
    if(ui->time_checkBox->isChecked())
    {
        settings.time = true;
        at_least_one = true;
    }
    if(ui->x_checkBox->isChecked())
    {
        settings.x = true;
        at_least_one = true;
    }
    if (ui->y_checkBox->isChecked())
    {
        settings.y = true;
        at_least_one = true;
    }
    if (not at_least_one)
    {
        QMessageBox::warning(this, tr("Warning"), tr("Choose as least one values type"));
    }
    else
    {
        settings.time_measurement = extract_time_measurement();
        if (ui->comma_radioButton->isChecked())
            settings.comma = true;
        else
            settings.comma = false;
        emit save_to_file(settings);
    }
}


void SaveToFileForm::on_calncel_pushButton_clicked()
{
    this->hide();
    emit form_closed();
}


void SaveToFileForm::on_time_checkBox_stateChanged(int arg1)
{
    ui->timeMeasureGroupBox->setEnabled(arg1);
}


void SaveToFileForm::on_comma_radioButton_clicked()
{

}


void SaveToFileForm::on_dot_radioButton_clicked()
{

}

