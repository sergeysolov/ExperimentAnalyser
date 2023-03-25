#ifndef SAVESETTINGS_H
#define SAVESETTINGS_H



struct SaveSettings
{
    enum FileFormat
    {
        txt,
        csv,
        xlsx
    };

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
};

#endif // SAVESETTINGS_H
