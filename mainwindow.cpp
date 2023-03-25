#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <qlabel.h>
#include <string>
#include <fstream>
#include <QMessageBox>
#include <sstream>
#include <QDebug>

AxesRange current_range;

constexpr unsigned Y_tick_count = 14, Y_minor_tick_count = 1;

constexpr float default_min_X = 350, default_max_X = 750;
constexpr float default_min_Y = -100;
constexpr std::pair<float, float> max_range = {500, 650};

constexpr std::array<QColor, 10> colors = { QColor(31, 119, 180), QColor(255, 127, 14), QColor(44, 160, 44),
                      QColor(214, 39, 40), QColor(148, 103, 189), QColor(140, 86, 75),
                      QColor(227, 119, 194), QColor(127, 127, 127), QColor(188, 189, 34),
                      QColor(23, 190, 207) };

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), result_form(new result_chart_form()), ui(new Ui::MainWindow), chart_settings_form(new chageChartSettingsForm()), save_to_file_form(new SaveToFileForm)
{
    ui->setupUi(this);
    this->current_data_table_widget.table_widget = ui->current_table_listWidget;
    ui->current_table_listWidget->setColumnWidth(0, 62);
    ui->current_table_listWidget->setColumnWidth(1, 50);
    ui->current_table_listWidget->setLocale(QLocale::English);

    //Replacing double_spin_box
    double_spin_box_container.reset(new DoubleSpinBoxContainer(ui->timeGroupBox));
    double_spin_box_container->setKeyboardTracking(true);
    double_spin_box_container->setLocale(QLocale::English);

    double_spin_box_container->setRange(ui->timeNumberDoubleSpinBox->minimum(), ui->timeNumberDoubleSpinBox->maximum());
    double_spin_box_container->setValue(ui->timeNumberDoubleSpinBox->value());
    double_spin_box_container->setSingleStep(ui->timeNumberDoubleSpinBox->singleStep());
    double_spin_box_container->setDecimals(ui->timeNumberDoubleSpinBox->decimals());
    double_spin_box_container->move(10, 30);

    delete ui->timeNumberDoubleSpinBox;
    ui->timeNumberDoubleSpinBox = double_spin_box_container.get();
    connect(double_spin_box_container.get(), &DoubleSpinBoxContainer::key_plus_pressed, this, &MainWindow::key_plus_pressed_handle_slot);

    chart_settings_form->setWindowFlags(chart_settings_form->windowFlags() & ~Qt::WindowMaximizeButtonHint);
    connect(chart_settings_form.get(), &chageChartSettingsForm::returnResult, this, &MainWindow::change_chart_setting_form_closed);

    connect(ui->actionChange_axes_range, &QAction::triggered, this, &MainWindow::actionChange_axes_range_triggered);
    connect(ui->actionSave_to_file, &QAction::triggered, this, &MainWindow::action_save_to_file_triggered);

    connect(ui->actionAmplitude_2, &QAction::triggered, this, &MainWindow::action_change_Y_name_to_amplitude);
    connect(ui->actionAbsorbtion, &QAction::triggered, this, &MainWindow::action_change_Y_name_to_absorbtion);
    connect(ui->actionReflection, &QAction::triggered, this, &MainWindow::action_change_Y_name_to_reflection);

    connect(save_to_file_form.get(), &SaveToFileForm::form_closed, this, &MainWindow::save_to_file_form_closed);
    connect(save_to_file_form.get(), &SaveToFileForm::save_to_file, this, &MainWindow::save_to_file_slot);
    save_to_file_form->setWindowFlags(save_to_file_form->windowFlags() & ~Qt::WindowMaximizeButtonHint);

    connect(ui->actionOpen_file, &QAction::triggered, this, &MainWindow::on_pushButton_clicked);

    ui->secondsRadioButton->setChecked(true);

    ui->currentFile_const_label->hide();

    chart_set = new ChartSet(ui);  

    QObject::connect(this, &MainWindow::currentMeasureChangedInTable, this, &MainWindow::handleCurrentMeasureChangedInTable);

    QLabel *label = this->findChild<QLabel*>("coord_label");
    if (label)
        this->coord_label = label;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete chart_set;
    delete coord_label;
}


