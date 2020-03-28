#include "memory_map.h"
#include "symtable.h"
#include "templeos.h"
#include "vfs.h"
#include "vsyscall.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// arch_prctl
//if not musl:
//#include <asm/prctl.h>
#include <sys/prctl.h>
#include <sys/select.h>

#ifndef ARCH_SET_GS
#define ARCH_SET_GS             0x1001
#endif
#ifndef ARCH_SET_FS
#define ARCH_SET_FS             0x1002
#endif
long arch_prctl(int, unsigned long);

//#define trace_syscall(args) printf args
#define trace_syscall(args) do {} while(0)

int64_t vsyscall_dispatcher(int64_t num, int64_t arg1, int64_t arg2, int64_t arg3, int64_t arg4) {
    switch (num) {
        case VSYSCALL_ADDSYM: {
            const char* module = (const char*) arg1;
            const char* name = (const char*) arg2;
            void* addr = (void*) arg3;
            trace_syscall(("VSYSCALL_ADDSYM(%s, %s, %p)\n", module, name, addr));

            addsym(module, name, addr);
            return 0;
        }
        case VSYSCALL_CLOSEDIR: {
            struct vfs_dir_t* dirp = (struct vfs_dir_t*) arg1;
            unsigned char dv = arg2;

            trace_syscall(("VSYSCALL_CLOSEDIR(%p, %d)\n", dirp, dv));
            return vfs_closedir(dirp, dv);
        }
        case VSYSCALL_DEBUG: {
            printf("[DEBUG] %s\t%d\t%08X\n", (const char*) arg1, arg2, arg2);
            return 0;
        }
        case VSYSCALL_EXIT: {
            exit(arg1);
        }
        case VSYSCALL_FGET: {
            const char* path = (const char*) arg1;
            uint8_t* buf = (uint8_t*) arg2;
            size_t bufsiz = (size_t) arg3;
            unsigned char dv = arg4;
            trace_syscall(("VSYSCALL_FGET(%s, %p, %d, %d)\n", path, buf, bufsiz, dv));

            return vfs_fget(path, buf, bufsiz, dv);
        }
        case VSYSCALL_FPUT: {
            const char* path = (const char*) arg1;
            const uint8_t* buf = (const uint8_t*) arg2;
            size_t bufsiz = (size_t) arg3;
            unsigned char dv = arg4;
            trace_syscall(("VSYSCALL_FPUT(%s, %p, %d, %d)\n", path, buf, bufsiz, dv));

            return vfs_fput(path, buf, bufsiz, dv);
        }
        case VSYSCALL_MEMSIZE: {
            trace_syscall(("VSYSCALL_MEMSIZE\n"));
            return FLAT_SIZE - 0x1000;
        }
        case VSYSCALL_MKDIR: {
            const char* path = (const char*) arg1;
            unsigned char dv = arg2;

            trace_syscall(("VSYSCALL_MKDIR(%s,%d)\n", path, dv));
            return vfs_mkdir(path, dv);
        }
        case VSYSCALL_OPENDIR: {
            const char* path = (const char*) arg1;
            unsigned char dv = arg2;
            trace_syscall(("VSYSCALL_OPENDIR(%s, %d)\n", path, dv));
            return (int64_t) vfs_opendir(path, dv);
        }
        case VSYSCALL_PUTCHAR: {
            uint8_t c = arg1;

            static bool fmt_mode = 0;
            static char fmt_buf[50];
            static size_t fmt_used;

            if (!fmt_mode) {
                if (c == '$') {
                    fmt_mode = true;
                    fmt_used = 0;
                }
                else {
                    write(1, &c, 1);
                }
            }
            else {
                if (c == '$') {
                    fmt_buf[fmt_used] = 0;
                    //apply_fmt(fmt_buf);
                    fmt_mode = false;
                }
                else {
                    fmt_buf[fmt_used++] = c;

                    if (fmt_used + 1 == sizeof(fmt_buf)) {
                        c = '$';
                        write(1, &c, 1);
                        write(1, fmt_buf, fmt_used);
                        fmt_mode = false;
                    }
                }
            }

            return 0;
        }
        case VSYSCALL_READ: {
            int fd = (int) arg1;
            void *buf = (void*) arg2;
            size_t nbytes = (size_t) arg3;

            trace_syscall(("VSYSCALL_READ(%d, %p, %zu)\n", fd, buf, nbytes));

            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(0, &fds);

            struct timeval immediate = { .tv_sec = 0, .tv_usec = 100000 };

            if (select(1, &fds, NULL, NULL, &immediate)){
                fprintf(stderr, "Got input?\n");
                // As usual, it is more complicated (TASKf_AWAITING_MSG...)
                return read(0, buf, nbytes);
            }
            else {
                fprintf(stderr, "N\n");
                return 0;
            }
        }
        case VSYSCALL_READDIR: {
            struct vfs_dir_t* dirp = (struct vfs_dir_t*) arg1;
            struct CHostFsStat* st_out = (struct CHostFsStat*) arg2;
            unsigned char dv = arg3;

            trace_syscall(("VSYSCALL_READDIR(%p, %p, %d)\n", dirp, st_out, dv));
            return vfs_readdir(dirp, st_out, dv);
        }
        case VSYSCALL_SETFS: {
            int rc = arch_prctl(ARCH_SET_FS, arg1);
            trace_syscall(("VSYSCALL_SETFS(%p)\n", arg1));
            return 0;
        }
        case VSYSCALL_SETGS: {
            int rc = arch_prctl(ARCH_SET_GS, arg1);
            trace_syscall(("VSYSCALL_SETGS(%p)\n", arg1));
            return 0;
        }
        case VSYSCALL_STATCLUS: {
            clus_t clus = arg1;
            struct CHostFsStat* st_out = (struct CHostFsStat*) arg2;
            unsigned char dv = arg3;
            trace_syscall(("VSYSCALL_STATCLUS(%d, %p, %d)\n", (int) clus, st_out, dv));

            int rc = vfs_statclus(clus, st_out, dv);
            trace_syscall((" -> %d\n", rc));
            return rc;
        }
        case VSYSCALL_STAT: {
            const char* path = (const char*) arg1;
            struct CHostFsStat* st_out = (struct CHostFsStat*) arg2;
            unsigned char dv = arg3;
            trace_syscall(("VSYSCALL_STAT(%s, %p, %d)\n", path, st_out, dv));
            int rc = -1;
            rc = vfs_stat(path, st_out, dv);
            trace_syscall((" -> %d\n", rc));
            return rc;
        }
        case VSYSCALL_UNLINK: {
            const char* path = (const char*) arg1;
            unsigned char dv = arg2;

            trace_syscall(("VSYSCALL_UNLINK(%s, %d)\n", path, dv));
            return vfs_unlink(path, dv);
        }
        case VSYSCALL_USLEEP: {
            trace_syscall(("VSYSCALL_USLEEP(%d)\n", arg1));
            usleep(arg1);
            return 0;
        }
        default: {
            fprintf(stderr, "Bad syscall %d\n", num);
            //*((char*)0) = 0;        // TODO: crash
            return -1;
        }
    }
}
