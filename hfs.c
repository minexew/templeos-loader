#include "hfs.h"

size_t hfs_fget(const char* path, uint8_t* buf, size_t bufsiz) {
    size_t read_size;
    FILE* f = fopen(path,"r");

    if (f) {
        fseek(f, 0L, SEEK_END);
        read_size = ftell(f);
        fseek(f, 0L, SEEK_SET);
        
        if (read_size > bufsiz) {
            read_size = bufsiz;
        }
 
        int64_t read = fread(buf, 1, bufsiz, f);
        fclose(f);
        return (read >= 0) ? read : 0;
    }
    else {
        return 0;
    }
}

size_t hfs_fput(const char* path, const uint8_t* buf, size_t bufsiz) {
    FILE* f = fopen(path, "w");

    if (f) {
        int64_t written = fwrite(buf, 1, bufsiz, f);
        fclose(f);
        return (written >= 0) ? written : 0;
    }
    else {
        return 0;
    }
}

int hfs_stat(const char* path, struct CHostFsStat* st_out) {

    size_t read_size;

    st_out->attr = 0;
    st_out->clus = 0;
    st_out->abs_path = (char*)path;
    st_out->name = basename((char*)path);
    st_out->size = 0;

    DIR *dir = opendir(path);

    if (dir) {
        st_out->attr |= RS_ATTR_DIR;
        closedir(dir);
    }
    else {
        FILE* f = fopen(path,"r");
        if (f) {
            fseek(f, 0L, SEEK_END);
            st_out->size = ftell(f);
            fseek(f, 0L, SEEK_SET);
        }
        else {
            return -1;
        }
    }

    return 0;
}

