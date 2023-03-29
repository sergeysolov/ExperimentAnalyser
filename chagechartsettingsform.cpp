#include "chagechartsettingsform.h"
#include "ui_chagechartsettingsform.h"

chageChartSettingsForm::chageChartSettingsForm(QWidget *parent) :
    QWidget(parent), ui(new Ui::chageChartSettingsForm)
{
    ui->setupUi(this);
    ui->defaultAxes_radioButton->setChecked(true);
    set_enabled_spin_boxes(false);

    ui->minXdoubleSpinBox->setKeyboardTracking(true);
    ui->minXdoubleSpinBox->setLocale(QLocale::English);

    ui->maxXdoubleSpinBox->setKeyboardTracking(true);
    ui->maxXdoubleSpinBox->setLocale(QLocale::English);

    ui->minYdoubleSpinBox->setKeyboardTracking(true);
    ui->minYdoubleSpinBox->setLocale(QLocale::English);

    ui->maxYdoubleSpinBox->setKeyboardTracking(true);
    ui->maxYdoubleSpinBox->setLocale(QLocale::English);
}

chageChartSettingsForm::~chageChartSettingsForm()
{
    delete ui;
}

void chageChartSettingsForm::set_spin_boxes_values(AxesRange range)
{
    ui->minXdoubleSpinBox->setValue(range.min_x);
    ui->maxXdoubleSpinBox->setValue(range.max_x);
    ui->minYdoubleSpinBox->setValue(range.min_y);
    ui->maxYdoubleSpinBox->setValue(range.max_y);
}

void chageChartSettingsForm::set_Y_spin_boxes_step(double step_min, double step_max)
{
    ui->minYdoubleSpinBox->setSingleStep(step_min);
    ui->maxYdoubleSpinBox->setSingleStep(step_max);
}

void chageChartSettingsForm::set_Y_spin_boxes_decimals(unsigned int decimals_min, unsigned int decimals_max)
{
    ui->minYdoubleSpinBox->setDecimals(decimals_min);
    ui->maxYdoubleSpinBox->setDecimals(decimals_max);
}


void chageChartSettingsForm::set_enabled_spin_boxes(bool is_active)
{
    ui->maxXdoubleSpinBox->setEnabled(is_active);
    ui->minXdoubleSpinBox->setEnabled(is_active);
    ui->maxYdoubleSpinBox->setEnabled(is_active);
    ui->minYdoubleSpinBox->setEnabled(is_active);
}

AxesRange chageChartSettingsForm::get_values_from_spin_boxes() const
{
    AxesRange range;
    range.min_x = ui->minXdoubleSpinBox->value();
    range.max_x = ui->maxXdoubleSpinBox->value();
    range.min_y = ui->minYdoubleSpinBox->value();
    range.max_y = ui->maxYdoubleSpinBox->value();
    return range;
}

void chageChartSettingsForm::on_manualSettings_radioButton_clicked()
{
    set_enabled_spin_boxes(true);
}


void chageChartSettingsForm::on_fitToLines_radioButton_clicked()
{
    set_enabled_spin_boxes(false);
}


void chageChartSettingsForm::on_defaultAxes_radioButton_clicked()
{
    set_enabled_spin_boxes(false);
}

void chageChartSettingsForm::closeEvent(QCloseEvent *event)
{
    AxesRange range = get_values_from_spin_boxes();
    emit returnResult({false, range});
    QWidget::closeEvent(event);
}

void chageChartSettingsForm::on_okCancel_buttonBox_accepted()
{
    AxesRange range = get_values_from_spin_boxes();
    range.mode = get_current_mode();
    emit returnResult({true, range});
    this->hide();
}


void chageChartSettingsForm::on_okCancel_buttonBox_rejected()
{
    AxesRange range = get_values_from_spin_boxes();
    emit returnResult({false, range});
    this->hide();
}

AxesMode chageChartSettingsForm::get_current_mode() const
{
    if (ui->manualSettings_radioButton->isChecked())
        return AxesMode::Manual;
    if (ui->fitToLines_radioButton->isChecked())
        return AxesMode::FitToLine;
    return AxesMode::Default;
}

