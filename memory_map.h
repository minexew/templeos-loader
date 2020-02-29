#define PAGE_SIZE       4096

#define SYS_FIXED_AREA_START    0x100000
// FIXME: is this guessed? should be sizeof(CSysFixedArea) in Kernel
#define SYS_FIXED_AREA_SIZE     0x4000

#define KERNEL_START    0x107c00
#define KERNEL_END      0x140000

// Start at 16 MB into the address space
#define FLAT_START 0x1000000

// Adam needs at least 16 MB for its heap, otherwise silent heap corruption happens
// Later corruption happens in Compiler if you only allocate 32 MB
// But because file handling in VKernel currently uses up a lot of memory, we go generous:
// TempleOS declares 512Meg minimum, and so this is what it gets
// +4KiB because MAlloc seems to overallocate at the end
#define FLAT_SIZE  0x20001000

int init_memory_map();
