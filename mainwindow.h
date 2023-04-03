#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <list>
#include <memory>
#include <vector>
#include <array>
#include <algorithm>

#include <QMainWindow>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QtCharts>
#include <QtCharts/QValueAxis>

#include <QList>
#include <QVector>
#include <QSharedPointer>

#include <QTextStream>
#include <QFile>

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <result_chart_form.h>
#include "AxesRange.h"
#include <chagechartsettingsform.h>
#include "savetofileform.h"
#include "SaveSettings.h"
#include "experiment_name_form.h"
#include "addnewexperimentform.h"
#include "convert_oscilloscope_fileform.h"

#include <OpenXLSX.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


//Classes to work with charts

class DoubleSpinBoxDelegate : public QStyledItemDelegate {
public:

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};


class ChartData
{
public:
    ChartData() = default;
    ChartData(std::vector<QLineSeries>&& data, AxesRange ranges, float max_Y_value, QValueAxis* axis_X = nullptr, QValueAxis* axis_Y = nullptr, const QString& title = "") :
        fit_to_line_ranges(ranges), max_Y_value(max_Y_value), qline_series_array(std::move(data)), axis_X(axis_X), axis_Y(axis_Y), title(title)
    {
        lines_enabled_array.assign(qline_series_array.size(), true);
    };

    ~ChartData()
    {    }
    AxesRange fit_to_line_ranges;
    float max_Y_value;
    std::vector<QLineSeries> qline_series_array;
    std::vector<bool> lines_enabled_array;
    QSharedPointer<QValueAxis> axis_X = nullptr;
    QSharedPointer<QValueAxis> axis_Y = nullptr;
    QString title;
    int current_line = -1;
};


class InteractiveChartView : public QChartView {

public:
    InteractiveChartView(QWidget *parent = nullptr) : QChartView(parent)
    {    }

void setUI(Ui::MainWindow *ui)
{
    this->ui = ui;
}

QPointF getXY() const
{
    return QPointF(current_x, current_y);
}

protected:
    Ui::MainWindow *ui;

    float cross_scope = 10;
    float current_x = 0, current_y = 0;

    QLineSeries* cross_X = nullptr;
    QLineSeries* cross_Y = nullptr;

    void mousePressEvent(QMouseEvent *event) override;

private slots:         
};


class ChartSet : public QObject
{

    Q_OBJECT
public:
    ChartSet(Ui::MainWindow* ui, QListWidget* lines_list_widget, std::list<ChartData>* chart_data_list, const size_t* current_chart_data_index);

    static std::pair<QValueAxis*, QValueAxis*> createQValueAxes(AxesRange fit_to_line_range, float default_max_Y);
    void show_plots(ChartData& data, QChart::AnimationOption option = QChart::AllAnimations, bool rebuild_lines_list_widget = true);
    void next_line();
    void prev_line();
    void turn_on_all_lines();

    QChart chart;
    InteractiveChartView chartView;
    QSharedPointer<QValueAxis> current_axis_X;
    QSharedPointer<QValueAxis> current_axis_Y;
    QListWidget* lines_list_widget;
    std::list<ChartData>* chart_data_list;
    const size_t* current_chart_data_index = 0;

public slots:
    void check_box_in_lines_widget_list_state_changed(int state);
};

class ExperimentPoint
{
public:
    enum TimeMeasurement
    {
        t_seconds,
        t_minutes,
        t_hours,
        t_days,
        categorial
    };

    ExperimentPoint(float x, float y, double seconds, TimeMeasurement measure) :
        seconds(seconds), measure(measure)
    {
        points.append({x, y});
    }
    ExperimentPoint(QPointF point, double seconds, TimeMeasurement measure) :
        seconds(seconds), measure(measure)
    {
        points.append(point);
    }
    ExperimentPoint(double seconds, TimeMeasurement measure) :
        seconds(seconds), measure(measure)
    {   }

    void append_experiment_value(QPointF point);
    void change_experiment_value(QPointF point, int index);

    QList<QPointF> points;
    double seconds;
    double normalization_value = 1;
    bool changed_normalize = false;
    TimeMeasurement measure = t_seconds;
};


class DataListContainer
{
public:
    DataListContainer() = default;

    QTableWidget* table_widget = nullptr;
    QTableWidget* norm_table_widget = nullptr;
    bool is_inserting = false;
    bool auto_cell_switch = true;

    void addTimePoint(double time, ExperimentPoint::TimeMeasurement measure, QComboBox* comboBox);
    void addPoint(QPointF point, double time, ExperimentPoint::TimeMeasurement measure, QComboBox* comboBox);
    void deletePoint(int row);
    void updatePoint_in_list(int row, bool use_normalization);
    void updateMeasure_in_list(int row, ExperimentPoint::TimeMeasurement measure);
    void addExperiment_point(int row, int colomn, QPointF point, bool normalize);
    void addExperiment_name(QString name);
    void addNormalization_line();
    void changeNormalization_value(int row, double value, bool use_normalization);
    void changeExperiment_name(QString name, int index);
    void removeExperiment_name(int index);
    void normalize_row_in_table_widget(int row, bool noramlize);
    void set_show_x(bool is_show);
    bool select_cell_to_add_exp_point();
    bool select_cell_to_add_norm_value();

