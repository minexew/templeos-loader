project(templeos_loader)

add_executable(templeos_loader
        loader.c
        load_kernel.c
        load_kernel.h
        memory_map.c
        memory_map.h
        symtable.c
        symtable.h
        templeos.h
        vsyscall.h
        vsyscall.c
        vfs.h vfs.c)
