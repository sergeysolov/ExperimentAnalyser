#include "experiment_name_form.h"
#include "qevent.h"
#include "ui_experiment_name_form.h"

Experiment_Name_Form::Experiment_Name_Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Experiment_Name_Form)
{
    ui->setupUi(this);
    ui->lineEdit_1->setEnabled(false);
}

Experiment_Name_Form::~Experiment_Name_Form()
{
    delete ui;
}

void Experiment_Name_Form::set_current_name(const QString &name)
{
    ui->lineEdit_1->setText(name);
    ui->lineEdit_2->setText(name);
}

void Experiment_Name_Form::set_colomn(int colomn)
{
    this->colomn = colomn;
}

int Experiment_Name_Form::get_colomn() const
{
    return this->colomn;
}

void Experiment_Name_Form::set_focus_line_edit_2() const
{
    ui->lineEdit_2->setFocus();
}


void Experiment_Name_Form::on_confirm_buttonBox_accepted()
{
    if (ui->lineEdit_2->text().size() != 0)
    {
        emit form_closed(ui->lineEdit_2->text());
        this->hide();
    }
}


void Experiment_Name_Form::on_confirm_buttonBox_rejected()
{
    emit form_closed("");
    this->hide();
}

void Experiment_Name_Form::closeEvent(QCloseEvent* event)
{
    emit form_closed("");
    QWidget::closeEvent(event);
    this->hide();
}

void Experiment_Name_Form::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter or event->key() == Qt::Key_Return)
    {
        if (ui->lineEdit_2->text().size() != 0)
        {
            emit form_closed(ui->lineEdit_2->text());
            this->hide();
        }
    }
    else
        QWidget::keyPressEvent(event);
}


void Experiment_Name_Form::on_clear_pushButton_clicked()
{
    ui->lineEdit_2->clear();
    ui->lineEdit_2->setFocus();
}

