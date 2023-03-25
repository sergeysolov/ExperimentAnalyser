#ifndef AXESRANGE_H
#define AXESRANGE_H


enum AxesMode
{
    Default,
    FitToLine,
    Manual
};

struct AxesRange
{
    AxesRange() = default;
    AxesRange(float min_x, float max_x, float min_y, float max_y) :
        min_x(min_x), max_x(max_x), min_y(min_y), max_y(max_y)
    {   }
    float min_x, max_x;
    float min_y, max_y;
    AxesMode mode = AxesMode::Default;
};


#endif // AXESRANGE_H
