#ifndef TEMPLEOS_LOADER_H
#define TEMPLEOS_LOADER_H

#include <stdbool.h>
#include <stddef.h>

struct drive_mapping {
    char letter;
    char const* dir;
    char const* writedir;
};

int setup_vfs(char const* argv0, struct drive_mapping const* drive_mappings, size_t num_drive_mappings);
void install_trap_handlers(void);
int loader_enter(void* kernel_base);

#endif