void MainWindow::on_pushButton_clicked()
{   
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), prev_filepath, tr("Text Files (*.txt);;All Files (*.*)"));
    prev_filepath = remove_filename_from_path(filename);
    std::wifstream file(filename.toStdWString());

    try
    {
        if (file.is_open())
        {
            ui->currentFile_const_label->show();
            auto reduced_filename = extract_file_name_from_path(filename);
            ui->currentFile_label->setText(reduced_filename);


            float max_value = 0;
            float min_x = 1E+15f, max_x = -1E+15f;
            float min_y = 1E+15f, max_y = -1E+15f;
            int colomns_number = 0;

            std::wstring line;
            line.reserve(100);
            std::getline(file, line);

            std::wstringstream initial_stream(line);
            while(not initial_stream.eof())
            {
                std::wstring num;
                initial_stream >> num;
                colomns_number++;
            }

            std::vector<QLineSeries> temp_qline_series_array(colomns_number / 2);

            while (std::getline(file, line))
            {
                std::wstringstream line_stream(line);
                std::wstring num_x, num_y;

                for (int i = 0; i < colomns_number; i += 2)
                {
                    line_stream >> num_x >> num_y;
                    float x = convert_string_comma_to_float(num_x);
                    float y = convert_string_comma_to_float(num_y);
                    max_x = std::max(x, max_x);
                    min_x = std::min(x, min_x);
                    max_y = std::max(y, max_y);
                    min_y = std::min(y, min_y);

                    if (x > max_range.first and x < max_range.second)
                        max_value = std::max(max_value, y);
                    temp_qline_series_array[i / 2].append(x, y);
                }
            }

            // Preparing to build chart
            ui->opend_files_listWidget->addItem(reduced_filename);
            {
                QSignalBlocker blocker(ui->opend_files_listWidget);
                ui->opend_files_listWidget->setCurrentRow(ui->opend_files_listWidget->count() - 1);
            }

            AxesRange fit_to_line_range{min_x, max_x, min_y, max_y};
            auto axes = ChartSet::createQValueAxes(fit_to_line_range, max_value);

            chart_data_list.emplace_back(std::move(temp_qline_series_array),fit_to_line_range, max_value, axes.first, axes.second, "Amplitude");
            chart_set->show_plots(chart_data_list.back());
            current_chart_data_index = chart_data_list.size() - 1;
        }
    }
    catch(...)
    {
        QMessageBox::critical(this, tr("Error"), tr("File reading error"));
    }
    file.close();
}

ChartSet::ChartSet(Ui::MainWindow* ui)
{
    chart.setParent(ui->horizontalFrame);
    chart.setAnimationOptions(QChart::AllAnimations);
    chart.legend()->setVisible(true);
    chart.setTheme(QChart::ChartTheme::ChartThemeLight);

    chartView.setUI(ui);
    chartView.setChart(&chart);
    chartView.setFixedSize(ui->horizontalFrame->size().width(), ui->horizontalFrame->size().height());
    ui->horizontalLayout->addWidget(&chartView);
}

