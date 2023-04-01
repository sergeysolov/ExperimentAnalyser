#include "addnewexperimentform.h"
#include "qevent.h"
#include "ui_addnewexperimentform.h"

AddNewExperimentForm::AddNewExperimentForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddNewExperimentForm)
{
    ui->setupUi(this);
}

AddNewExperimentForm::~AddNewExperimentForm()
{
    delete ui;
}

void AddNewExperimentForm::clear_line_edit()
{
    ui->lineEdit->clear();
}

void AddNewExperimentForm::set_focus_line_edit() const
{
    ui->lineEdit->setFocus();
}

void AddNewExperimentForm::on_buttonBox_accepted()
{
    if (ui->lineEdit->text().size() != 0)
    {
        emit form_closed(ui->lineEdit->text());
        this->hide();
    }
}


void AddNewExperimentForm::on_buttonBox_rejected()
{
    emit form_closed("");
    this->hide();
}

void AddNewExperimentForm::closeEvent(QCloseEvent *event)
{
    emit form_closed("");
    QWidget::closeEvent(event);
    this->hide();
}

void AddNewExperimentForm::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter or event->key() == Qt::Key_Return)
    {
        if (ui->lineEdit->text().size() != 0)
        {
            emit form_closed(ui->lineEdit->text());
            this->hide();
        }
    }
    else
        QWidget::keyPressEvent(event);
}

void AddNewExperimentForm::on_clear_pushButton_clicked()
{
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
}

