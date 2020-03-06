#include "templeos.h"
#include "vfs.h"

#include "physfs.h"

#include <libgen.h>
#include <stdint.h>
#include <stdlib.h>
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

struct vfs_dir_t {
    char* path;
    char** list;
    size_t pos;
};

enum { MAX_ASSIGNMENTS = 10000 };       // FIXME: handle dynamically
struct clus_assignment assignments[NUM_DRIVES][MAX_ASSIGNMENTS];

static struct clus_assignment* get_node_by_clus(clus_t clus, const unsigned char dv) {
    intptr_t index = clus - CLUS_MIN;

    if (index >= 0 && index < MAX_ASSIGNMENTS && assignments[dv][index].path != NULL) {
        return &assignments[dv][index];
    }

    return NULL;
}

static struct clus_assignment* get_node_by_path(char const* path, const unsigned char dv) {
    size_t i;

    for (i = 0; i < MAX_ASSIGNMENTS && assignments[dv][i].path != NULL; i++) {
        if (strcmp(assignments[dv][i].path, path) == 0) {
            return &assignments[dv][i];
        }
    }

    assignments[dv][i].path = strdup(path);
    assignments[dv][i].clus = CLUS_MIN + i;
    return &assignments[dv][i];
}

void vfs_init(const char* argv0, const char* vfspath, const char* writepath, const unsigned char dv) {
    // TODO: handle errors
    printf("Init dv = %d\n",dv);
    PHYSFS_init(argv0, dv);
    PHYSFS_setWriteDir(writepath, dv);

    // "The write dir is not included in the search path unless you specifically add it."
    PHYSFS_mount(writepath, "/", 1, dv);
    PHYSFS_mount(vfspath, "/", 1, dv);
}

int vfs_closedir(struct vfs_dir_t* dirp, const unsigned char dv) {
    free(dirp->path);
    PHYSFS_freeList(dirp->list, dv);
    free(dirp);
}

size_t vfs_fget(const char* path, uint8_t* buf, size_t bufsiz, const unsigned char dv) {
    PHYSFS_File* f = PHYSFS_openRead(path, dv);

    if (f) {
        int64_t read = PHYSFS_readBytes(f, buf, bufsiz, dv);
        PHYSFS_close(f, dv);
        return (read >= 0) ? read : 0;
    }
    else {
        return 0;
    }
}

size_t vfs_fput(const char* path, const uint8_t* buf, size_t bufsiz, const unsigned char dv) {
    PHYSFS_File* f = PHYSFS_openWrite(path, dv);

    if (f) {
        int64_t written = PHYSFS_writeBytes(f, buf, bufsiz, dv);
        PHYSFS_close(f, dv);
        return (written >= 0) ? written : 0;
    }
    else {
        return 0;
    }
}

int vfs_mkdir(const char* path, const unsigned char dv) {
    if (PHYSFS_mkdir(path, dv) ) {
        return 0;
    }
    else {
        return -1;
    }
}

struct vfs_dir_t* vfs_opendir(const char* path, const unsigned char dv) {
    char** list = PHYSFS_enumerateFiles(path, dv);

    if (!list) {
        return NULL;
    }

    struct vfs_dir_t* d = (struct vfs_dir_t*) malloc(sizeof(struct vfs_dir_t));
    d->path = strdup(path);
    d->list = list;
    d->pos = 0;
    return d;
}

int vfs_readdir(struct vfs_dir_t* dirp, struct CHostFsStat* st_out, const unsigned char dv) {
    if (!dirp->list[dirp->pos]) {
        return -1;
    }

    char buf[4096];
    snprintf(buf, sizeof(buf), "%s/%s", dirp->path, dirp->list[dirp->pos++]);

    if (vfs_stat(buf, st_out, dv) != 0) {
        // maybe file disappeared in the mean-time?
        // re-try in a recursive way. not the cleanest design.
        return vfs_readdir(dirp, st_out, dv);
    }

    return 0;
}

int vfs_stat(const char* path, struct CHostFsStat* st_out, const unsigned char dv) {
    PHYSFS_Stat st;

    if (!PHYSFS_stat(path, &st, dv)) {
        return -1;
    }

    struct clus_assignment* assignment = get_node_by_path(path, dv);

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
    st_out->name = basename(st_out->abs_path);

    return 0;
}

int vfs_statclus(clus_t clus, struct CHostFsStat* st_out, const unsigned char dv) {
    struct clus_assignment* assignment = get_node_by_clus(clus, dv);

    if (!assignment) {
        return -1;
    }

    PHYSFS_Stat st;

    if (!PHYSFS_stat(assignment->path, &st, dv)) {
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

int vfs_unlink(const char* path, const unsigned char dv) {
    if (PHYSFS_delete(path, dv) ) {
        return 0;
    }
    else {
        return -1;
    }
}
