#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE       4096

#define SYS_FIXED_AREA_START    0x100000
// FIXME: is this guessed? should be sizeof(CSysFixedArea) in Kernel
#define SYS_FIXED_AREA_SIZE     0x4000

// Start at 16 MB into the address space
#define FLAT_START 0x1000000

// Adam needs at least 16 MB for its heap, otherwise silent heap corruption happens
// Later corruption happens in Compiler if you only allocate 32 MB
// But because file handling in VKernel currently uses up a lot of memory, we go generous:
// TempleOS declares 512Meg minimum, and so this is what it gets
// +4KiB because MAlloc seems to overallocate at the end
#define FLAT_SIZE  0x20001000

/*
 * pass 0 as kernel_start_address to let the OS decide
 */
int init_memory_map(uintptr_t kernel_start_address, size_t kernel_reserve_size, void** kernel_out);
