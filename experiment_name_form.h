#ifndef EXPERIMENT_NAME_FORM_H
#define EXPERIMENT_NAME_FORM_H

#include <QWidget>

namespace Ui {
class Experiment_Name_Form;
}

class Experiment_Name_Form : public QWidget
{
    Q_OBJECT

signals:
    void form_closed(const QString& new_name);

public:

    explicit Experiment_Name_Form(QWidget *parent = nullptr);
    ~Experiment_Name_Form();
    void set_current_name(const QString& name);
    void set_colomn(int colomn);
    int get_colomn() const;
    void set_focus_line_edit_2() const;

private slots:
    void on_confirm_buttonBox_accepted();
    void on_confirm_buttonBox_rejected();

    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    int colomn;
    Ui::Experiment_Name_Form *ui;
};

#endif // EXPERIMENT_NAME_FORM_H
