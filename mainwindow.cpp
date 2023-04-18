#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <qlabel.h>
#include <string>
#include <fstream>
#include <QMessageBox>
#include <sstream>
#include <QDebug>
#include <codecvt>

AxesRange current_range;

constexpr unsigned Y_tick_count = 14, Y_minor_tick_count = 1;

constexpr float default_min_X = 350, default_max_X = 750;
constexpr float default_min_Y = -100;
constexpr std::pair<float, float> max_range = {500, 650};

constexpr QColor Y_colomn_color = QColor{195, 255, 250};
constexpr std::array<QColor, 30> colors = { QColor(31, 119, 180), QColor(255, 127, 14), QColor(44, 160, 44),
                    QColor(214, 39, 40), QColor(148, 103, 189), QColor(140, 86, 75),
                    QColor(227, 119, 194), QColor(127, 127, 127), QColor(188, 189, 34),
                    QColor(23, 190, 207), QColor(234, 87, 153), QColor(129, 199, 132), QColor(100, 181, 246),
                    QColor(102, 187, 106), QColor(255, 202, 40), QColor(156, 39, 176),
                    QColor(205, 220, 57), QColor(121, 85, 72), QColor(239, 83, 80),
                    QColor(255, 160, 0), QColor(121, 85, 72), QColor(171, 71, 188),
                    QColor(63, 81, 181), QColor(255, 235, 59), QColor(76, 175, 80),
                    QColor(255, 99, 71), QColor(0, 150, 136), QColor(255, 193, 7),
                    QColor(48, 63, 159), QColor(255, 152, 0) };

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), result_form(new result_chart_form()), chart_settings_form(new chageChartSettingsForm()),
      save_to_file_form(new SaveToFileForm), convert_oscilloscope_file_form(new ConvertOscilloscopeFileForm())
{
    ui->setupUi(this);
    this->current_data_table_widget.table_widget = ui->current_table_listWidget;
    this->current_data_table_widget.norm_table_widget = ui->normalization_tableWidget;
    this->current_data_table_widget.addExperiment_name("y1");
    this->current_data_table_widget.set_show_x(false);
    this->current_data_table_widget.table_widget->installEventFilter(this);

    experiment_name_change_form.reset(new Experiment_Name_Form());
    connect(experiment_name_change_form.get(), &Experiment_Name_Form::form_closed, this, &MainWindow::change_experiment_name_form_closed);
    experiment_name_change_form->setWindowFlags(experiment_name_change_form->windowFlags() & ~Qt::WindowMaximizeButtonHint);

    add_new_experiment_form.reset(new AddNewExperimentForm());
    connect(add_new_experiment_form.get(), &AddNewExperimentForm::form_closed, this, &MainWindow::add_new_experiment_form_closed);
    add_new_experiment_form->setWindowFlags(add_new_experiment_form->windowFlags() & ~Qt::WindowMaximizeButtonHint);

    ui->current_table_listWidget->setColumnWidth(0, 62);
    ui->current_table_listWidget->setColumnWidth(1, 50);
    ui->current_table_listWidget->setLocale(QLocale::English);

    ui->normalization_tableWidget->setColumnWidth(0, 80);
    ui->normalization_tableWidget->verticalHeader()->hide();
    ui->normalization_tableWidget->setLocale(QLocale::English);

    DoubleSpinBoxDelegate *delegate = new DoubleSpinBoxDelegate;
    ui->normalization_tableWidget->setItemDelegate(delegate);

    connect(ui->actionOpen_file_excel, &QAction::triggered, this, &MainWindow::action_open_excel_file_triggered);

    connect(ui->actionConvert_file_Oscilloscope, &QAction::triggered, this, &MainWindow::action_convert_oscilloscope_file_triggered);
    connect(convert_oscilloscope_file_form.get(), &ConvertOscilloscopeFileForm::form_closed, this, &MainWindow::convert_oscilloscope_form_closed);
    convert_oscilloscope_file_form->setWindowFlags(convert_oscilloscope_file_form->windowFlags() & ~Qt::WindowMaximizeButtonHint);

    connect(ui->current_table_listWidget->verticalScrollBar(), &QScrollBar::valueChanged, ui->normalization_tableWidget->verticalScrollBar(),
        [this](int value){
            ui->normalization_tableWidget->verticalScrollBar()->setValue(value);
        }
    );
    connect(ui->normalization_tableWidget->verticalScrollBar(), &QScrollBar::valueChanged, current_data_table_widget.table_widget->verticalScrollBar(), &QScrollBar::setValue);

    //ui->add_norm_value_radioButton->setEnabled(false);
    ui->add_time_point_radioButton->setChecked(true);

    //Replacing double_spin_box
    double_spin_box_container.reset(new DoubleSpinBoxContainer(ui->timeGroupBox));
    double_spin_box_container->setKeyboardTracking(true);
    double_spin_box_container->setLocale(QLocale::English);

    double_spin_box_container->setRange(ui->timeNumberDoubleSpinBox->minimum(), ui->timeNumberDoubleSpinBox->maximum());
    double_spin_box_container->setValue(ui->timeNumberDoubleSpinBox->value());
    double_spin_box_container->setSingleStep(ui->timeNumberDoubleSpinBox->singleStep());
    double_spin_box_container->setDecimals(ui->timeNumberDoubleSpinBox->decimals());
    double_spin_box_container->move(10, 30);
    double_spin_box_container->setFixedSize(111, 30);

    delete ui->timeNumberDoubleSpinBox;
    ui->timeNumberDoubleSpinBox = double_spin_box_container.get();
    connect(double_spin_box_container.get(), &DoubleSpinBoxContainer::key_plus_pressed, this, &MainWindow::key_plus_pressed_handle_slot);

    chart_settings_form->setWindowFlags(chart_settings_form->windowFlags() & ~Qt::WindowMaximizeButtonHint);
    connect(chart_settings_form.get(), &chageChartSettingsForm::returnResult, this, &MainWindow::change_chart_setting_form_closed);
    chart_settings_form->setWindowFlags(chart_settings_form->windowFlags() & ~Qt::WindowMaximizeButtonHint);

    connect(ui->actionChange_axes_range, &QAction::triggered, this, &MainWindow::actionChange_axes_range_triggered);
    connect(ui->actionSave_to_file, &QAction::triggered, this, &MainWindow::action_save_to_file_triggered);

    connect(ui->actionAmplitude_2, &QAction::triggered, this, &MainWindow::action_change_Y_name_to_amplitude);
    connect(ui->actionAbsorbtion, &QAction::triggered, this, &MainWindow::action_change_Y_name_to_absorbtion);
    connect(ui->actionReflection, &QAction::triggered, this, &MainWindow::action_change_Y_name_to_reflection);

    connect(ui->actionAuto_cell_switching, &QAction::toggled, this, &MainWindow::action_auto_cell_switch_check_box_state_changed);

    connect(save_to_file_form.get(), &SaveToFileForm::form_closed, this, &MainWindow::save_to_file_form_closed);
    connect(save_to_file_form.get(), &SaveToFileForm::save_to_file, this, &MainWindow::save_to_file_slot);
    save_to_file_form->setWindowFlags(save_to_file_form->windowFlags() & ~Qt::WindowMaximizeButtonHint);

    connect(ui->actionOpen_file, &QAction::triggered, this, &MainWindow::on_pushButton_clicked);

    connect(result_form.get(), &result_chart_form::form_closed, this, &MainWindow::showResults_form_closed);

    ui->secondsRadioButton->setChecked(true);

    ui->currentFile_const_label->hide();

    chart_set = new ChartSet(ui, ui->lines_on_chart_listWidget, &chart_data_list, &current_chart_data_index);
    ui->showResults_pushButton->hide();
    QObject::connect(this, &MainWindow::currentMeasureChangedInTable, this, &MainWindow::handleCurrentMeasureChangedInTable);
}

