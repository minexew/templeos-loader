cmake_minimum_required(VERSION 3.1)
project("templeos-loader")

find_package(PhysFS REQUIRED)

add_executable("templeos-loader"
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

target_include_directories("templeos-loader" PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries("templeos-loader" PRIVATE ${PHYSFS_LIBRARY} -static)
target_compile_options("templeos-loader" PRIVATE -fno-stack-protector)
