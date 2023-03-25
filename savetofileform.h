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

private:
    Ui::SaveToFileForm *ui;
    int extract_time_measurement() const;

signals:
    void form_closed();
    void save_to_file(SaveSettings settings);

private slots:
    void closeEvent(QCloseEvent *event);
    void on_save_pushButton_clicked();
    void on_calncel_pushButton_clicked();

    void on_time_checkBox_stateChanged(int arg1);
    void on_comma_radioButton_clicked();
    void on_dot_radioButton_clicked();
};

#endif // SAVETOFILEFORM_H