MainWindow::~MainWindow()
{
    delete ui;
    //delete chart_set;
}


void MainWindow::on_pushButton_clicked()
{   
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), prev_open_filepath, tr("Text Files (*.txt);;All Files (*.*)"));
    prev_open_filepath = remove_filename_from_path(filename);
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
            int columns_number = 0;

            std::wstring line;
            line.reserve(100);
            std::getline(file, line);

            std::wstringstream initial_stream(line);
            while(not initial_stream.eof())
            {
                std::wstring num;
                initial_stream >> num;
                columns_number++;
            }

            std::vector<QLineSeries> temp_qline_series_array(columns_number / 2);

            while (std::getline(file, line))
            {
                std::wstringstream line_stream(line);
                std::wstring num_x, num_y;

                for (int i = 0; i < columns_number; i += 2)
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
            chart_set->show_plots(chart_data_list.back(), QChart::AllAnimations);
            current_chart_data_index = chart_data_list.size() - 1;
        }
    }
    catch(...)
    {
        QMessageBox::critical(this, tr("Error"), tr("File reading error"));
    }
    file.close();
}

void MainWindow::action_convert_oscilloscope_file_triggered()
{
    convert_oscilloscope_file_form->show();
    this->setEnabled(false);
    this->hide();
}

void MainWindow::convert_oscilloscope_form_closed()
{
    this->setEnabled(true);
    this->show();
}

void MainWindow::action_open_excel_file_triggered()
{

}

