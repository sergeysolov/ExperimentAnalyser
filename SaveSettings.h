#ifndef SAVESETTINGS_H
#define SAVESETTINGS_H
#include <set>

struct SaveSettings
{

    SaveSettings() = default;
    SaveSettings(bool time, bool x, bool y, int time_measurement) :
        time(time), x(x), y(y), time_measurement(time_measurement)
    {   }
    SaveSettings(bool time, bool x, bool y, int time_measurement, bool comma) :
        time(time), x(x), y(y), time_measurement(time_measurement), comma(comma)
    {   }
    bool time = false;
    bool x = false;
    bool y = false;
    int time_measurement = 0;
    bool comma = true;

    bool normalize_y = false;
    bool save_norm_column = false;
    bool save_column_titles = false;
    std::set<int> experiments;
};

#endif // SAVESETTINGS_H
