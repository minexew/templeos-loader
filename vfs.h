#ifndef VFS_H
#define VFS_H

#include "vsyscall.h"

#include <stdio.h>

#define NUM_DRIVES 5

typedef int64_t clus_t;

void vfs_init(const char* argv0, const char* vfspath, const char* writepath, unsigned char dv);

size_t vfs_fget(const char* path, uint8_t* buf, size_t bufsiz, const unsigned char dv);
size_t vfs_fput(const char* path, const uint8_t* buf, size_t bufsiz, const unsigned char dv);
int vfs_stat(const char* path, struct CHostFsStat* st_out, const unsigned char dv);
int vfs_statclus(clus_t clus, struct CHostFsStat* st_out, const unsigned char dv);
int vfs_unlink(const char* path, const unsigned char dv);

int vfs_mkdir(const char* path, const unsigned char dv);

struct vfs_dir_t* vfs_opendir(const char* path, const unsigned char dv);
int vfs_readdir(struct vfs_dir_t* dirp, struct CHostFsStat* st_out, const unsigned char dv);
int vfs_closedir(struct vfs_dir_t* dirp, const unsigned char dv);

#endif
