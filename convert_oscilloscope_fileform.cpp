#include "convert_oscilloscope_fileform.h"
#include "ui_convert_oscilloscope_fileform.h"

ConvertOscilloscopeFileForm::ConvertOscilloscopeFileForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConvertOscilloscopeFileForm)
{
    ui->setupUi(this);
    ui->convert_pushButton->setEnabled(false);
    ui->progressBar->hide();
    ui->secondsRadioButton->setChecked(true);
    ui->normalize_checkBox->setChecked(true);
}

ConvertOscilloscopeFileForm::~ConvertOscilloscopeFileForm()
{
    delete ui;
}


void ConvertOscilloscopeFileForm::on_choose_file_pushButton_clicked()
{
    QString filepath = QFileDialog::getOpenFileName(this, tr("Open File"), prev_open_filepath, tr("Text Files (*.txt);;All Files (*.*)"));
    if (filepath != "")
    {
        prev_open_filepath = remove_filename_from_path(filepath);
        ui->convert_pushButton->setEnabled(true);
        ui->file_path_label->setText(filepath);
    }
}

QString remove_filename_from_path(const QString &path)
{
    int i = 0;
    for (auto it = path.rbegin(); it != path.crend(); ++it, i++)
        if (*it == '/')
            break;
    return path.mid(0, path.size() - i);
}


void ConvertOscilloscopeFileForm::on_cancel_pushButton_clicked()
{
    emit form_closed();
    this->hide();
}

void ConvertOscilloscopeFileForm::closeEvent(QCloseEvent *event)
{
    emit form_closed();
    QWidget::closeEvent(event);
}


void ConvertOscilloscopeFileForm::on_convert_pushButton_clicked()
{
    int save_every_value = ui->spinBox->value();
    ui->spinBox->setEnabled(false);
    ui->progressBar->show();
    ui->progressBar->setValue(0);
    int lines_number = getNumLinesInFile(ui->file_path_label->text().toStdWString());

    QString saveFilePath = QFileDialog::getSaveFileName(nullptr, tr("Save File"), prev_save_filepath, "Excel Files (*.xlsx)");
    if (saveFilePath == "")
    {
        ui->spinBox->setEnabled(true);
        return;
    }
    prev_save_filepath = remove_filename_from_path(saveFilePath);

    std::wifstream file;
    file.open(ui->file_path_label->text().toStdWString());

    if (not file.is_open() or lines_number == 0)
    {
        QMessageBox::critical(this, tr("Error"), tr("File reading error"));
        ui->spinBox->setEnabled(true);
        return;
    }

    try
    {
        std::vector<std::pair<double, double>> data;
        data.reserve(lines_number / save_every_value);
        double max_value = -1E+15;

        std::vector<std::wstring> buffer;
        time_t prev_time;
        std::wstring date, time, value;

        file >> date >> time >> value;
        time_t initial_time = prev_time = convert_to_seconds(date, time);
        buffer.push_back(value);

        for (size_t i = 0; not file.eof(); i++)
        {
            file >> date >> time >> value;
            if (i % save_every_value == 0)
            {
                time_t seconds = convert_to_seconds(date, time);
                if (seconds == prev_time)
                {
                    buffer.push_back(value);
                }
                else
                {
                    double delta_t = static_cast<double>(seconds - prev_time) / buffer.size();
                    for (size_t j = 0; j < buffer.size(); j++)
                    {
                        data.emplace_back((prev_time - initial_time) + j * delta_t, std::stod(buffer[j]));
                        max_value = std::max(max_value, std::stod(buffer[j]));
                    }
                    buffer.clear();
                    buffer.push_back(value);
                    prev_time = seconds;
                }

                int progress_bar_value = (i / static_cast<double>(lines_number)) * 50;
                if (progress_bar_value > ui->progressBar->value())
                    ui->progressBar->setValue(progress_bar_value);
            }

        }
        double delta_t = 1.0f / buffer.size();
        for (size_t j = 0; j < buffer.size(); j++)
        {
            data.emplace_back((prev_time - initial_time) + j * delta_t, std::stod(buffer[j]));
            max_value = std::max(max_value, std::stod(buffer[j]));
        }

        OpenXLSX::XLDocument exel_doc;
        exel_doc.create(saveFilePath.toStdString());
        auto worksheet = exel_doc.workbook().worksheet("Sheet1");

        worksheet.cell(1, 1).value() = "time, " + get_string_measure();
        worksheet.cell(1, 2).value() = "Amplitude";

        double time_divider = get_time_divider();
        bool requres_normalization = ui->normalize_checkBox->isChecked();
        for (uint32_t i = 0; i < data.size(); i++)
        {
            double amplitude = data[i].second;
            if (requres_normalization)
                amplitude /= max_value;

            worksheet.cell(i + 2, 1).value() = data[i].first / time_divider;
            worksheet.cell(i + 2, 2).value() = amplitude;

            int progress_bar_value = 50 + (i / static_cast<double>(lines_number)) * 50;
            if (progress_bar_value > ui->progressBar->value())
                ui->progressBar->setValue(progress_bar_value);
        }

        exel_doc.save();
        exel_doc.close();
        QMessageBox::information(this, tr("File saving"), tr("Complete!"));
        ui->progressBar->setValue(100);

    } catch (...)
    {
        QMessageBox::critical(this, tr("Error"), tr("File saving error"));        
    }
    ui->spinBox->setEnabled(true);
    file.close();   
}

std::string ConvertOscilloscopeFileForm::get_string_measure() const
{
    if (ui->secondsRadioButton->isChecked())
        return "s";
    if (ui->minutesRadioButton->isChecked())
        return "m";
    if (ui->hoursRadioButton->isChecked())
        return "h";
    return "d";
}

double ConvertOscilloscopeFileForm::get_time_divider() const
{
    if (ui->secondsRadioButton->isChecked())
        return 1;
    if (ui->minutesRadioButton->isChecked())
        return 60;
    if (ui->hoursRadioButton->isChecked())
        return 60 * 60;
    return 60 * 60 * 24;
}

time_t convert_to_seconds(const std::wstring &date, const std::wstring &time)
{
    int day, month, year;
    swscanf_s(date.c_str(), L"%d.%d.%d", &day, &month, &year);

    int hour, min, sec;
    swscanf_s(time.c_str(), L"%d:%d:%d", &hour, &min, &sec);

    struct tm tm_time = { 0 };
    tm_time.tm_year = year - 1900;
    tm_time.tm_mon = month - 1;
    tm_time.tm_mday = day;
    tm_time.tm_hour = hour;
    tm_time.tm_min = min;
    tm_time.tm_sec = sec;

    return mktime(&tm_time);
}

uint32_t getNumLinesInFile(const std::wstring& filename)
{
    std::wifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        // Handle file open error
        return 0;
    }
    // Get the current position of the file pointer
    std::streampos fileSize = file.tellg();

    std::wifstream simple_file(filename);
    if (!simple_file.is_open()) {
        // Handle file open error
        return 0;
    }
    std::wstring first_str;
    std::getline(simple_file, first_str);
    size_t avgLineLength = first_str.size() + 1;

    // Estimate the number of lines in the file
    uint32_t numLines = (uint32_t)(fileSize / avgLineLength);

    file.close();
    simple_file.close();

    return numLines;
}
