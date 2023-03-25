#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <list>
#include <memory>
#include <vector>
#include <array>

#include <QMainWindow>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QtCharts>
#include <QtCharts/QValueAxis>

#include <QList>
#include <QVector>
#include <QSharedPointer>

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <result_chart_form.h>
#include "AxesRange.h"
#include <chagechartsettingsform.h>
#include "savetofileform.h"
#include "SaveSettings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


//Classes to work with charts


class ChartData
{
public:
    ChartData() = default;
    ChartData(std::vector<QLineSeries>&& data, AxesRange ranges, float max_Y_value, QValueAxis* axis_X = nullptr, QValueAxis* axis_Y = nullptr, const QString& title = "") :
        fit_to_line_ranges(ranges), max_Y_value(max_Y_value), qline_series_array(std::move(data)), axis_X(axis_X), axis_Y(axis_Y), title(title)
    {    };

    ~ChartData()
    {    }
    AxesRange fit_to_line_ranges;
    float max_Y_value;
    std::vector<QLineSeries> qline_series_array;
    QSharedPointer<QValueAxis> axis_X = nullptr;
    QSharedPointer<QValueAxis> axis_Y = nullptr;
    QString title;
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


class ChartSet
{
public:
    ChartSet(Ui::MainWindow* ui);

    static std::pair<QValueAxis*, QValueAxis*> createQValueAxes(AxesRange fit_to_line_range, float default_max_Y);
    void show_plots(ChartData& data);

    QChart chart;
    InteractiveChartView chartView;
    QSharedPointer<QValueAxis> current_axis_X;
    QSharedPointer<QValueAxis> current_axis_Y;
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
        point = {x, y};
    }
    ExperimentPoint(QPointF point, double seconds, TimeMeasurement measure) :
        point(point), seconds(seconds), measure(measure)
    {   }

    QPointF point;
    double seconds;
    TimeMeasurement measure = t_seconds;
};


class DataListContainer
{
public:
    DataListContainer();

    QTableWidget* table_widget = nullptr;
    QDoubleValidator* validator = nullptr;
    bool is_inserting = false;

    void addPoint(QPointF point, double time, ExperimentPoint::TimeMeasurement measure, QComboBox* comboBox);
    void deletePoint(int row);
    void updatePoint_in_list(int row);
    void updateMeasure_in_list(int row, ExperimentPoint::TimeMeasurement measure);
    const QList<ExperimentPoint>& getPointList() const;
    static QComboBox* create_measure_ComboBox();
    static double convert_to_measure(double seconds, ExperimentPoint::TimeMeasurement measure);
    static QString timeMeasurement_toQString(ExperimentPoint::TimeMeasurement measure);

    ~DataListContainer()
    {
        delete validator;        
    }

protected:

    static double convert_to_seconds(double time, ExperimentPoint::TimeMeasurement measure);   
    static ExperimentPoint::TimeMeasurement qString_to_timeMeasurement(const QString& measure);

    QList<ExperimentPoint> point_list;
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

public slots:


private slots:
    void on_pushButton_clicked();
    void on_add_current_data_pushButton_clicked();   

    void on_secondsRadioButton_clicked();
    void on_minutesRadioButton_clicked();
    void on_hoursRadioButton_clicked();
    void on_daysRadioButton_clicked();   

    void on_current_table_listWidget_itemChanged(QTableWidgetItem *item);
    void on_delete_line_from_current_list_pushButton_clicked();
    void on_showResults_pushButton_clicked();

    void handleCurrentMeasureChangedInTable(int index);
    void actionChange_axes_range_triggered();
    void action_save_to_file_triggered();
    void action_change_Y_name_to_amplitude();
    void action_change_Y_name_to_reflection();
    void action_change_Y_name_to_absorbtion();

    void change_chart_setting_form_closed(std::pair<bool, AxesRange>);
    void save_to_file_form_closed();
    void save_to_file_slot(const SaveSettings& settings);

    void on_opend_files_listWidget_currentRowChanged(int currentRow);
    void key_plus_pressed_handle_slot();

private:
    ChartSet* chart_set;
    DataListContainer current_data_table_widget;
    std::unique_ptr<DoubleSpinBoxContainer> double_spin_box_container = nullptr;

    QSharedPointer<result_chart_form> result_form;

    std::list<ChartData> chart_data_list;
    size_t current_chart_data_index = 0;

    void add_current_data();
    ExperimentPoint::TimeMeasurement current_time_measure = ExperimentPoint::TimeMeasurement::t_seconds;
    Ui::MainWindow *ui;
    QLabel* coord_label;
    std::shared_ptr<chageChartSettingsForm> chart_settings_form;
    std::shared_ptr<SaveToFileForm> save_to_file_form;

    QString prev_filepath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    void keyPressEvent(QKeyEvent *event) override;
};

float convert_string_comma_to_float(const std::wstring& num);
QString extract_file_name_from_path(const QString& path);
QString remove_filename_from_path(const QString& path);

#endif // MAINWINDOW_H
