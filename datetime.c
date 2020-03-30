#include "datetime.h"

void timespec_to_CDateStruct_local(time_t tv_sec, long tv_nsec, struct CDateStruct* ds_out) {
    struct tm tm;
    if (!localtime_r(&tv_sec, &tm)) {
        // TODO: what should we do? what *can* we do?
        memset(ds_out, 0, sizeof(*ds_out));
        return;
    }

    int sec10000 = tv_nsec / 100;

    ds_out->sec10000 = sec10000 % 100;
    ds_out->sec100 = sec10000 / 100;
    ds_out->sec = tm.tm_sec;
    ds_out->min = tm.tm_min;
    ds_out->hour = tm.tm_hour;
    ds_out->day_of_week = tm.tm_wday;       // correct?
    ds_out->day_of_mon = tm.tm_mday;        // correct?
    ds_out->mon = tm.tm_mon;
    ds_out->year = tm.tm_year;
}
