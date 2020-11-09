#define _GNU_SOURCE 1  /* To pick up REG_RIP */

#include "load_kernel.h"
#include "memory_map.h"
#include "templeos.h"
#include "symtable.h"
#include "templeos-loader.h"
#include "vfs.h"
#include "vsyscall.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

    if (code == SIGFPE) {
        uint64_t rip = REG_GET(REG_RIP);

        fprintf(stderr, "TRAP FPE: rip=%p fp_ctrl=%04Xh fp_stat=%04Xh\n",
                (void*) rip, u->uc_mcontext.fpregs->cwd, u->uc_mcontext.fpregs->swd);
        bugcheck(u);
    }
	else if (code == SIGSEGV) {
        void *addr = info->si_addr;
        int err = u->uc_mcontext.gregs[REG_ERR];
        int is_write = !!(err & 0x2);

        uint64_t rip = REG_GET(REG_RIP);

        enum { X86_CLI = 0xFA };

        // Neutralized in kernel via #define in VKernel.HH, but may show up in Adam + apps
        if (/*rip >= KERNEL_START &&*/ rip < FLAT_START + FLAT_SIZE && *(uint8_t*) rip == X86_CLI) {
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

int setup_vfs(char const* argv0, struct drive_mapping const* drive_mappings, size_t num_drive_mappings) {
    bool have_C = false, have_D = false;

    for (int i = 0; i < num_drive_mappings; i++) {
        // TODO: when we allow >2 drives, idx should be just dynamically assigned
        // (kernel will call HostGetOptionDriveList to discover)
        int idx;
        if (drive_mappings[i].letter == 'C') { idx = 0; have_C = true; }
        else if (drive_mappings[i].letter == 'D') { idx = 1; have_D = true; }
        else {
            fprintf(stderr, "only C and D drives can be mounted for the time being\n");
            return -1;
        }

        // TODO: sanity check for re-definition of the same idx
        vfs_init(argv0, drive_mappings[i].dir, drive_mappings[i].writedir, idx);
    }

    // Ensure both drives are initialized to prevent a crash
    // TODO: is this needed?
    if (!have_C) { vfs_init(argv0, NULL, NULL, 0); }
    if (!have_D) { vfs_init(argv0, NULL, NULL, 1); }

    return 0;
}

void install_trap_handlers(void) {
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

    /* also save FS */
    vsyscall_save_host_fs();
}

static bool get_symbol(const char* name, void** sym) {
    struct sym* sym_ = findsym(name);

    if (!sym_) {
        fprintf(stderr, "can't find symbol \"%s\"\n", name);
        return false;
    }

    *sym = sym_->address;
    return true;
}

int loader_enter(void* kernel_base) {
	//printf("pid: %d\n", getpid());
    //printf("pc: %p\n", get_pc());

	/* locate entry point */
    uint64_t* VSYSCALL_DISPATCHER;
    uint32_t* mem_boot_base;
    void (*InitRuntime)();
    void (*InitRuntime2)();
    void (*InitRuntime3)();
    void (*RuntimeLoadCompiler)();
    void (*RuntimeStartOS)();

    if (!get_symbol("InitRuntime",          (void*) &InitRuntime)
     || !get_symbol("_VKSTART64",           (void*) &InitRuntime2)
     || !get_symbol("InitRuntime3",         (void*) &InitRuntime3)
     || !get_symbol("_VSYSCALL_DISPATCHER", (void*) &VSYSCALL_DISPATCHER)
     || !get_symbol("mem_boot_base",        (void*) &mem_boot_base)
     || !get_symbol("RuntimeLoadCompiler",  (void*) &RuntimeLoadCompiler)
     || !get_symbol("RuntimeStartOS",       (void*) &RuntimeStartOS)
        ) {
        return -1;
    }

    //kvm_calltable_t ct;
    //ct.Putchar = &TempleOS_Putchar;
    //memcpy((void*) (KERNEL_START + sizeof(struct CBinFile)), &ct, sizeof(ct));

    //printf("_VSYSCALL: %p\n", VSysCall_sym->address);

    if ((uintptr_t) kernel_base >= INT32_MAX) {
        fprintf(stderr, "assertion failure: kernel loaded too high\n");
        return -1;
    }

    // needed for LoadKernel and IDK if anything else
    *mem_boot_base = ((uint32_t) (uintptr_t) kernel_base) + sizeof(struct CBinFile);

    *VSYSCALL_DISPATCHER = (uint64_t) &vsyscall_dispatcher;

//printf("putchar: %p\n", TempleOS_Putchar);
//    printf("vsyscall_dispatcher: %p\n", vsyscall_dispatcher);
	/* jump to it */
	/* past this point, Fs/Gs will be clobbered so we should not use glibc functions */
    //printf("KMain: %p\n", KMain_sym->address);
    InitRuntime();
    InitRuntime2();
    InitRuntime3();
    RuntimeLoadCompiler();
    RuntimeStartOS();

    return 0;
}
