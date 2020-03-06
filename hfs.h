#ifndef HFS_H
#define HFS_H

#include "templeos.h"
#include "vsyscall.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <libgen.h>

/* Must be a valid filename character according 
 * to Temple OS char_bmp_filename */
#define HFS_ESC_CHAR -
#define HFS_ESC_PATH_SEARCH(s) TO_STRING(/s)
#define HFS_ESC_PATH_START(s)  TO_CHAR(s)
#define TO_STRING(s) #s
#define TO_CHAR(s) ((#s)[0])

size_t hfs_fget(const char* path, uint8_t* buf, size_t bufsiz);
size_t hfs_fput(const char* path, const uint8_t* buf, size_t bufsiz);
int hfs_stat(const char* path, struct CHostFsStat* st_out);

#endif
