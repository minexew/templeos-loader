#include "templeos.h"
#include "vfs.h"

#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

enum { CLUS_MIN = 10 };
enum { COUNT = 1000 };

struct statcache {
    char *abs_path;
    char *native_path;
    struct stat st;
    uint64_t assigned_clus;
};

struct statcache cache[COUNT];

static char const* get_sandboxed_path(char const *path) {
    // path must always be absolute (start with '/')

    if (path[0] != '/') {
        fprintf(stderr, "Warning: invalid VFS path '%s'\n", path);
    }

    while (path[0] == '/') {
        path++;
    }

    return path;
}

static struct statcache* get_node_by_clus(clus_t clus) {
    intptr_t index = clus - CLUS_MIN;

    if (index >= 0 && index < COUNT && cache[index].native_path) {
        return &cache[index];
    }

    return NULL;
}

static struct statcache* get_node_by_path(char const *vfs_path) {
    char const *native_path = get_sandboxed_path(vfs_path);

    size_t i;

    for (i = 0; i < COUNT && cache[i].native_path; i++) {
        if (strcmp(cache[i].native_path, native_path) == 0) {
            return &cache[i];
        }
    }

    if (stat(native_path, &cache[i].st) != 0) {
        return NULL;
    }

    cache[i].native_path = strdup(native_path);
    cache[i].assigned_clus = CLUS_MIN + i;
    cache[i].abs_path = strdup(vfs_path);
}

FILE *vfs_fopen(const char *path, const char *mode) {
    struct statcache *cache = get_node_by_path(path);

    if (!cache) {
        return NULL;
    }

    return fopen(cache->native_path, mode);
}

int vfs_stat(const char* path, struct CHostFsStat* st_out) {
    struct statcache *cache = get_node_by_path(path);

    if (!cache) {
        return -1;
    }

    st_out->attr = 0;
    if (S_ISDIR(cache->st.st_mode)) {
        st_out->attr |= RS_ATTR_DIR;
    }
    st_out->size = cache->st.st_size;
    st_out->clus = cache->assigned_clus;
    st_out->abs_path = cache->abs_path;

    return 0;
}

int vfs_statclus(clus_t clus, struct CHostFsStat* st_out) {
    struct statcache *cache = get_node_by_clus(clus);

    if (!cache) {
        return -1;
    }

    st_out->attr = 0;
    if (S_ISDIR(cache->st.st_mode)) {
        st_out->attr |= RS_ATTR_DIR;
    }
    st_out->size = cache->st.st_size;
    st_out->clus = cache->assigned_clus;
    st_out->abs_path = cache->abs_path;

    return 0;
}
