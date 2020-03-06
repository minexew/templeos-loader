#ifndef VSYSCALL_H
#define VSYSCALL_H

#include <stdint.h>

enum {
    VSYSCALL_DEBUG =        0,
    VSYSCALL_MEMSIZE =      1,
    VSYSCALL_SETFS =        2,
    VSYSCALL_SETGS =        3,
    VSYSCALL_USLEEP =       4,
    VSYSCALL_PUTCHAR =      5,
    VSYSCALL_STAT =         6,
    VSYSCALL_STATCLUS =     7,
    VSYSCALL_FGET =         8,
    VSYSCALL_READ =         9,
    VSYSCALL_ADDSYM =       10,
    VSYSCALL_FPUT =         11,
    VSYSCALL_EXIT =         12,
    VSYSCALL_OPENDIR =      13,
    VSYSCALL_READDIR =      14,
    VSYSCALL_CLOSEDIR =     15,
    VSYSCALL_MKDIR =        16,
    VSYSCALL_UNLINK =       17,
};

struct CHostFsStat {
    uint16_t    attr;
    uint16_t    pad[3];
    int64_t     clus, size;
    char*       abs_path;
    char*       name;
};

int64_t vsyscall_dispatcher(int64_t num, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4);

#endif