ChartSet::ChartSet(Ui::MainWindow* ui, QListWidget* lines_list_widget, std::list<ChartData>* chart_data_list, const size_t* current_chart_data_index)
    : lines_list_widget(lines_list_widget), chart_data_list(chart_data_list), current_chart_data_index(current_chart_data_index)
{
    chart.setParent(ui->horizontalFrame);
    chart.setAnimationOptions(QChart::SeriesAnimations);
    chart.legend()->setVisible(true);
    auto font = chart.legend()->font();
    font.setPointSize(12);
    chart.legend()->setFont(font);
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
    axis_X->setTitleText("Wavelength, nm");
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


void ChartSet::show_plots(ChartData& chart_data, QChart::AnimationOption option, bool rebuild_lines_list_widget)
{   
    chart.setAnimationOptions(option);
    QList<QAbstractSeries*> all_series = chart.series();
    for (QAbstractSeries* series : all_series)
        chart.removeSeries(series);

    if (rebuild_lines_list_widget)
        lines_list_widget->clear();
    int i = 0;
    for (auto series = chart_data.qline_series_array.rbegin(); series != chart_data.qline_series_array.rend(); ++series)
    {
        series->setPen(QPen(colors[i % colors.size()], 0.7f));
        series->setName(QString::number(i + 1));

        if (rebuild_lines_list_widget)
        {
            QListWidgetItem* list_item = new QListWidgetItem();
            lines_list_widget->addItem(list_item);
            QCheckBox* list_check_box = new QCheckBox();
            list_check_box->setText(QString::number(i + 1));
            list_check_box->setChecked(chart_data.lines_enabled_array[(chart_data.lines_enabled_array.size() - 1) - i]);
            connect(list_check_box, &QCheckBox::stateChanged, this, &ChartSet::check_box_in_lines_widget_list_state_changed);
            lines_list_widget->setItemWidget(list_item, list_check_box);
        }
        else
        {
            QListWidgetItem* item = lines_list_widget->item(i);

            QCheckBox* check_box = qobject_cast<QCheckBox*> (lines_list_widget->itemWidget(item));
            int index = (chart_data.lines_enabled_array.size() - 1) - i;
            bool value = chart_data.lines_enabled_array[index];
            check_box->blockSignals(true);
            if (check_box != nullptr)
                check_box->setChecked(value);
            check_box->blockSignals(false);
        }
        i++;
    }

    for (int i = static_cast<int> (chart_data.qline_series_array.size()) - 1; i >= 0; i--)
        if(chart_data.lines_enabled_array[i])
            chart.addSeries(&chart_data.qline_series_array[i]);

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

        int i = 0;
        for (auto& series : chart_data.qline_series_array)
        {
            if (chart_data.lines_enabled_array[i])
            {
                series.attachAxis(chart_data.axis_X.get());
                series.attachAxis(chart_data.axis_Y.get());
            }
            i++;
        }
    }
    chartView.setRenderHint(QPainter::Antialiasing);    
}

void ChartSet::next_line()
{
    auto current_lines = chart_data_list->begin();
    std::advance(current_lines, *current_chart_data_index);

    if (current_lines->current_line < 1)
        current_lines->current_line = static_cast<int> (current_lines->qline_series_array.size()) - 1;
    else
        current_lines->current_line--;

    for (int i = static_cast<int> (current_lines->lines_enabled_array.size()) - 1; i >= 0 ; i--)
        current_lines->lines_enabled_array[i] = (i == current_lines->current_line ? true : false);

    show_plots(*current_lines, QChart::NoAnimation, false);

    lines_list_widget->scrollToItem(lines_list_widget->item(lines_list_widget->count() - 1 - current_lines->current_line));
}

void ChartSet::prev_line()
{
    auto current_lines = chart_data_list->begin();
    std::advance(current_lines, *current_chart_data_index);

    if (current_lines->current_line >=  current_lines->qline_series_array.size() - 1)
        current_lines->current_line = 0;
    else
        current_lines->current_line++;

    for (int i = static_cast<int> (current_lines->lines_enabled_array.size()) - 1; i >= 0 ; i--)
        current_lines->lines_enabled_array[i] = (i == current_lines->current_line ? true : false);

    show_plots(*current_lines, QChart::NoAnimation, false);

    lines_list_widget->scrollToItem(lines_list_widget->item(lines_list_widget->count() - 1 - current_lines->current_line));
}

void ChartSet::turn_on_all_lines()
{
    auto current_lines = chart_data_list->begin();
    std::advance(current_lines, *current_chart_data_index);

    for (int i = static_cast<int> (current_lines->lines_enabled_array.size()) - 1; i >= 0 ; i--)
        current_lines->lines_enabled_array[i] = true;

    show_plots(*current_lines, QChart::NoAnimation);
}

