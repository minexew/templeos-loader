#ifndef TEMPLEOS_LOADER_DATETIME_H
#define TEMPLEOS_LOADER_DATETIME_H

#include "templeos.h"

#include <time.h>

void timespec_to_CDateStruct_local(time_t tv_sec, long tv_nsec, struct CDateStruct* ds_out);

#endif
