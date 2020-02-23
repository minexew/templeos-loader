struct sym {
    struct sym* next;
    char*       name;
    void*       address;
};

struct sym* lookupsym(void* address);
void addsym(const char* name, void* address);
struct sym* findsym(const char* name);