std::pair<QValueAxis *, QValueAxis *> ChartSet::createQValueAxes(AxesRange fit_to_line_range, float default_max_Y)
{
    float min_x = fit_to_line_range.min_x, max_x = fit_to_line_range.max_x;
    float min_y = fit_to_line_range.min_y, max_y = fit_to_line_range.max_y;

    QValueAxis* axis_X = new QValueAxis();
    axis_X->setTitleText("Wawelength, nm");
    if (current_range.mode == AxesMode::Default)
    {
        axis_X->setRange(default_min_X, default_max_X);
        axis_X->setTickCount((default_max_X - default_min_X) / 50 + 1);
        axis_X->setMinorTickCount(2);
    }
    else if (current_range.mode == AxesMode::Manual)
    {
        axis_X->setRange(current_range.min_x, current_range.max_x);
        axis_X->setTickCount((default_max_X - default_min_X) / 50 + 1);
        axis_X->setMinorTickCount(2);
        axis_X->applyNiceNumbers();
    }
    else if (current_range.mode == AxesMode::FitToLine)
    {
        axis_X->setRange(min_x * 1.05, max_x * 1.05);
        axis_X->setTickCount((default_max_X - default_min_X) / 50 + 1);
        axis_X->setMinorTickCount(2);
        axis_X->applyNiceNumbers();
    }

    QValueAxis* axis_Y = new QValueAxis();
    axis_Y->setTitleText("Amplitude");

    if (current_range.mode == AxesMode::Default)
    {
        axis_Y->setRange(default_min_Y, default_max_Y * 1.05f);
        axis_Y->setTickCount(Y_tick_count);
        axis_Y->setMinorTickCount(Y_minor_tick_count);
        axis_Y->applyNiceNumbers();
    }
    else if (current_range.mode == AxesMode::Manual)
    {
        axis_Y->setRange(current_range.min_y, current_range.max_y);
        axis_Y->setTickCount(Y_tick_count);
        axis_Y->setMinorTickCount(Y_minor_tick_count);
        axis_Y->applyNiceNumbers();
    }
    else if (current_range.mode == AxesMode::FitToLine)
    {
        axis_Y->setRange(min_y * 1.05, max_y * 1.05);
        axis_Y->setTickCount(Y_tick_count);
        axis_Y->setMinorTickCount(Y_minor_tick_count);
        axis_Y->applyNiceNumbers();
    }

    return {axis_X, axis_Y};
}


void ChartSet::show_plots(ChartData& chart_data)
{
    chart.setAnimationOptions(QChart::AllAnimations);
    QList<QAbstractSeries*> all_series = chart.series();
    for (QAbstractSeries* series : all_series)
        chart.removeSeries(series);

    int i = 0;
    for (auto& series : chart_data.qline_series_array)
    {
        series.setPen(QPen(colors[i % colors.size()], 0.5f));
        series.setName(QString::number(i + 1));
        i++;
    }

    for (auto& series : chart_data.qline_series_array)
        chart.addSeries(&series);

    if (chart_data.axis_X == nullptr or chart_data.axis_Y == nullptr)
        chart.createDefaultAxes();

    else
    {
        if (current_axis_X != nullptr and current_axis_Y != nullptr)
        {
            chart.removeAxis(current_axis_X.get());
            chart.removeAxis(current_axis_Y.get());
        }

        chart.addAxis(chart_data.axis_X.get(), Qt::AlignBottom);
        chart.addAxis(chart_data.axis_Y.get(), Qt::AlignLeft);

        current_axis_X = chart_data.axis_X;
        current_axis_Y = chart_data.axis_Y;

        for (auto& series : chart_data.qline_series_array)
        {
            series.attachAxis(chart_data.axis_X.get());
            series.attachAxis(chart_data.axis_Y.get());
        }
    }

    chartView.setRenderHint(QPainter::Antialiasing);
    chartView.show();
}


float convert_string_comma_to_float(const std::wstring& num)
{
    std::wstring converted_num = num;
    for (auto it = converted_num.rbegin(); it != converted_num.rend(); ++it)
        if (*it == L',')
        {
            *it = L'.';
            break;
        }
    return std::stof(converted_num);
}


