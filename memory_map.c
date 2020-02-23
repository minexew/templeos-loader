#include "memory_map.h"

#include <stddef.h>
#include <stdio.h>

#include <sys/mman.h>

int init_memory_map() {
    // Kernel memory (0x107C00 .. 0x140000)
    void* kernel_start = (void*) (KERNEL_START & ~(PAGE_SIZE - 1));
    size_t kernel_size = (void*) KERNEL_END - kernel_start;

    //fprintf(stderr, "kernel_start: %p\n", kernel_start);

    void* p = mmap(kernel_start, kernel_size,
            PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if (p == MAP_FAILED) {
        perror("ulto: kernel mmap failed");
        return -1;
    }

    if (p > kernel_start) {
        fprintf(stderr, "ulto: kernel mapped at %p (sysctl -w vm.mmap_min_addr=\"0\")\n", p);
        return -1;
    }

    p = mmap((void*) FLAT_START, FLAT_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC,
            MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED, -1, 0);

    if (p == MAP_FAILED) {
        perror("ulto: mmap failed");
        return -1;
    }

    p = mmap((void*) SYS_FIXED_AREA_START, SYS_FIXED_AREA_SIZE, PROT_READ|PROT_WRITE,
             MAP_PRIVATE|MAP_ANONYMOUS/*|MAP_FIXED*/, -1, 0);

    if (p != SYS_FIXED_AREA_START) {
        perror("ulto: mmap failed");
        fprintf(stderr, "ulto: mmap(SYS_FIXED_AREA_START) returns %p\n", p);
        return -1;
    }
    
    return 0;
}