void ChartSet::check_box_in_lines_widget_list_state_changed(int state)
{
    int index = (lines_list_widget->count() - 1) - lines_list_widget->currentRow();
    auto current_lines = chart_data_list->begin();
    std::advance(current_lines, *current_chart_data_index);
    current_lines->lines_enabled_array[index] = state;
    chart.setAnimationOptions(QChart::NoAnimation);
    if (state)
    {
        chart.addSeries(&current_lines->qline_series_array[index]);
        current_lines->qline_series_array[index].attachAxis(current_axis_X.get());
        current_lines->qline_series_array[index].attachAxis(current_axis_Y.get());

    }
    else
        chart.removeSeries(&current_lines->qline_series_array[index]);
    show_plots(*current_lines, QChart::NoAnimation, false);
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

void DataListContainer::addTimePoint(double time, ExperimentPoint::TimeMeasurement measure, QComboBox *comboBox)
{
    point_list.append(ExperimentPoint(DataListContainer::convert_to_seconds(time, measure), measure));
    for (int i = 0; i < experiment_names.size(); i++)
    {
        point_list[point_list.size() - 1].points.emplaceBack(1E+16, 1E+16);
    }

    is_inserting = true;
    table_widget->insertRow(table_widget->rowCount());

    QTableWidgetItem *t = new QTableWidgetItem();
    t->setData(2, time);
    t->setTextAlignment(Qt::AlignCenter);
    table_widget->setItem(table_widget->rowCount() - 1, 0, t);

    QTableWidgetItem *m = new QTableWidgetItem();
    table_widget->setItem(table_widget->rowCount() - 1, 1, m);
    comboBox->setCurrentIndex(measure);
    table_widget->setCellWidget(table_widget->rowCount() - 1, 1, comboBox);
    is_inserting = false;
}

void DataListContainer::addPoint(QPointF point, double time, ExperimentPoint::TimeMeasurement measure, QComboBox* comboBox)
{
    point_list.append(ExperimentPoint(point, DataListContainer::convert_to_seconds(time, measure), measure));

    for (int i = 1; i < experiment_names.size(); i++)
    {
        point_list[point_list.size() - 1].points.emplaceBack(1E+16, 1E+16);
    }

    is_inserting = true;
    table_widget->insertRow(table_widget->rowCount());    

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
    y->setBackground(Y_colomn_color);
    table_widget->setItem(table_widget->rowCount() - 1, 3, y);
    is_inserting = false;
}

void DataListContainer::deletePoint(int row)
{
    if (row >= 0 and row < point_list.size())
    {
        table_widget->removeRow(row);
        norm_table_widget->removeRow(row);
        point_list.removeAt(row);
    }
}

void DataListContainer::updatePoint_in_list(int row, bool use_normalization)
{
    if (not is_inserting)
    {
        point_list[row].seconds = convert_to_seconds(table_widget->item(row, 0)->text().toDouble(), point_list[row].measure);
        int numColumns = table_widget->columnCount();
        for (int i = 2; i < numColumns; i += 2)
        {
            QTableWidgetItem* item_x = table_widget->item(row, i);
            QTableWidgetItem* item_y = table_widget->item(row, i + 1);
            if (not (item_x == nullptr and item_y == nullptr))
            {
                float x = (item_x == nullptr) ? 0 : item_x->text().toFloat();
                float y = (item_y == nullptr) ? 0 : item_y->text().toFloat();
                if (not use_normalization)
                    point_list[row].points[(i - 2) / 2] = QPointF(x, y);
                else
                    point_list[row].points[(i - 2) / 2] = QPointF(x, y * norm_table_widget->item(row, 0)->text().toDouble());
            }
        }
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

void DataListContainer::addExperiment_point(int row, int colomn, QPointF point, bool normalize)
{
    //qDebug() << point_list.size()  << '\n';
    //qDebug() << point_list[row].points.size() << '\n';
    point_list[row].points[colomn / 2 - 1] = point;
    is_inserting = true;

    QTableWidgetItem *x = new QTableWidgetItem();
    x->setData(2, point.x());
    table_widget->setItem(row, colomn, x);
    table_widget->setColumnHidden(colomn, not show_x);

    double value = point.y();
    QTableWidgetItem *y = new QTableWidgetItem();
    if (normalize)
        value /= point_list[row].normalization_value;
    y->setData(2, value);
    y->setBackground(Y_colomn_color);
    table_widget->setItem(row, colomn + 1, y);

    is_inserting = false;
}

void DataListContainer::addExperiment_name(QString name)
{
     experiment_names.append(name);
     for (auto& time_point : point_list)
         time_point.points.emplaceBack(1E+16, 1E+16);
}

void DataListContainer::addNormalization_line()
{
    is_inserting = true;
    norm_table_widget->insertRow(norm_table_widget->rowCount());
    QTableWidgetItem* norm = new QTableWidgetItem();
    norm->setData(2, 1.0f);
    norm->setTextAlignment(Qt::AlignCenter);
    norm_table_widget->setItem(norm_table_widget->rowCount() - 1, 0, norm);
    is_inserting = false;
}

void DataListContainer::changeNormalization_value(int row, double value, bool use_normalization)
{
    if (not is_inserting)
    {
        point_list[row].normalization_value = value;
        point_list[row].changed_normalize = true;
        norm_table_widget->item(row, 0)->setData(2, value);
        normalize_row_in_table_widget(row, use_normalization);
    }
}

void DataListContainer::changeExperiment_name(QString name, int index)
{
    experiment_names[index] = name;
}

void DataListContainer::removeExperiment_name(int index)
{
    experiment_names.removeAt(index);
    for (auto& point : point_list)
    {
        if (point.points.size() - 1 >= index)
            point.points.removeAt(index);
    }
}

void DataListContainer::normalize_row_in_table_widget(int row, bool normalize)
{
    is_inserting = true;
    double norm_value = point_list[row].normalization_value;
    for (int j = 3; j < table_widget->columnCount(); j += 2)
    {
        if (table_widget->item(row, j) == nullptr)
            continue;
        double y_value = point_list[row].points[j / 2 - 1].y();
        if (y_value > 1E+15)
            continue;
        if (normalize)
            y_value /= norm_value;
        table_widget->item(row, j)->setData(2, y_value);
    }
    is_inserting = false;
}

void DataListContainer::set_show_x(bool is_show)
{
    show_x = is_show;
    int numColumns = table_widget->columnCount();
    for (int i = 2; i < numColumns; i += 2)
        table_widget->setColumnHidden(i, !is_show);
}

bool DataListContainer::select_cell_to_add_exp_point()
{
    if (not auto_cell_switch)
        return true;

    for (int i = 0; i < table_widget->rowCount(); ++i)
        for (int j = 2; j < table_widget->columnCount(); j += 2)
            if (table_widget->item(i, j) == nullptr or
                    table_widget->item(i, j)->text().isEmpty())
            {
                table_widget->setCurrentCell(i, j + 1);
                return true;
            }
    return false;
}

bool DataListContainer::select_cell_to_add_norm_value()
{
    if (not auto_cell_switch)
        return true;

    int i = 0;
    for (const auto& point : point_list)
    {
        if (not point.changed_normalize)
        {
            norm_table_widget->setCurrentCell(i, 0);
            return true;
        }
        i++;
    }
    return false;
}

void DataListContainer::move_line(Direction direction)
{
    int current_index = table_widget->currentRow();
    if (current_index < 0 or table_widget->rowCount() <= 1)
        return;

    int next_index;
    if (direction == Direction::upwards)
    {
        next_index = (current_index - 1) % table_widget->rowCount();
        if (next_index < 0)
            next_index = table_widget->rowCount() - 1;
    }
    else if (direction == Direction::downwards)
        next_index = (current_index + 1) % table_widget->rowCount();
    else
        return;

    table_widget->blockSignals(true);
    norm_table_widget->blockSignals(true);

    for (int i = 0; i < table_widget->columnCount(); i++)
    {
        QTableWidgetItem* item1 = table_widget->takeItem(current_index, i);
        QTableWidgetItem* item2 = table_widget->takeItem(next_index, i);
        if (i == 1)
        {
            QComboBox* box1 = qobject_cast<QComboBox*>(table_widget->cellWidget(current_index, 1));
            QComboBox* box2 = qobject_cast<QComboBox*>(table_widget->cellWidget(next_index, 1));
            int r1 = box1->currentIndex();
            int r2 = box2->currentIndex();
            box1->setCurrentIndex(r2);
            box2->setCurrentIndex(r1);

        }
        table_widget->setItem(current_index, i, item2);
        table_widget->setItem(next_index, i, item1);
    }

    point_list.swapItemsAt(current_index, next_index);

    QTableWidgetItem* item1 = norm_table_widget->takeItem(current_index, 0);
    QTableWidgetItem* item2 = norm_table_widget->takeItem(next_index, 0);
    norm_table_widget->setItem(current_index, 0, item2);
    norm_table_widget->setItem(next_index, 0, item1);


    table_widget->clearSelection();
    table_widget->selectRow(next_index);
    norm_table_widget->clearSelection();
    norm_table_widget->selectRow(next_index);

    table_widget->blockSignals(false);
    norm_table_widget->blockSignals(false);
}


const QList<QString> &DataListContainer::get_experiment_names() const
{
    return experiment_names;
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
        save_to_file_form->load_experiment_names(current_data_table_widget.get_experiment_names());
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
        chart_set->show_plots(*data_list_it, QChart::AllAnimations);
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
        chart_set->show_plots(*data_list_it, QChart::AllAnimations);
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
        chart_set->show_plots(*data_list_it, QChart::AllAnimations);
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("No files uploaded"));
    }
}

void MainWindow::action_auto_cell_switch_check_box_state_changed(bool checked)
{
    current_data_table_widget.auto_cell_switch = checked;
    if (checked)
    {
        if (ui->add_exp_point_radioButton->isChecked())
            current_data_table_widget.select_cell_to_add_exp_point();
        else if (ui->add_norm_value_radioButton->isChecked())
            current_data_table_widget.select_cell_to_add_norm_value();
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
        chart_set->show_plots(*data_list_it, QChart::AllAnimations);
    }
}

void MainWindow::save_to_file_form_closed()
{
    this->setEnabled(true);
}

void MainWindow::save_to_file_slot(const SaveSettings& settings)
{
    QString saveFilePath;

    if (settings.append_to_excel)
        saveFilePath = QFileDialog::getOpenFileName(nullptr, "Append to excel file", prev_save_filepath, "Excel Files (*.xlsx)");
    else
        saveFilePath = QFileDialog::getSaveFileName(nullptr, "Save File", prev_save_filepath,
                    "Excel Files (*.xlsx);;CSV Files (*.csv);;Text Files (*.txt)");
    try
    {
        if (not saveFilePath.isEmpty())
        {
            prev_save_filepath = remove_filename_from_path(saveFilePath);
            if (QFileInfo(saveFilePath).suffix() == "xlsx")
            {
                //saveFilePath.toStdString()
                OpenXLSX::XLDocument doc;

                if (settings.append_to_excel)
                    doc.open(saveFilePath.toStdString());
                else
                    doc.create(saveFilePath.toStdString());
                auto worksheet = doc.workbook().worksheet("Sheet1");

                //start saving
                const QList<QString>& experiment_names = current_data_table_widget.get_experiment_names();
                const QList<ExperimentPoint>& point_list = current_data_table_widget.getPointList();
                //save column titles
                int i_excel = 1, j_excel = 1;
                if (settings.append_to_excel)
                {
                    while(worksheet.cell(i_excel, j_excel).value().type() != OpenXLSX::XLValueType::Empty)
                        i_excel++;
                }
                if (settings.save_column_titles)
                {
                    if (settings.time)
                    {
                        std::string time_str = "time";
                        if (settings.time_measurement != 4)
                            time_str += ", " + DataListContainer::timeMeasurement_toQString(static_cast<ExperimentPoint::TimeMeasurement>(settings.time_measurement)).toStdString();
                        worksheet.cell(i_excel, j_excel).value() = time_str;
                        j_excel++;
                    }

                    for (int i = 0; i < experiment_names.size(); i++)
                    {
                        if (settings.experiments.contains(i))
                        {
                            if (settings.x)
                            {
                                worksheet.cell(i_excel, j_excel).value() = "x" + std::to_string(i + 1);
                                j_excel++;
                            }
                            if (settings.y)
                            {
                                worksheet.cell(i_excel, j_excel).value() = experiment_names[i].toStdString();
                                j_excel++;
                            }
                        }
                    }
                    if (settings.save_norm_column)
                        worksheet.cell(i_excel, j_excel).value() = "norm.";
                    i_excel++; j_excel = 1;
                }

                //Save points
                for (const auto& time_point : point_list)
                {
                    if (settings.time)
                    {
                        if (settings.time_measurement == 4)
                            worksheet.cell(i_excel, j_excel).value() = std::to_string(DataListContainer::convert_to_measure(time_point.seconds, time_point.measure)) + " " + DataListContainer::timeMeasurement_toQString(time_point.measure).toStdString();
                        else
                            worksheet.cell(i_excel, j_excel).value() = DataListContainer::convert_to_measure(time_point.seconds, static_cast<ExperimentPoint::TimeMeasurement>(settings.time_measurement));
                        j_excel++;
                    }

                    for (int i = 0; i < experiment_names.size(); i++)
                    {
                        if (settings.experiments.contains(i))
                        {
                            if (settings.x)
                            {
                                if (time_point.points[i].x() < 1E+15)
                                    worksheet.cell(i_excel, j_excel).value() = time_point.points[i].x();
                                else
                                    worksheet.cell(i_excel, j_excel).value() = "-";
                                j_excel++;
                            }
                            if (settings.y)
                            {
                                if (time_point.points[i].y() < 1E+15)
                                {
                                    float y_value = time_point.points[i].y();
                                    if (settings.normalize_y)
                                        y_value /= time_point.normalization_value;
                                    worksheet.cell(i_excel, j_excel).value() = y_value;

                                }
                                else
                                    worksheet.cell(i_excel, j_excel).value() = "-";
                                j_excel++;
                            }
                        }
                    }
                    if (settings.save_norm_column)
                    {
                        worksheet.cell(i_excel, j_excel).value() = time_point.normalization_value;
                        j_excel++;
                    }

                    i_excel++; j_excel = 1;
                }

                doc.save();
                doc.close();
            }
            else
            {
                std::wofstream file(saveFilePath.toStdWString());

                struct my_numpunct : std::numpunct<wchar_t>
                {
                    bool comma = false;
                protected:
                    wchar_t do_decimal_point() const override
                    {
                        if (comma)
                            return L','; // use comma as decimal separator
                        return L'.'; // use dot as decimal separator
                    }
                    wchar_t do_thousands_sep() const override { return L'\0'; }
                    std::string do_grouping() const override { return "\3\0"; }// remove thousands separator
                };

                my_numpunct* numpunct = new my_numpunct;
                numpunct->comma = settings.comma;
                std::locale russian("ru_RU.UTF-8");
                file.imbue(std::locale(russian, numpunct));

                //file << std::fixed << std::setprecision(3);
                char separator = '\t';
                const QList<QString>& experiment_names = current_data_table_widget.get_experiment_names();
                const QList<ExperimentPoint>& point_list = current_data_table_widget.getPointList();

                if (settings.save_column_titles)
                {
                    if (settings.time)
                    {
                        std::wstring time_str = L"time";
                        if (settings.time_measurement != 4)
                            time_str += L", " + DataListContainer::timeMeasurement_toQString(static_cast<ExperimentPoint::TimeMeasurement>(settings.time_measurement)).toStdWString();
                        file << time_str << separator;
                    }
                    for (int i = 0; i < experiment_names.size(); i++)
                    {
                        if (settings.experiments.contains(i))
                        {
                            if (settings.x)
                                file << L"x" + std::to_wstring(i + 1) << separator;
                            if (settings.y)
                                file << experiment_names[i].toStdWString() << separator;
                        }
                    }
                    if (settings.save_norm_column)
                        file << L"norm." << separator;
                    file << L'\n';
                }

                for (const auto& time_point : point_list)
                {
                    if (settings.time)
                    {
                        if (settings.time_measurement == 4)
                            file << DataListContainer::convert_to_measure(time_point.seconds, time_point.measure) << ' ' << DataListContainer::timeMeasurement_toQString(time_point.measure).toStdWString() << separator;
                        else
                            file << DataListContainer::convert_to_measure(time_point.seconds, static_cast<ExperimentPoint::TimeMeasurement>(settings.time_measurement)) << separator;
                    }

                    for (int i = 0; i < experiment_names.size(); i++)
                    {
                        if (settings.experiments.contains(i))
                        {
                            if (settings.x)
                            {
                                if (time_point.points[i].x() < 1E+15)
                                    file << time_point.points[i].x() << separator;
                                else
                                    file << L"-" << separator;
                            }
                            if (settings.y)
                            {
                                if (time_point.points[i].y() < 1E+15)
                                {
                                    float y_value = time_point.points[i].y();
                                    if (settings.normalize_y)
                                        y_value /= time_point.normalization_value;
                                    file << y_value << separator;

                                }
                                else
                                    file << L"-" << separator;
                            }
                        }
                    }
                    if (settings.save_norm_column)
                        file << time_point.normalization_value << separator;

                    file << L'\n';
                }
                file.close();
            }
            QMessageBox::information(this, tr("File saving"), tr("Complete!"));
        }
    }
    catch(std::runtime_error& ex)
    {
        QMessageBox::critical(this, tr("Error"), tr(ex.what()));
    }
    catch(...)
    {
        QMessageBox::critical(this, tr("Error"), tr("File saving error"));
    }
    this->setEnabled(true);
    save_to_file_form->hide();
}

void MainWindow::add_current_data()
{
    if (add_mode == AddMode::AddTimePoint)
    {
        auto comboBox = DataListContainer::create_measure_ComboBox();
        QObject::connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                             [&](int index) {

                                 emit MainWindow::currentMeasureChangedInTable(index);
                             });
        double time = ui->timeNumberDoubleSpinBox->value();
        current_data_table_widget.addPoint(chart_set->chartView.getXY(), time, current_time_measure, comboBox);
        current_data_table_widget.addNormalization_line();

        auto v_scroll_bar = current_data_table_widget.table_widget->verticalScrollBar();
        v_scroll_bar->setValue(v_scroll_bar->maximum());
    }
    else if (add_mode == AddMode::AddExperimentPoint)
    {
        QPointF point = chart_set->chartView.getXY();
        int row = current_data_table_widget.table_widget->currentRow();
        int colomn = current_data_table_widget.table_widget->currentColumn();
        if (colomn > 2)
            current_data_table_widget.addExperiment_point(row, colomn - (colomn % 2), point, ui->use_normalization_checkBox->isChecked());

        if (not current_data_table_widget.select_cell_to_add_exp_point())
        {
            on_add_time_point_radioButton_clicked(true);
            ui->add_time_point_radioButton->setChecked(true);
        }
        current_data_table_widget.table_widget->setFocus();
    }
    else if (add_mode == AddMode::AddNormalizationValue)
    {
        QPointF point = chart_set->chartView.getXY();
        int row = ui->normalization_tableWidget->currentRow();
        if (row >= 0)
        {
            if (abs(point.y()) > 1e-9 )
            {
                current_data_table_widget.changeNormalization_value(row, point.y(), ui->use_normalization_checkBox->isChecked());
                current_data_table_widget.select_cell_to_add_norm_value();
            }
            else
            {
                QMessageBox::warning(this, "Warning", tr("The value must not be zero"));
            }
        }
        current_data_table_widget.norm_table_widget->setFocus();
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->modifiers() == Qt::ControlModifier)
        {
            if (keyEvent->key() == Qt::Key_Up)
            {
                current_data_table_widget.move_line(DataListContainer::upwards);
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Down)
            {
                current_data_table_widget.move_line(DataListContainer::downwards);
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Plus)
        add_current_data();
    /*else if (event->key() == Qt::Key_Up)
        on_previous_line_pushButton_clicked();
    else if (event->key() == Qt::Key_Down)
        on_next_line_pushButton_clicked();*/
    else
        QWidget::keyPressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Exit"), tr("Are you sure you want to exit?"), QMessageBox::Cancel | QMessageBox::Ok);
    if (reply == QMessageBox::Ok)
    {
        chart_settings_form->hide();
        save_to_file_form->hide();
        experiment_name_change_form->hide();
        add_new_experiment_form->hide();
        result_form->hide();
        convert_oscilloscope_file_form->hide();
        QWidget::closeEvent(event);
    }
    else
    {
        event->ignore();
    }
}


void MainWindow::on_current_table_listWidget_itemChanged(QTableWidgetItem *item)
{
    current_data_table_widget.updatePoint_in_list(item->row(), ui->use_normalization_checkBox->isChecked());
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
    this->setEnabled(false);

    result_form->show();
}

void MainWindow::showResults_form_closed()
{
    this->setEnabled(true);
    result_form->hide();
}


void MainWindow::on_opend_files_listWidget_currentRowChanged(int currentRow)
{
    auto data_list_it = chart_data_list.begin();
    std::advance(data_list_it, currentRow);
    chart_set->show_plots(*data_list_it);
    current_chart_data_index = currentRow;
    ui->currentFile_label->setText(ui->opend_files_listWidget->currentItem()->text());
}

void MainWindow::key_plus_pressed_handle_slot()
{
    add_current_data();
}

void DoubleSpinBoxContainer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Plus)
        emit key_plus_pressed();
    else
        QDoubleSpinBox::keyPressEvent(event);
}


