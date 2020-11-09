# TempleOS user-space loader

## How is this useful?

_templeos-loader_ allows you to use your Linux system for some tasks that previously required running full TempleOS on hardware or in a VM:

- compiling the kernel, compiler, or a whole distribution ISO
- running non-interactive HolyC programs (file processing, network services)

However, the primary goal is not "to bring HolyC to Linux". The TempleOS programming environment is heavily graphics-based, and too unique to blend in with anything else. Therefore, in the long run, we would like to reproduce the complete, authentic TempleOS experience, in Linux user-space, without a clunky VM. 

## Building prerequisites

glibc definitely does not appreciate the way we handle memory.
For this reason, to avoid crashing, you must use a libc that is more "accomodating", such as musl

### Build musl libc

    git clone git://git.musl-libc.org/musl musl-src
    cd musl-src
    ./configure --prefix=$PWD/../build/musl --disable-shared
    make install
    cd ..
    export PATH=$PWD/build/musl/bin:$PATH

### Build PhysFS

    git submodule update --init --recursive
    cd physfslt-3.0.2
    env CC=musl-gcc cmake -DCMAKE_INSTALL_PREFIX:PATH=$PWD/../build/physfs -DPHYSFS_BUILD_SHARED=OFF . && make install
    cd ..

## Build TempleOS Kernel For Linux in ("static mode", recommended)

    mkdir cmake-build-debug
    cd cmake-build-debug
    env CC=musl-gcc PHYSFSDIR=$PWD/../build/physfs/ cmake ..
    cmake --build . --target templeoskernel
    cd ..

### Run

    # Example: compile the HolyC runtime
    mkdir -p CmpOutput

    env STARTOS=D:/HolyCRT/CmpHolyCRT.HC \
        ./cmake-build-debug/templeos \
        --drive=C,MiniSystem \
        --drive=D,.,CmpOutput

### Building a TempleOS ISO from source

    # check out the TempleOS source tree
    git clone https://github.com/cia-foundation/TempleOS.git
    cd TempleOS

    # bootstrap the necessary programs
    mkdir -p User/Kernel
    cp $LOADER_DIR/examples/StartDoDistro.HC User/

    # build it!
    env STARTOS=StartDoDistro $LOADER_DIR/cmake-build-debug/templeos --drive=C,.,User

    # look at the result
    ls -l User/Tmp/MyDistro.ISO.C

## Build TempleOS Loader For Linux ("dynamic mode")

    mkdir cmake-build-debug
    cd cmake-build-debug
    env CC=musl-gcc PHYSFSDIR=$PWD/../build/physfs/ cmake ..
    cmake --build . --target templeos-loader
    cd ..

## Environment variables

loader is actually agnostic to these, but for now we document them here:

- `COMPILER` (default _Compiler_) - compiler binary path relative to /Compiler/
- `STARTOS` (default _StartOS_) - StartOS.HC path relative to /

Note that these files must be accessible through one of the virtualized drives!

## Other tips

- To not break into GDB every time a SEGV is trapped (e.g. when encountering an unpatched CLI instruction), use the GDB command `handle SIGSEGV nostop`

# TODO

- bug: PhysFS will not create necessary directories that exist in RO but not in RW overlay!!
- implement fopen, fread, fwrite as syscalls
- basedir() call is questionable
- Clean up syscalls

# Design details

To be covered in a series of blog posts.

## Calling conventions

See https://www.jwhitham.org/2015/07/redirecting-system-calls-from-c-library.html

TempleOS calling convention: RAX function(<arguments on stack>)

SysV ABI: EAX function(RDI, RSI, RDX, RCX, R8, R9)

Goal: keep VKernel Host ABI-agnostic