void InteractiveChartView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton and not this->chart()->series().isEmpty())
    {
        this->chart()->setAnimationOptions(QChart::NoAnimation);
        QPointF clickedPoint = this->chart()->mapToValue(event->pos());
        if (cross_X != nullptr)
        {
            if(chart()->series().contains(cross_X))
                chart()->removeSeries(cross_X);
            delete cross_X;
        }
        if (cross_Y != nullptr)
        {
            if(chart()->series().contains(cross_Y))
                chart()->removeSeries(cross_Y);
            delete cross_Y;
        }

        cross_X = new QLineSeries();
        float cross_min_x = qobject_cast<QValueAxis*>(chart()->axes(Qt::Horizontal).first())->min();
        float cross_max_x = qobject_cast<QValueAxis*>(chart()->axes(Qt::Horizontal).first())->max();
        float x_cross_range = (cross_max_x - cross_min_x) * (3.0f / 4.0f) / cross_scope;
        cross_X->append(clickedPoint.x() + x_cross_range, clickedPoint.y());
        cross_X->append(clickedPoint.x() - x_cross_range, clickedPoint.y());

        cross_Y = new QLineSeries();
        float cross_min_y = qobject_cast<QValueAxis*>(chart()->axes(Qt::Vertical).first())->min();
        float cross_max_y = qobject_cast<QValueAxis*>(chart()->axes(Qt::Vertical).first())->max();
        float y_cross_range = (cross_max_y - cross_min_y) / cross_scope;
        cross_Y->append(clickedPoint.x(), clickedPoint.y() - y_cross_range);
        cross_Y->append(clickedPoint.x(), clickedPoint.y() + y_cross_range);

        QPen cross_pen(Qt::black, 0.7f);
        cross_X->setPen(cross_pen);
        cross_Y->setPen(cross_pen);

        chart()->addSeries(cross_X);
        chart()->addSeries(cross_Y);

        chart()->legend()->markers(cross_X).at(0)->setVisible(false);
        chart()->legend()->markers(cross_Y).at(0)->setVisible(false);

        cross_X->attachAxis(qobject_cast<QValueAxis*>(chart()->axes(Qt::Horizontal).first()));
        cross_X->attachAxis(qobject_cast<QValueAxis*>(chart()->axes(Qt::Vertical).first()));
        cross_Y->attachAxis(qobject_cast<QValueAxis*>(chart()->axes(Qt::Horizontal).first()));
        cross_Y->attachAxis(qobject_cast<QValueAxis*>(chart()->axes(Qt::Vertical).first()));
        this->show();

        QString text = "x: " + QString::number(clickedPoint.x()) + "\n" +
                       "y: " + QString::number(clickedPoint.y());
        ui->coord_label->setText(text);
        current_x = clickedPoint.x();
        current_y = clickedPoint.y();
    }
    QChartView::mousePressEvent(event);
}

void MainWindow::on_add_current_data_pushButton_clicked()
{
    add_current_data();
}


DataListContainer::DataListContainer()
{
    auto limit = std::numeric_limits<double>::infinity();
    validator = new QDoubleValidator(-limit, limit, 2);
    validator->setNotation(QDoubleValidator::Notation::StandardNotation);  
}

void DataListContainer::addPoint(QPointF point, double time, ExperimentPoint::TimeMeasurement measure, QComboBox* comboBox)
{
    point_list.append(ExperimentPoint(point, DataListContainer::convert_to_seconds(time, measure), measure));

    is_inserting = true;
    table_widget->insertRow(table_widget->rowCount());    

    //QString time_measure_as_QString = DataListContainer::timeMeasurement_toQString(measure);

    QTableWidgetItem *t = new QTableWidgetItem();
    t->setData(2, time);
    t->setTextAlignment(Qt::AlignCenter);
    table_widget->setItem(table_widget->rowCount() - 1, 0, t);

    QTableWidgetItem *m = new QTableWidgetItem();
    table_widget->setItem(table_widget->rowCount() - 1, 1, m);
    comboBox->setCurrentIndex(measure);
    table_widget->setCellWidget(table_widget->rowCount() - 1, 1, comboBox);

    QTableWidgetItem *x = new QTableWidgetItem();
    x->setData(2, point.x());
    table_widget->setItem(table_widget->rowCount() - 1, 2, x);

    QTableWidgetItem *y = new QTableWidgetItem();
    y->setData(2, point.y());
    table_widget->setItem(table_widget->rowCount() - 1, 3, y);
    is_inserting = false;
}

