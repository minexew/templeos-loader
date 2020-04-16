cmake_minimum_required(VERSION 3.1)
project("templeos-loader")

add_subdirectory(argtable3)

find_package(PhysFS REQUIRED)

add_library("libtempleos-loader"
        datetime.c
        datetime.h
        host.c
        host.h
        loader.c
        load_kernel.c
        load_kernel.h
        memory_map.c
        memory_map.h
        symtable.c
        symtable.h
        templeos.h
        templeos-loader.h
        thunk.h
        vsyscall.h
        vsyscall.c
        vfs.h
        vfs.c
        )

# HolyC code doesn't exactly follow GCC's expectations...
target_compile_options("libtempleos-loader" PUBLIC -fno-stack-protector)

target_include_directories("libtempleos-loader" PRIVATE ${PHYSFS_INCLUDE_DIR})
target_include_directories("libtempleos-loader" PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries("libtempleos-loader" PRIVATE ${PHYSFS_LIBRARY})

add_executable("templeos-loader" main.c)
target_include_directories("templeos-loader" PRIVATE ${PROJECT_SOURCE_DIR}/argtable3/src)
target_link_libraries("templeos-loader" PRIVATE argtable3_static "libtempleos-loader" -static)
