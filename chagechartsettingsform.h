#ifndef CHAGECHARTSETTINGSFORM_H
#define CHAGECHARTSETTINGSFORM_H

#include <QWidget>
#include "AxesRange.h"

namespace Ui {
class chageChartSettingsForm;
}

class chageChartSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit chageChartSettingsForm(QWidget *parent = nullptr);
    ~chageChartSettingsForm();

    void set_spin_boxes_values(AxesRange range);
    void set_Y_spin_boxes_step(double step_min, double step_max);
    void set_Y_spin_boxes_decimals(unsigned decimals_min, unsigned decimals_max);
signals:
    void returnResult(std::pair<bool, AxesRange> result);


private slots:
    void on_manualSettings_radioButton_clicked();
    void on_fitToLines_radioButton_clicked();
    void on_defaultAxes_radioButton_clicked();

    void closeEvent(QCloseEvent *event);

    void on_okCancel_buttonBox_accepted();

    void on_okCancel_buttonBox_rejected();

private:
    AxesMode get_current_mode() const;
    void set_enabled_spin_boxes(bool is_active);
    AxesRange get_values_from_spin_boxes() const;

    Ui::chageChartSettingsForm *ui;
};

#endif // CHAGECHARTSETTINGSFORM_H
