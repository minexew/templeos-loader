#include "memory_map.h"

#include <stddef.h>
#include <stdio.h>

#include <sys/mman.h>

int init_memory_map(uintptr_t kernel_start_address, size_t kernel_reserve_size, void** kernel_out) {
    void* p;

    if (kernel_reserve_size > 0) {
        // align area start to page size
        uintptr_t kernel_start_align = (kernel_start_address & ~(PAGE_SIZE - 1));
        // align area end to page size
        uintptr_t kernel_size_align = ((kernel_start_address + kernel_reserve_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)) - kernel_start_align;

        p = mmap((void*) kernel_start_align, kernel_size_align,
                PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS|MAP_32BIT, -1, 0);

        if (p == MAP_FAILED) {
            perror("ulto: kernel mmap failed");
            return -1;
        }

        if ((kernel_start_align != 0 && (uintptr_t) p > kernel_start_align) || (uintptr_t) p >= INT32_MAX - kernel_reserve_size) {
            fprintf(stderr, "ulto: kernel mapped at %p (sysctl -w vm.mmap_min_addr=\"0\")\n", p);
            return -1;
        }

        if (kernel_out) {
            if (kernel_start_align == 0) {
                *kernel_out = p;
            }
            else {
                // If a fixed address was requested, return that
                *kernel_out = (void*) kernel_start_address;
            }
        }
    }

    p = mmap((void*) FLAT_START, FLAT_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC,
            MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED, -1, 0);

    if (p == MAP_FAILED) {
        perror("ulto: mmap failed");
        return -1;
    }

    p = mmap((void*) SYS_FIXED_AREA_START, SYS_FIXED_AREA_SIZE, PROT_READ|PROT_WRITE,
             MAP_PRIVATE|MAP_ANONYMOUS/*|MAP_FIXED*/, -1, 0);

    if ((uintptr_t) p != SYS_FIXED_AREA_START) {
        perror("ulto: mmap failed");
        fprintf(stderr, "ulto: mmap(SYS_FIXED_AREA_START) returns %p\n", p);
        return -1;
    }
    
    return 0;
}
