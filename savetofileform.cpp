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
    ui->save_column_titles_checkBox->setChecked(true);

    ui->dot_radioButton->setChecked(true);
}

SaveToFileForm::~SaveToFileForm()
{
    delete ui;
}

void SaveToFileForm::load_experiment_names(const QList<QString> &names)
{
    ui->experiments_listWidget->clear();
    for (const auto& name : names)
    {
        QListWidgetItem* item = new QListWidgetItem();
        QCheckBox* checkBox = new QCheckBox(ui->experiments_listWidget);
        checkBox->setText(name);
        checkBox->setChecked(true);
        ui->experiments_listWidget->addItem(item);
        ui->experiments_listWidget->setItemWidget(item, checkBox);
    }
    //ui->experiments_listWidget->adjustSize();
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
        settings.comma = ui->comma_radioButton->isChecked();
        settings.normalize_y = ui->save_normalized_y_checkBox->isChecked();
        settings.save_norm_column = ui->save_noramization_checkBox->isChecked();
        settings.save_column_titles = ui->save_column_titles_checkBox->isChecked();

        for (int i = 0; i < ui->experiments_listWidget->count(); i++)
        {
            QListWidgetItem* item = ui->experiments_listWidget->item(i);
            QCheckBox* check_box = qobject_cast<QCheckBox*>(ui->experiments_listWidget->itemWidget(item));
            if (check_box->isChecked())
                settings.experiments.insert(i);
        }
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


void SaveToFileForm::on_y_checkBox_stateChanged(int arg1)
{
    ui->save_normalized_y_checkBox->setEnabled(arg1);

    if (not arg1 and not ui->x_checkBox->isChecked())
        ui->experiments_listWidget->setEnabled(false);
    else
        ui->experiments_listWidget->setEnabled(true);
}


void SaveToFileForm::on_x_checkBox_stateChanged(int arg1)
{
    if (not arg1 and not ui->y_checkBox->isChecked())
        ui->experiments_listWidget->setEnabled(false);
    else
        ui->experiments_listWidget->setEnabled(true);
}

