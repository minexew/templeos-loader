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

set(PYTHONPATH ${CMAKE_CURRENT_LIST_DIR}/dependencies/bininfo:${CMAKE_CURRENT_LIST_DIR}/dependencies/makeelf)

## HolyCRT

set(KERNEL_BIN ${CMAKE_CURRENT_LIST_DIR}/MiniSystem/Kernel/HolyCRT.BIN)
set(HOLYCRT_EXPORT_DEFS ${CMAKE_CURRENT_LIST_DIR}/HolyCRT/ExportDefs.HH)
set(HOLYCRT_IMPORT_DEFS ${CMAKE_CURRENT_LIST_DIR}/HolyCRT/ImportDefs.HH)

set(KERNEL_O32 ${CMAKE_CURRENT_BINARY_DIR}/HolyCRT.o32)
set(KERNEL_O ${CMAKE_CURRENT_BINARY_DIR}/HolyCRT.o)
set(HOLYCRT_EXPORT_TABLE ${CMAKE_CURRENT_BINARY_DIR}/HolyCRT.exports.s)
set(KERNEL_THUNKS ${CMAKE_CURRENT_BINARY_DIR}/HolyCRT.thunks.s)

add_custom_command(OUTPUT ${KERNEL_O32} ${KERNEL_THUNKS} ${HOLYCRT_EXPORT_TABLE}
                   COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${PYTHONPATH}
                        ${CMAKE_CURRENT_LIST_DIR}/bin2elf.py ${KERNEL_BIN}
                        -o ${KERNEL_O32}
                        --thunks-out ${KERNEL_THUNKS}
                        --export-defs ${HOLYCRT_EXPORT_DEFS}
                        --import-defs ${HOLYCRT_IMPORT_DEFS}
                        --export-table-out ${HOLYCRT_EXPORT_TABLE}
                   DEPENDS ${KERNEL_BIN} ${HOLYCRT_EXPORT_DEFS} ${HOLYCRT_IMPORT_DEFS})

# our python library only makes ELF32. oops!
add_custom_command(OUTPUT ${KERNEL_O}
                   COMMAND objcopy -I elf32-x86-64 -O elf64-x86-64 ${KERNEL_O32} ${KERNEL_O}
                   DEPENDS ${KERNEL_O32})

## Compiler

set(COMPILER_BIN ${CMAKE_CURRENT_LIST_DIR}/MiniSystem/Compiler/Compiler.BIN)
set(COMPILER_EXPORT_DEFS ${CMAKE_CURRENT_LIST_DIR}/HolyCRT/CompilerExportDefs.HH)
set(COMPILER_IMPORT_DEFS ${CMAKE_CURRENT_LIST_DIR}/HolyCRT/CompilerImportDefs.HH)

set(COMPILER_O32 ${CMAKE_CURRENT_BINARY_DIR}/Compiler.o32)
set(COMPILER_O ${CMAKE_CURRENT_BINARY_DIR}/Compiler.o)
set(COMPILER_EXPORT_TABLE ${CMAKE_CURRENT_BINARY_DIR}/Compiler.exports.s)
set(COMPILER_THUNKS ${CMAKE_CURRENT_BINARY_DIR}/Compiler.thunks.s)

add_custom_command(OUTPUT ${COMPILER_O32} ${COMPILER_THUNKS} ${COMPILER_EXPORT_TABLE}
                   COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${PYTHONPATH}
                        ${CMAKE_CURRENT_LIST_DIR}/bin2elf.py ${COMPILER_BIN}
                        -o ${COMPILER_O32}
                        --thunks-out ${COMPILER_THUNKS}
                        --export-defs ${COMPILER_EXPORT_DEFS}
                        --import-defs ${COMPILER_IMPORT_DEFS}
                        --export-table-out ${COMPILER_EXPORT_TABLE}
                        --export-main CompilerInit
                   DEPENDS ${COMPILER_BIN} ${COMPILER_EXPORT_DEFS} ${COMPILER_IMPORT_DEFS}
)

# our python library only makes ELF32. oops!
add_custom_command(OUTPUT ${COMPILER_O}
                   COMMAND objcopy -I elf32-x86-64 -O elf64-x86-64 ${COMPILER_O32} ${COMPILER_O}
                   DEPENDS ${COMPILER_O32})

add_executable("templeos"
                ${KERNEL_THUNKS} ${KERNEL_O} ${HOLYCRT_EXPORT_TABLE}
                ${COMPILER_THUNKS} ${COMPILER_O} ${COMPILER_EXPORT_TABLE}
                mainstatic.c)
target_include_directories("templeos" PRIVATE ${PROJECT_SOURCE_DIR}/argtable3/src)
target_link_libraries("templeos" PRIVATE argtable3_static "libtempleos-loader" -static)
target_link_options("templeos" PUBLIC "-Wl,-T${CMAKE_CURRENT_LIST_DIR}/elf64-x86-64.ld")
# Make .text writable (HolyC places variables there)
target_link_options("templeos" PUBLIC "-Wl,--omagic")
