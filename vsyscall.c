#include "memory_map.h"
#include "symtable.h"
#include "templeos.h"
#include "vfs.h"
#include "vsyscall.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

// arch_prctl
//if not musl:
//#include <asm/prctl.h>
#include <sys/prctl.h>
#ifndef ARCH_SET_GS
#define ARCH_SET_GS             0x1001
#endif
#ifndef ARCH_SET_FS
#define ARCH_SET_FS             0x1002
#endif
long arch_prctl(int, unsigned long);

//#define trace_syscall(args) printf args
#define trace_syscall(args) do {} while(0)

int64_t vsyscall_dispatcher(int64_t num, int64_t arg1, int64_t arg2, int64_t arg3) {
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

            trace_syscall(("VSYSCALL_CLOSEDIR(%p)\n", dirp));
            return vfs_closedir(dirp);
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
            trace_syscall(("VSYSCALL_FGET(%s, %p, %d)\n", path, buf, bufsiz));

            return vfs_fget(path, buf, bufsiz);
        }
        case VSYSCALL_FPUT: {
            const char* path = (const char*) arg1;
            const uint8_t* buf = (const uint8_t*) arg2;
            size_t bufsiz = (size_t) arg3;
            trace_syscall(("VSYSCALL_FPUT(%s, %p, %d)\n", path, buf, bufsiz));

            return vfs_fput(path, buf, bufsiz);
        }
        case VSYSCALL_MEMSIZE: {
            trace_syscall(("VSYSCALL_MEMSIZE\n"));
            return FLAT_SIZE - 0x1000;
        }
        case VSYSCALL_OPENDIR: {
            const char* path = (const char*) arg1;

            trace_syscall(("VSYSCALL_OPENDIR(%s)\n", path));
            return (int64_t) vfs_opendir(path);
        }
        case VSYSCALL_PUTCHAR: {
            uint8_t c = arg1;

            static bool fmt_mode = 0;
            static char fmt_buf[16];
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
            return read(0, buf, nbytes);
        }
        case VSYSCALL_READDIR: {
            struct vfs_dir_t* dirp = (struct vfs_dir_t*) arg1;
            struct CHostFsStat* st_out = (struct CHostFsStat*) arg2;

            trace_syscall(("VSYSCALL_READDIR(%p, %p)\n", dirp, st_out));
            return vfs_readdir(dirp, st_out);
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
            trace_syscall(("VSYSCALL_STATCLUS(%d, %p)\n", (int) clus, st_out));

            int rc = vfs_statclus(clus, st_out);
            trace_syscall((" -> %d\n", rc));
            return rc;
        }
        case VSYSCALL_STAT: {
            const char* path = (const char*) arg1;
            struct CHostFsStat* st_out = (struct CHostFsStat*) arg2;
            trace_syscall(("VSYSCALL_STAT(%s, %p)\n", path, st_out));

            int rc = vfs_stat(path, st_out);
            trace_syscall((" -> %d\n", rc));
            return rc;
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
