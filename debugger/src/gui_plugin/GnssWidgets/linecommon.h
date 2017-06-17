/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Plot's Line common class.
 */
#pragma once

#include "attribute.h"

namespace debugger {

class LineCommon {
public:
    LineCommon(AttributeType &descr);

    const AttributeType &getDescription();
    unsigned size();
    void append(double y);
    void append(double x, double y);
    const char *getName() { return descr_["Name"].to_string(); }
    const char *getColor() { return color_; }

    void setPlotSize(int w, int h);
    void selectData(int start_idx, int total);
    bool getNext(int &x, int &y);
    bool getXY(int idx, int &x, int &y);
    bool getAxisValue(int axis, int idx, double &outval);
    bool getAxisValue(int axis, int idx, char *outbuf, size_t bufsz);
    void getAxisMin(int axis, char *outbuf, size_t bufsz);
    void getAxisMax(int axis, char *outbuf, size_t bufsz);
    int getNearestByX(int x);

private:
    AttributeType descr_;
    struct AxisType {
        double *data;
        double accum;
        double minVal;
        double maxVal;
    } axis_[2];
    bool is_ring_;
    int start_;
    int cnt_;
    int len_;
    char color_[8];
    char format_[16];
    double plot_w;
    double plot_h;
    double dx;
    double dy;
    int sel_start_idx;
    int sel_cnt;
};

}  // namespace debugger