void ExperimentPoint::append_experiment_value(QPointF point)
{
    points.append(point);
}

void ExperimentPoint::change_experiment_value(QPointF point, int index)
{
    points[index] = point;
}

void MainWindow::on_show_x_checkBox_stateChanged(int arg1)
{
    this->current_data_table_widget.set_show_x(arg1);
}


void MainWindow::on_change_experiment_name_pushButton_clicked()
{
    if (current_data_table_widget.get_experiment_names().size() == 0)
        return;

    int current_colomn = this->current_data_table_widget.table_widget->currentColumn();
    if (current_colomn <= 1)
        current_colomn = 2;

    int colomn = (current_colomn / 2)  * 2 + 1;
    experiment_name_change_form->set_current_name(this->current_data_table_widget.table_widget->horizontalHeaderItem(colomn)->text());
    experiment_name_change_form->set_colomn(colomn);
    experiment_name_change_form->show();
    experiment_name_change_form->set_focus_line_edit_2();
    this->setEnabled(false);

}


void MainWindow::on_add_new_experiment_pushButton_clicked()
{
    add_new_experiment_form->show();
    add_new_experiment_form->set_focus_line_edit();
    add_new_experiment_form->clear_line_edit();
    this->setEnabled(false);
}

void MainWindow::change_experiment_name_form_closed(const QString& new_name)
{
    this->setEnabled(true);
    if (new_name != "")
    {
        int colomn = experiment_name_change_form->get_colomn();
        current_data_table_widget.table_widget->horizontalHeaderItem(colomn)->setText(new_name);
        current_data_table_widget.changeExperiment_name(new_name, colomn / 2 - 1);
    }
}