    const QList<QString>& get_experiment_names() const;
    const QList<ExperimentPoint>& getPointList() const;

    static QComboBox* create_measure_ComboBox();
    static double convert_to_measure(double seconds, ExperimentPoint::TimeMeasurement measure);
    static QString timeMeasurement_toQString(ExperimentPoint::TimeMeasurement measure);
    static ExperimentPoint::TimeMeasurement qString_to_timeMeasurement(const QString& measure);
    static double convert_to_seconds(double time, ExperimentPoint::TimeMeasurement measure);

protected:          
    bool show_x = true;
    QList<QString> experiment_names;
    QVector<ExperimentPoint> point_list;
};


class DoubleSpinBoxContainer : public QDoubleSpinBox
{
    Q_OBJECT
public:
    DoubleSpinBoxContainer(QWidget *parent = nullptr) : QDoubleSpinBox(parent)
    {    }

signals:
    void key_plus_pressed();
protected:
    QDoubleSpinBox* double_spin_box;
    void keyPressEvent(QKeyEvent *event) override;
};


//UI Main Class
class MainWindow : public QMainWindow   
{
    Q_OBJECT

signals:
    void currentMeasureChangedInTable(int index);

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum AddMode
    {
        AddTimePoint,
        AddExperimentPoint,
        AddNormalizationValue
    };

private slots:
    void on_pushButton_clicked();
    void action_convert_oscilloscope_file_triggered();
    void convert_oscilloscope_form_closed();
    void action_open_excel_file_triggered();
    void on_add_current_data_pushButton_clicked();   

    void on_secondsRadioButton_clicked();
    void on_minutesRadioButton_clicked();
    void on_hoursRadioButton_clicked();
    void on_daysRadioButton_clicked();   

    void on_current_table_listWidget_itemChanged(QTableWidgetItem *item);
    void on_delete_line_from_current_list_pushButton_clicked();
    void on_showResults_pushButton_clicked();
    void showResults_form_closed();

    void handleCurrentMeasureChangedInTable(int index);
    void actionChange_axes_range_triggered();
    void action_save_to_file_triggered();
    void action_change_Y_name_to_amplitude();
    void action_change_Y_name_to_reflection();
    void action_change_Y_name_to_absorbtion();
    void action_auto_cell_switch_check_box_state_changed(bool checked);

    void change_chart_setting_form_closed(std::pair<bool, AxesRange>);
    void save_to_file_form_closed();
    void save_to_file_slot(const SaveSettings& settings);

    void on_opend_files_listWidget_currentRowChanged(int currentRow);
    void key_plus_pressed_handle_slot();

    void on_show_x_checkBox_stateChanged(int arg1);
    void on_change_experiment_name_pushButton_clicked();
    void on_add_new_experiment_pushButton_clicked();
    void change_experiment_name_form_closed(const QString& new_name);
    void add_new_experiment_form_closed(const QString& new_name);
    void on_delete_experiment_pushButton_clicked();
    void on_use_normalization_checkBox_stateChanged(int arg1);
    void on_add_time_point_radioButton_clicked(bool checked);
    void on_add_exp_point_radioButton_clicked(bool checked);
    void on_add_norm_value_radioButton_clicked(bool checked);

    void on_normalization_tableWidget_itemChanged(QTableWidgetItem *item);

    void on_next_line_pushButton_clicked();
    void on_previous_line_pushButton_clicked();
    void on_turn_on_all_lines_pushButton_clicked();
private:
    Ui::MainWindow *ui;
    ChartSet* chart_set;
    DataListContainer current_data_table_widget;
    std::unique_ptr<DoubleSpinBoxContainer> double_spin_box_container = nullptr;

    AddMode add_mode = AddMode::AddTimePoint;

    std::list<ChartData> chart_data_list;
    size_t current_chart_data_index = 0;

    void add_current_data();
    ExperimentPoint::TimeMeasurement current_time_measure = ExperimentPoint::TimeMeasurement::t_seconds;
    int save_every_value = 1;

    std::unique_ptr<result_chart_form> result_form;
    std::unique_ptr<chageChartSettingsForm> chart_settings_form;
    std::unique_ptr<SaveToFileForm> save_to_file_form;
    std::unique_ptr<Experiment_Name_Form> experiment_name_change_form = nullptr;
    std::unique_ptr<AddNewExperimentForm> add_new_experiment_form = nullptr;
    std::unique_ptr<ConvertOscilloscopeFileForm> convert_oscilloscope_file_form = nullptr;

    QString prev_open_filepath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString prev_save_filepath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};

float convert_string_comma_to_float(const std::wstring& num);
QString extract_file_name_from_path(const QString& path);
QString remove_filename_from_path(const QString& path);

#endif // MAINWINDOW_H
