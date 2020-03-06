#define _GNU_SOURCE 1  /* To pick up REG_RIP */

#include "load_kernel.h"
#include "memory_map.h"
#include "templeos.h"
#include "symtable.h"
#include "vfs.h"
#include "vsyscall.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
//#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

// backtrace
// if not musl:
//#include <execinfo.h>

void* get_pc () { return __builtin_return_address(0); }

__attribute__((noreturn)) static void bugcheck(ucontext_t* u) {
#define REG_GET(reg_) (u->uc_mcontext.gregs[(reg_)])

    // Stack trace
    uint64_t frame = REG_GET(REG_RBP);
    void* addr = (void*) REG_GET(REG_RIP);

    int dummy_pipe[2];
    pipe(dummy_pipe);

    while (1) {
        uint64_t current_frame = frame;

        printf("\tframe @ %p\t%p", (void*) current_frame, addr);

        struct sym* guest_symbol = lookupsym(addr);

        if (guest_symbol)
            printf("\t%s::%s+%lxh", guest_symbol->module, guest_symbol->name, addr - guest_symbol->address);

        printf("\n");

        // fetch next stack frame
        //printf("%p\n", current_frame);

        // probe if address is readable
        if (write(dummy_pipe[1], (void*) current_frame, 16) == -1 && errno == EFAULT)
            break;

        //read(dummy_pipe[0], &frame, 8);
        //read(dummy_pipe[0], &addr, 8);

        frame = *(uint64_t*)(current_frame);
        addr = *(void**)(current_frame + 8);
    }

    exit(-1);
}

void handler(int code, siginfo_t *info, void *ctx) {
	ucontext_t *u = (ucontext_t *)ctx;

    /*if (code == SIGILL) {
    }
	else */if (code == SIGSEGV) {
        void *addr = info->si_addr;
        int err = u->uc_mcontext.gregs[REG_ERR];
        int is_write = !!(err & 0x2);

        uint64_t rip = REG_GET(REG_RIP);

        // Neutralized in kernel via #define in VKernel.HH, but may show up in Adam + apps
        if (rip >= KERNEL_START && rip < FLAT_START + FLAT_SIZE && *(uint8_t*) rip == 0xFA) {
            //  hangs in libc :(
            // probably because we fuck with the memory so much
            //fprintf(stderr, "NOTE: patching out CLI instruction (opcode %02Xh) @ %p\n", *(uint8_t*) rip, rip);
            int NOP = 0x90;
            *(uint8_t*) rip = NOP;
            return;
        }

		fprintf(stderr, "TRAP SEGV: rip=%p addr=%p write?=%d err=%08X\n",
                (void*) rip, addr, is_write, err);
	}
	else {
        fprintf(stderr, "TRAP ??%d\n", code);
    }
    bugcheck(u);
}

/*typedef struct {
	int64_t (*Putchar)(int64_t);
} kvm_calltable_t;*/

int main(int argc, char** argv) {
	//printf("pid: %d\n", getpid());
    //printf("pc: %p\n", get_pc());

    if (argc != 6) {
        fprintf(stderr, "usage: templeos-loader <kernel> <rootfs> <rootfs writable> <vfs> <vfs writable>\n");
        exit(-1);
    }

	/* init memory map */
	if (init_memory_map() < 0)
        exit(-1);

	/* load kernel image */
    if (load_kernel(argv[1], (void*) KERNEL_START, KERNEL_END - KERNEL_START) < 0) {
        fprintf(stderr, "failed to load kernel %s\n", argv[1]);
        exit(-1);
    }

	/* locate entry point */
    struct sym* KMain_sym = findsym("VKMain");
    if (!KMain_sym) {
        fprintf(stderr, "can't find symbol \"VKMain\"\n");
        exit(-1);
    }

    struct sym* VSysCall_sym = findsym("_VSYSCALL_DISPATCHER");
    if (!VSysCall_sym) {
        fprintf(stderr, "can't find symbol \"_VSYSCALL_DISPATCHER\"\n");
        exit(-1);
    }

    /* ROOTFS init (Temple OS drive C) */
    vfs_init(argv[0], argv[2], argv[3], 0);

    /* VFS init (Temple OS drive D) */
    vfs_init(argv[0], argv[4], argv[5], 1);

    /* install trap handler */
    struct sigaction sa = { };
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    if (sigaction(SIGSEGV, &sa, NULL)) {
        perror("sigaction() failed");
        exit(1);
    }
    if (sigaction(SIGFPE, &sa, NULL)) {
        perror("sigaction() failed");
        exit(1);
    }
    if (sigaction(SIGILL, &sa, NULL)) {
        perror("sigaction() failed");
        exit(1);
    }

    //kvm_calltable_t ct;
    //ct.Putchar = &TempleOS_Putchar;
    //memcpy((void*) (KERNEL_START + sizeof(struct CBinFile)), &ct, sizeof(ct));

    //printf("_VSYSCALL: %p\n", VSysCall_sym->address);

    *(uint64_t*)VSysCall_sym->address = (uint64_t) &vsyscall_dispatcher;

//printf("putchar: %p\n", TempleOS_Putchar);
//    printf("vsyscall_dispatcher: %p\n", vsyscall_dispatcher);
	/* jump to it */
	/* past this point, Fs/Gs will be clobbered so we should not use glibc functions */
    //printf("KMain: %p\n", KMain_sym->address);
    void (*KMain)() = (void(*)()) KMain_sym->address;

    KMain();

    exit(0);
}