void DataListContainer::deletePoint(int row)
{
    if (row >= 0 and row < point_list.size())
    {
        table_widget->removeRow(row);
        point_list.removeAt(row);
    }
}

void DataListContainer::updatePoint_in_list(int row)
{
    if (not is_inserting)
    {
        auto it = point_list.begin();
        for (int i = 0; i < row; ++i)
            ++it;

        it->point = QPointF(table_widget->item(row, 2)->text().toFloat(), table_widget->item(row, 3)->text().toFloat());
        it->seconds = table_widget->item(row, 0)->text().toFloat();
    }
}

void DataListContainer::updateMeasure_in_list(int row, ExperimentPoint::TimeMeasurement measure)
{
    if (not is_inserting)
    {
        auto it = point_list.begin();
        for (int i = 0; i < row; ++i)
            ++it;
        it->seconds = convert_to_seconds(convert_to_measure(it->seconds, it->measure), measure);
        it->measure = measure;
    }
}

const QList<ExperimentPoint>& DataListContainer::getPointList() const
{
    return point_list;
}

QComboBox* DataListContainer::create_measure_ComboBox()
{
    QComboBox* measure_box = new QComboBox();
    measure_box->addItem("s");
    measure_box->addItem("m");
    measure_box->addItem("h");
    measure_box->addItem("d");
    return measure_box;
}


double DataListContainer::convert_to_seconds(double time, ExperimentPoint::TimeMeasurement measure)
{
    switch (measure)
    {
    case ExperimentPoint::TimeMeasurement::t_seconds:
        return time;
    case ExperimentPoint::TimeMeasurement::t_minutes:
        return time * 60;
    case ExperimentPoint::TimeMeasurement::t_hours:
        return time * 60 * 60;
    default:
        return time * 60 * 60 * 24;
    }
}

double DataListContainer::convert_to_measure(double seconds, ExperimentPoint::TimeMeasurement measure)
{
    switch (measure)
    {
    case ExperimentPoint::TimeMeasurement::t_seconds:
        return seconds;
    case ExperimentPoint::TimeMeasurement::t_minutes:
        return seconds / 60;
    case ExperimentPoint::TimeMeasurement::t_hours:
        return seconds / (60 * 60);
    default:
        return seconds / (60 * 60 * 24);
    }
}

QString DataListContainer::timeMeasurement_toQString(ExperimentPoint::TimeMeasurement measure)
{
    switch (measure)
    {
    case ExperimentPoint::TimeMeasurement::t_seconds:
        return "s";
    case ExperimentPoint::TimeMeasurement::t_minutes:
        return "m";
    case ExperimentPoint::TimeMeasurement::t_hours:
        return "h";
    default:
        return "d";
    }
}

ExperimentPoint::TimeMeasurement DataListContainer::qString_to_timeMeasurement(const QString &measure)
{
    if (measure == "s")
        return ExperimentPoint::t_seconds;
    if (measure == "m")
        return ExperimentPoint::t_minutes;
    if (measure == "h")
        return ExperimentPoint::t_hours;
    return ExperimentPoint::t_days;
}

void MainWindow::on_secondsRadioButton_clicked()
{
    current_time_measure = ExperimentPoint::TimeMeasurement::t_seconds;
}


void MainWindow::on_minutesRadioButton_clicked()
{
    current_time_measure = ExperimentPoint::TimeMeasurement::t_minutes;
}


void MainWindow::on_hoursRadioButton_clicked()
{
    current_time_measure = ExperimentPoint::TimeMeasurement::t_hours;
}


void MainWindow::on_daysRadioButton_clicked()
{
    current_time_measure = ExperimentPoint::TimeMeasurement::t_days;
}


void MainWindow::on_delete_line_from_current_list_pushButton_clicked()
{
    int row = ui->current_table_listWidget->currentRow();
    current_data_table_widget.deletePoint(row);
}

void MainWindow::handleCurrentMeasureChangedInTable(int index)
{
    current_data_table_widget.updateMeasure_in_list(current_data_table_widget.table_widget->currentRow(), static_cast<ExperimentPoint::TimeMeasurement>(index));
}