void MainWindow::add_new_experiment_form_closed(const QString &new_name)
{
    this->setEnabled(true);
    if (new_name != "")
    {
        current_data_table_widget.addExperiment_name(new_name);

        current_data_table_widget.table_widget->insertColumn(current_data_table_widget.table_widget->columnCount());
        int x_count = current_data_table_widget.get_experiment_names().size();
        current_data_table_widget.table_widget->setHorizontalHeaderItem(current_data_table_widget.table_widget->columnCount() - 1, new QTableWidgetItem("x" + QString::number(x_count)));
        current_data_table_widget.table_widget->setColumnHidden(current_data_table_widget.table_widget->columnCount() - 1, not ui->show_x_checkBox->isChecked());
        current_data_table_widget.table_widget->insertColumn(current_data_table_widget.table_widget->columnCount());
        QTableWidgetItem* y = new QTableWidgetItem(new_name);
        y->setBackground(Y_colomn_color);
        current_data_table_widget.table_widget->setHorizontalHeaderItem(current_data_table_widget.table_widget->columnCount() - 1, y);       
    }
}


void MainWindow::on_delete_experiment_pushButton_clicked()
{
    if (current_data_table_widget.get_experiment_names().size() == 0)
        return;

    int current_colomn = this->current_data_table_widget.table_widget->currentColumn();
    if (current_colomn <= 1)
        current_colomn = 2;

    int colomn = (current_colomn / 2)  * 2 + 1;
    QString colomn_name = current_data_table_widget.table_widget->horizontalHeaderItem(colomn)->text();
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Delete experiment"), tr("Are you sure you want to delete <b>") + colomn_name + tr("</b> experiment?"), QMessageBox::Cancel | QMessageBox::Ok);
    if (reply == QMessageBox::Ok)
    {
        current_data_table_widget.table_widget->removeColumn(colomn);
        current_data_table_widget.table_widget->removeColumn(colomn - 1);
        current_data_table_widget.removeExperiment_name(colomn / 2 - 1);
    }
}


