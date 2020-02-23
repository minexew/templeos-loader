#include "memory_map.h"
#include "templeos.h"
#include "vfs.h"
#include "vsyscall.h"

#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

// arch_prctl
#include <asm/prctl.h>
#include <sys/prctl.h>
long arch_prctl(int, unsigned long);

//#define trace_syscall(args) printf args
#define trace_syscall(args) do {} while(0)

int64_t vsyscall_dispatcher(int64_t num, int64_t arg1, int64_t arg2, int64_t arg3) {
    switch (num) {
        case VSYSCALL_DEBUG: {
            printf("[DEBUG] %s\t%d\t%08X\n", (const char*) arg1, arg2, arg2);
            return 0;
        }
        case VSYSCALL_FREAD: {
            const char* path = (const char*) arg1;
            uint8_t* buf = (uint8_t*) arg2;
            size_t bufsiz = (size_t) arg3;
            trace_syscall(("VSYSCALL_FREAD(%s, %p, %d)", path, buf, bufsiz));

            FILE* f = vfs_fopen(path, "rb");
            if (f) {
                size_t read = fread(buf, 1, bufsiz, f);
                fclose(f);
                trace_syscall((" -> %zd\n", read));
                return read;
            }
            else {
                trace_syscall((" -> error\n"));
                return 0;
            }
        }
        case VSYSCALL_MEMSIZE: {
            trace_syscall(("VSYSCALL_MEMSIZE\n"));
            return FLAT_SIZE - 0x1000;
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
            trace_syscall(("VSYSCALL_STATCLUS(%d, %p)", (int) clus, st_out));

            int rc = vfs_statclus(clus, st_out);
            trace_syscall((" -> %d\n", rc));
            return rc;
        }
        case VSYSCALL_STAT: {
            const char* path = (const char*) arg1;
            struct CHostFsStat* st_out = (struct CHostFsStat*) arg2;
            trace_syscall(("VSYSCALL_STAT(%s, %p)", path, st_out));

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