void MainWindow::actionChange_axes_range_triggered()
{
    if (not chart_data_list.empty())
    {
        this->setEnabled(false);
        chart_settings_form->show();

        auto data_list_it = chart_data_list.begin();
        std::advance(data_list_it, current_chart_data_index);
        AxesRange range = {(float) data_list_it->axis_X->min(), (float)data_list_it->axis_X->max(), (float)data_list_it->axis_Y->min(), (float)data_list_it->axis_Y->max()};
        if (data_list_it->title == "Amplitude")
        {
            chart_settings_form->set_Y_spin_boxes_step(100, 500);
            chart_settings_form->set_Y_spin_boxes_decimals(0, 0);
        }
        else
        {
            chart_settings_form->set_Y_spin_boxes_decimals(3, 3);
            chart_settings_form->set_Y_spin_boxes_step(0.05, 0.05);
        }
        chart_settings_form->set_spin_boxes_values(range);
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("No files uploaded"));
    }
}

void MainWindow::action_save_to_file_triggered()
{
    if (not current_data_table_widget.getPointList().empty())
    {
        this->setEnabled(false);
        save_to_file_form->show();
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("No points to save"));
    }
}

void MainWindow::action_change_Y_name_to_amplitude()
{
    if (not chart_data_list.empty())
    {
        auto data_list_it = chart_data_list.begin();
        std::advance(data_list_it, current_chart_data_index);
        data_list_it->title = "Amplitude";
        data_list_it->axis_Y->setTitleText(data_list_it->title);
        data_list_it->axis_Y->setRange(default_min_Y, data_list_it->max_Y_value * 1.05f);
        data_list_it->axis_Y->setTickCount(Y_tick_count);
        data_list_it->axis_Y->setMinorTickCount(Y_minor_tick_count);
        data_list_it->axis_Y->applyNiceNumbers();
        data_list_it->fit_to_line_ranges.mode = AxesMode::Default;
        chart_set->show_plots(*data_list_it);
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("No files uploaded"));
    }
}

void MainWindow::action_change_Y_name_to_reflection()
{
    if (not chart_data_list.empty())
    {
        auto data_list_it = chart_data_list.begin();
        std::advance(data_list_it, current_chart_data_index);
        data_list_it->title = "Reflection";
        data_list_it->axis_Y->setTitleText(data_list_it->title);
        data_list_it->axis_Y->setRange(-0.1, data_list_it->max_Y_value);
        data_list_it->axis_Y->setTickCount(Y_tick_count);
        data_list_it->axis_Y->setMinorTickCount(Y_minor_tick_count);
        data_list_it->axis_Y->applyNiceNumbers();
        data_list_it->fit_to_line_ranges.mode = AxesMode::Default;
        chart_set->show_plots(*data_list_it);
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("No files uploaded"));
    }
}

void MainWindow::action_change_Y_name_to_absorbtion()
{
    if (not chart_data_list.empty())
    {
        auto data_list_it = chart_data_list.begin();
        std::advance(data_list_it, current_chart_data_index);
        data_list_it->title = "Absorbtion";
        data_list_it->axis_Y->setTitleText(data_list_it->title);
        data_list_it->axis_Y->setRange(-0.1, data_list_it->max_Y_value);
        data_list_it->axis_Y->setTickCount(Y_tick_count);
        data_list_it->axis_Y->setMinorTickCount(Y_minor_tick_count);
        data_list_it->axis_Y->applyNiceNumbers();
        data_list_it->fit_to_line_ranges.mode = AxesMode::Default;
        chart_set->show_plots(*data_list_it);
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("No files uploaded"));
    }
}

void MainWindow::change_chart_setting_form_closed(std::pair<bool, AxesRange> result)
{
    this->setEnabled(true);

    if (result.first == true)
    {
        current_range = result.second;
        auto data_list_it = chart_data_list.begin();
        std::advance(data_list_it, current_chart_data_index);

        auto axes = ChartSet::createQValueAxes(data_list_it->fit_to_line_ranges, data_list_it->max_Y_value);

        data_list_it->axis_X.reset(axes.first);
        data_list_it->axis_Y.reset(axes.second);
        data_list_it->axis_Y->setTitleText(data_list_it->title);
        chart_set->show_plots(*data_list_it);
    }
}

