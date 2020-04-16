cmake_minimum_required(VERSION 3.1)
project("templeos-loader" C ASM)

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

# Dynamic loader

add_executable("templeos-loader" main.c)
target_include_directories("templeos-loader" PRIVATE ${PROJECT_SOURCE_DIR}/argtable3/src)
target_link_libraries("templeos-loader" PRIVATE argtable3_static "libtempleos-loader" -static)

# Fully static build

set(KERNEL_BIN ${CMAKE_CURRENT_LIST_DIR}/MiniSystem/Kernel/HolyCRT.BIN)
set(KERNEL_O32 ${CMAKE_CURRENT_BINARY_DIR}/HolyCRT.o32)
set(KERNEL_O ${CMAKE_CURRENT_BINARY_DIR}/HolyCRT.o)
set(KERNEL_THUNKS ${CMAKE_CURRENT_BINARY_DIR}/HolyCRT.thunks.s)
set(PYTHONPATH ${CMAKE_CURRENT_LIST_DIR}/dependencies/bininfo:${CMAKE_CURRENT_LIST_DIR}/dependencies/makeelf)

add_custom_command(OUTPUT ${KERNEL_O32} ${KERNEL_THUNKS}
                   COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${PYTHONPATH} ${CMAKE_CURRENT_LIST_DIR}/bin2elf.py ${KERNEL_BIN} -o ${KERNEL_O32} -t ${KERNEL_THUNKS}
                   DEPENDS ${KERNEL_BIN})

# our python library only makes ELF32. oops!
add_custom_command(OUTPUT ${KERNEL_O}
                   COMMAND objcopy -I elf32-x86-64 -O elf64-x86-64 ${KERNEL_O32} ${KERNEL_O}
                   DEPENDS ${KERNEL_O32})

add_executable("templeos" ${KERNEL_THUNKS} ${KERNEL_O} mainstatic.c)
target_include_directories("templeos" PRIVATE ${PROJECT_SOURCE_DIR}/argtable3/src)
target_link_libraries("templeos" PRIVATE argtable3_static "libtempleos-loader" -static)
# Workaround until we have a linker script
target_link_options("templeos" PUBLIC "-Wl,--omagic")
