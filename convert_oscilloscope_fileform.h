#ifndef CONVERT_OSCILLOSCOPE_FILEFORM_H
#define CONVERT_OSCILLOSCOPE_FILEFORM_H

#include <QWidget>
#include <QMessageBox>
#include <QStandardPaths>
#include <QFileDialog>
#include <string>
#include <fstream>
#include <OpenXLSX.hpp>
#include <codecvt>

namespace Ui {
class ConvertOscilloscopeFileForm;
}

class ConvertOscilloscopeFileForm : public QWidget
{
    Q_OBJECT

signals:
    void form_closed();

public:
    explicit ConvertOscilloscopeFileForm(QWidget *parent = nullptr);
    ~ConvertOscilloscopeFileForm();

private slots:
    void on_choose_file_pushButton_clicked();
    void on_cancel_pushButton_clicked();

    void closeEvent(QCloseEvent *event) override;

    void on_convert_pushButton_clicked();

private:
    Ui::ConvertOscilloscopeFileForm *ui;
    std::string current_filepath;
    QString prev_open_filepath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString prev_save_filepath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    std::string get_string_measure() const;
    double get_time_divider() const;
};

static QString remove_filename_from_path(const QString &path);
time_t convert_to_seconds(const std::wstring& date, const std::wstring& time);
uint32_t getNumLinesInFile(const std::wstring& filename);
#endif // CONVERT_OSCILLOSCOPE_FILEFORM_H
