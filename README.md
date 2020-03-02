# TempleOS user-space loader

glibc definitely does not appreciate the way we handle memory.
For this reason, to avoid crashing, you must use a libc that is more "accomodating", such as musl

## Build musl libc

    git clone git://git.musl-libc.org/musl musl-src
    cd musl-src
    ./configure --prefix=~/musl --disable-shared
    make install
    cd ..

## Build PhysFS

    curl -L -O https://icculus.org/physfs/downloads/physfs-3.0.2.tar.bz2
    tar xfa physfs-3.0.2.tar.bz2
    cd physfs-3.0.2
    env CC=musl-gcc cmake -DCMAKE_INSTALL_PREFIX:PATH=~/physfs -DPHYSFS_BUILD_SHARED=OFF . && make install

## Build templeos-loader

    mkdir build
    cd build
    env CC=musl-gcc PHYSFSDIR=~/physfs/ cmake ..
    cmake --build .

One advantage of the described approach is that the resulting binary is linked statically and thus portable to any x86-64 Linux system

## Run

    # checkout shrine-v6 branch VKernel
    cd $SHRINEV6_DIR

    # bootstrap the necessary binaries
    mkdir -p Writable/Compiler Writable/Kernel
    cp $LOADER_DIR/bootstrap/Compiler.BIN Writable/Compiler/
    cp $LOADER_DIR/bootstrap/VKernel.BIN VKernel-good.BIN

    # Recompile the kernel (example)
    # templeos-loader <vkernel> <rootfs> <rw-overlay>
    env STARTOS=CompileKernel.HC $LOADER_DIR/build/templeos-loader VKernel-good.BIN . Writable

    # now test that newly compiled kernel works as expected
    env STARTOS=CompileKernel.HC $LOADER_DIR/build/templeos-loader Writable/Kernel/VKernel.BIN.C . Writable

## Environment variables

loader is actually agnostic to these, but for now we document them here:

- `COMPILER` (default _Compiler_) - compiler binary path relative to /Compiler/
- `STARTOS` (default _StartOS_) - StartOS.HC path relative to /

Note that these paths are constrained to the virtualized rootfs!

# TODO

- Proper commandline parsing
- bug: PhysFS will not create necessary directories that exist in RO but not in RW overlay!!
- implement fopen, fread, fwrite as syscalls
- Implement date & time syscalls
- Return date & time in HostFsStat_t
- basedir() call is questionable
- Clean up syscalls
- Fix CI -- needs to also use musl libc

# Design details

To be covered in a series of blog posts.

## Calling conventions

See https://www.jwhitham.org/2015/07/redirecting-system-calls-from-c-library.html

TempleOS calling convention: RAX function(<arguments on stack>)

SysV ABI: EAX function(RDI, RSI, RDX, RCX, R8, R9)

Goal: keep VKernel Host ABI-agnostic