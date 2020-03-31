#ifndef TEMPLEOS_LOADER_H
#define TEMPLEOS_LOADER_H

#include <stdbool.h>
#include <stddef.h>

struct drive_mapping {
    char letter;
    char const* dir;
    char const* writedir;
};

int loader_main(char const* argv0,
                char const* kernel, const char* entrypoint,
                bool vfs_configured_manually, struct drive_mapping const* drive_mappings, size_t num_drive_mappings
                );

#endif
