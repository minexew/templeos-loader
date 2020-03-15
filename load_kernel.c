#include "host.h"
#include "load_kernel.h"
#include "symtable.h"
#include "templeos.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void LoadOneImport(size_t module_base, uint8_t** _src) {
    // wtf!

    uint8_t *src = *_src;
    uint8_t etype;

    uintptr_t func_addr = 0;
    //CHashImport *tmpiss;
    bool first = true;

    while ((etype = *src++) != IET_END) {
        uintptr_t i = 0;
        memcpy(&i, src, 4);
        src += 4;
        char* name = src;
        src += strlen(name) + 1;

        if (*name) {
            if (!first) {
                *_src = name - 5;
                return;
            }
            else {
                first = false;
                // resolve only Host API calls, the kernel itself will take care of the rest later
                func_addr = (uintptr_t) host_get_API_by_name(name);
            }
        }

        uintptr_t ptr2=module_base+i;
        i = func_addr;

        if (!i) {
            continue;
        }

          switch (etype) {
            case IET_REL_I8:  *(uint8_t*)ptr2 =i-ptr2-1; break;
            case IET_IMM_U8:  *(uint8_t*)ptr2 =i;  break;
            case IET_REL_I16: *(uint16_t*)ptr2=i-ptr2-2; break;
            case IET_IMM_U16: *(uint16_t*)ptr2=i; break;
            case IET_REL_I32: *(uint32_t*)ptr2=i-ptr2-4; break;
            case IET_IMM_U32: *(uint32_t*)ptr2=i; break;
            case IET_REL_I64: *(uint64_t*)ptr2=i-ptr2-8; break;
            case IET_IMM_I64: *(uint64_t*)ptr2=i; break;
          }
    }

    *_src = src - 1;
}


int load_kernel(const char* path, void* address, size_t max_size) {
    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        perror("ulto: open failed");
        return -1;
    }

    struct CBinFile bin_file;

    if (read(fd, &bin_file, sizeof(bin_file)) != sizeof(bin_file)) {
        fprintf(stderr, "ulto: kernel unexpected EOF");
        return -1;
    }

    //printf("jmp\t%04Xh\n", bin_file.jmp);
    //printf("module_align_bits\t%d\n", bin_file.module_align_bits);
    //printf("signature\t%08X\n", bin_file.signature);
    //printf("module_org\t%016lXh\n", bin_file.module_org);
    //printf("patch_table_offset\t%lXh\n", bin_file.patch_table_offset);
    //printf("file_size\t%ld\n", bin_file.file_size);

    // Load starting at 7c00, INCLUDING THE BIN HEADER
    // The header is not actually needed for anything at runtime (I think),
    // but we want to match the addresses used by TOS's bootloader
    // (except for the +0x100000, of course), so that Un(addr) gives
    // the same as objdump of the image
    lseek(fd, 0, SEEK_SET);
    read(fd, address, max_size);

    //lseek(fd, bin_file.patch_table_offset, SEEK_SET);

    size_t module_base = (uintptr_t) address + sizeof(struct CBinFile);

    uint8_t* src = address + bin_file.patch_table_offset;
    uint8_t etype;
    while ((etype = *src++) != IET_END) {
        uintptr_t i = 0;
        memcpy(&i, src, 4);
        src += 4;
        char* name = src;
        src += strlen(name) + 1;

        //printf("%d\t%08lX\t%s\n", etype, i, name);

        switch (etype) {
            case IET_REL32_EXPORT:
            case IET_IMM32_EXPORT:
            case IET_REL64_EXPORT:
            case IET_IMM64_EXPORT:
                if (etype == IET_REL32_EXPORT || etype == IET_REL64_EXPORT)
                    i += module_base;

                addsym("Kernel", name, (void*) i);
                break;

            case IET_REL_I0:
            case IET_IMM_U0:
            case IET_REL_I8:
            case IET_IMM_U8:
            case IET_REL_I16:
            case IET_IMM_U16:
            case IET_REL_I32:
            case IET_IMM_U32:
            case IET_REL_I64:
            case IET_IMM_I64:
                src = name - 5;
                LoadOneImport(module_base, &src);
                break;

            case IET_ABS_ADDR:
                // Normally this would be done in KStart32: Patch abs addresses
                for (uint32_t j = 0; j < i; j++) {
                    uint32_t offset;
                    memcpy(&offset, src, 4);
                    src += 4;
                    //fprintf(stderr, "Patch offset %08X\n", offset);
                    *(uint32_t*)(module_base + offset) += module_base;
                }
                break;
        }
    }

    close(fd);
    return 0;
}
