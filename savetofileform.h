#ifndef SAVETOFILEFORM_H
#define SAVETOFILEFORM_H

#include "SaveSettings.h"
#include <QWidget>

namespace Ui {
class SaveToFileForm;
}

class SaveToFileForm : public QWidget
{
    Q_OBJECT

public:
    explicit SaveToFileForm(QWidget *parent = nullptr);
    ~SaveToFileForm();
    void load_experiment_names(const QList<QString>& names);

private:
    Ui::SaveToFileForm *ui;
    int extract_time_measurement() const;

signals:
    void form_closed();
    void save_to_file(SaveSettings settings);

private slots:
    void closeEvent(QCloseEvent *event) override;
    void on_save_pushButton_clicked();
    void on_calncel_pushButton_clicked();

    void on_time_checkBox_stateChanged(int arg1);
    void on_comma_radioButton_clicked();
    void on_dot_radioButton_clicked();
    void on_y_checkBox_stateChanged(int arg1);
    void on_x_checkBox_stateChanged(int arg1);
    void on_append_to_excel_file_checkBox_stateChanged(int arg1);
};

#endif // SAVETOFILEFORM_H
