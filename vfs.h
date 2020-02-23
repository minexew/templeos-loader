#ifndef VFS_H
#define VFS_H

#include "vsyscall.h"

#include <stdio.h>

typedef int64_t clus_t;

FILE *vfs_fopen(const char *path, const char *mode);
int vfs_stat(const char* path, struct CHostFsStat* st_out);
int vfs_statclus(clus_t clus, struct CHostFsStat* st_out);

#endif