void MainWindow::on_use_normalization_checkBox_stateChanged(int arg1)
{
    for (int i = 0; i < current_data_table_widget.table_widget->rowCount(); ++i)
        current_data_table_widget.normalize_row_in_table_widget(i, arg1);
}

void MainWindow::on_add_time_point_radioButton_clicked(bool checked)
{
    if (checked)
    {
        ui->timeMeasureGroupBox->setEnabled(true);
        double_spin_box_container->setEnabled(true);
        add_mode = AddMode::AddTimePoint;
        current_data_table_widget.table_widget->clearSelection();
    }
}


void MainWindow::on_add_exp_point_radioButton_clicked(bool checked)
{
    if (checked)
    {
        ui->timeMeasureGroupBox->setEnabled(false);
        double_spin_box_container->setEnabled(false);
        add_mode = AddMode::AddExperimentPoint;
        current_data_table_widget.select_cell_to_add_exp_point();
        current_data_table_widget.norm_table_widget->clearSelection();
    }
}


void MainWindow::on_add_norm_value_radioButton_clicked(bool checked)
{
    if (checked)
    {
        //current_data_table_widget.table_widget->setEnabled(false);
        //current_data_table_widget.norm_table_widget->setEnabled(true);
        ui->timeMeasureGroupBox->setEnabled(false);
        double_spin_box_container->setEnabled(false);
        add_mode = AddMode::AddNormalizationValue;      
        current_data_table_widget.select_cell_to_add_norm_value();
        current_data_table_widget.table_widget->clearSelection();
    }
}


QWidget *DoubleSpinBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
        QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
        editor->setMinimum(-1000000000.0);
        editor->setMaximum(1000000000.0);
        editor->setDecimals(2);
        return editor;
}

void DoubleSpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
        spinBox->setValue(value);
}

void DoubleSpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    double value = spinBox->value();
    model->setData(index, value, Qt::EditRole);
}

void MainWindow::on_normalization_tableWidget_itemChanged(QTableWidgetItem *item)
{
    current_data_table_widget.changeNormalization_value(current_data_table_widget.norm_table_widget->currentRow(), item->text().toDouble(), ui->use_normalization_checkBox->isChecked());
}

void MainWindow::on_next_line_pushButton_clicked()
{
    if (not chart_data_list.empty())
    {
        chart_set->next_line();
    }
}


void MainWindow::on_previous_line_pushButton_clicked()
{
    if (not chart_data_list.empty())
    {
        chart_set->prev_line();
    }
}


void MainWindow::on_turn_on_all_lines_pushButton_clicked()
{
    if (not chart_data_list.empty())
    {
        chart_set->turn_on_all_lines();
    }
}

