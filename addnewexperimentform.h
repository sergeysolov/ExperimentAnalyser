#ifndef ADDNEWEXPERIMENTFORM_H
#define ADDNEWEXPERIMENTFORM_H

#include <QWidget>

namespace Ui {
class AddNewExperimentForm;
}

class AddNewExperimentForm : public QWidget
{
    Q_OBJECT
signals:
    void form_closed(const QString& new_name);

public:
    explicit AddNewExperimentForm(QWidget *parent = nullptr);
    ~AddNewExperimentForm();
    void clear_line_edit();
    void set_focus_line_edit() const;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::AddNewExperimentForm *ui;
};

#endif // ADDNEWEXPERIMENTFORM_H
