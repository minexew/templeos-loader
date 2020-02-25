#include "templeos.h"
#include "vfs.h"

#include "physfs.h"

#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

// clus value of 0 is reserved for "none"
enum { CLUS_MIN = 1 };

// TempleOS often passes directory references only by means of cluster numbers.
// Clearly these have no meaning in a virtualized file system, but we must emulate them.
// For the moment we do the same for files, but it is not clear if that is necessary.
struct clus_assignment {
    char*       path;
    uint64_t    clus;
};

enum { MAX_ASSIGNMENTS = 1000 };
struct clus_assignment assignments[MAX_ASSIGNMENTS];

static struct clus_assignment* get_node_by_clus(clus_t clus) {
    intptr_t index = clus - CLUS_MIN;

    if (index >= 0 && index < MAX_ASSIGNMENTS && assignments[index].path != NULL) {
        return &assignments[index];
    }

    return NULL;
}

static struct clus_assignment* get_node_by_path(char const* path) {
    size_t i;

    for (i = 0; i < MAX_ASSIGNMENTS && assignments[i].path != NULL; i++) {
        if (strcmp(assignments[i].path, path) == 0) {
            return &assignments[i];
        }
    }

    assignments[i].path = strdup(path);
    assignments[i].clus = CLUS_MIN + i;
    return &assignments[i];
}

void vfs_init(const char* argv0, const char* vfspath, const char* writepath) {
  // TODO: handle errors
  PHYSFS_init(argv0);
  PHYSFS_setWriteDir(writepath);

  // "The write dir is not included in the search path unless you specifically add it."
  PHYSFS_mount(writepath, "/", 1);
  PHYSFS_mount(vfspath, "/", 1);
}

size_t vfs_fget(const char* path, uint8_t* buf, size_t bufsiz) {
    PHYSFS_File* f = PHYSFS_openRead(path);

    if (f) {
        int64_t read = PHYSFS_readBytes(f, buf, bufsiz);
        PHYSFS_close(f);
        return (read >= 0) ? read : 0;
    }
    else {
        return 0;
    }
}

size_t vfs_fput(const char* path, const uint8_t* buf, size_t bufsiz) {
    PHYSFS_File* f = PHYSFS_openWrite(path);

    if (f) {
        int64_t written = PHYSFS_writeBytes(f, buf, bufsiz);
        PHYSFS_close(f);
        return (written >= 0) ? written : 0;
    }
    else {
        return 0;
    }
}

// FILE *vfs_fopen(const char *path, const char *mode) {
//     struct statcache *cache = get_node_by_path(path);

//     if (!cache) {
//         return NULL;
//     }

//     return fopen(cache->native_path, mode);
// }

int vfs_stat(const char* path, struct CHostFsStat* st_out) {
    PHYSFS_Stat st;

    if (!PHYSFS_stat(path, &st)) {
        return -1;
    }

    struct clus_assignment* assignment = get_node_by_path(path);

    if (!assignment) {
        return -1;
    }

    st_out->attr = 0;
    if (st.filetype == PHYSFS_FILETYPE_DIRECTORY) {
        st_out->attr |= RS_ATTR_DIR;
    }
    st_out->size = st.filesize;
    st_out->clus = assignment->clus;
    st_out->abs_path = assignment->path;

    return 0;
}

int vfs_statclus(clus_t clus, struct CHostFsStat* st_out) {
    struct clus_assignment* assignment = get_node_by_clus(clus);

    if (!assignment) {
        return -1;
    }

    PHYSFS_Stat st;

    if (!PHYSFS_stat(assignment->path, &st)) {
        return -1;
    }

    st_out->attr = 0;
    if (st.filetype == PHYSFS_FILETYPE_DIRECTORY) {
        st_out->attr |= RS_ATTR_DIR;
    }
    st_out->size = st.filesize;
    st_out->clus = assignment->clus;
    st_out->abs_path = assignment->path;

    return 0;
}