void MainWindow::save_to_file_form_closed()
{
    this->setEnabled(true);
}

void MainWindow::save_to_file_slot(const SaveSettings& settings)
{
    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString saveFilePath = QFileDialog::getSaveFileName(
        nullptr,
        "Save File",
        defaultDir,
        "CSV Files (*.csv);;Text Files (*.txt)"
    );

    if (not saveFilePath.isEmpty())
    {
        std::wofstream file(saveFilePath.toStdWString());

        if (settings.comma == true)
        {
            struct my_numpunct : std::numpunct<wchar_t>
            {
            protected:
                wchar_t do_decimal_point() const override { return L','; } // use comma as decimal separator
                std::string do_grouping() const override { return "\0"; } // remove thousands separator
            };

            std::locale russian("ru_RU.utf8");
            file.imbue(std::locale(russian, new my_numpunct));
        }
        char separator = '\t';
        //if (QFileInfo(saveFilePath).suffix() == "csv")
         //   separator = ',';

        const QList<ExperimentPoint>& point_list = current_data_table_widget.getPointList();
        for (const auto& point : point_list)
        {
            if (settings.time)
            {
                if (settings.time_measurement == 4)
                    file << DataListContainer::convert_to_measure(point.seconds, point.measure) << ' ' << DataListContainer::timeMeasurement_toQString(point.measure).toStdWString() << separator;
                else
                    file << DataListContainer::convert_to_measure(point.seconds, static_cast<ExperimentPoint::TimeMeasurement>(settings.time_measurement)) << separator;
            }
            if (settings.x)
                 file << point.point.x() << separator;
            if (settings.y)
                file << point.point.y() << separator;
            file << '\n';
        }
        file.close();
        save_to_file_form->hide();
        this->setEnabled(true);
    }
}

void MainWindow::add_current_data()
{
    auto comboBox = DataListContainer::create_measure_ComboBox();
    QObject::connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                     [&](int index) {

                         emit MainWindow::currentMeasureChangedInTable(index);
                     });
    double time = ui->timeNumberDoubleSpinBox->value();
    current_data_table_widget.addPoint(chart_set->chartView.getXY(), time, current_time_measure, comboBox);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Plus)
        add_current_data();
}


void MainWindow::on_current_table_listWidget_itemChanged(QTableWidgetItem *item)
{
    current_data_table_widget.updatePoint_in_list(item->row());
}


QString extract_file_name_from_path(const QString &path)
{
    bool was_slash = false;
    int i = 0;
    for (auto it = path.rbegin(); it != path.crend(); ++it, ++i)
        if (*it == '/')
        {
            if (not was_slash)
                was_slash = true;
            else
                break;
        }
    return path.mid(path.size() - i, i);
}

QString remove_filename_from_path(const QString &path)
{
    int i = 0;
    for (auto it = path.rbegin(); it != path.crend(); ++it, i++)
        if (*it == '/')
            break;
    return path.mid(0, path.size() - i);
}

void MainWindow::on_showResults_pushButton_clicked()
{
    result_form->show();
}


void MainWindow::on_opend_files_listWidget_currentRowChanged(int currentRow)
{
    auto data_list_it = chart_data_list.begin();
    std::advance(data_list_it, currentRow);
    chart_set->show_plots(*data_list_it);
    current_chart_data_index = currentRow;
}

void MainWindow::key_plus_pressed_handle_slot()
{
    add_current_data();
}

void DoubleSpinBoxContainer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Plus)
    {
        //double_spin_box->grabKeyboard();
        emit key_plus_pressed();
        qDebug() << "key + pressed" << '\n';
    }
    else
    {
        QDoubleSpinBox::keyPressEvent(event);
    }
}


