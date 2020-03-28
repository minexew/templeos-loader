# TempleOS user-space loader

## How is this useful?

_templeos-loader_ allows you to use your Linux system for some tasks that previously required running full TempleOS on hardware or in a VM:

- compiling the kernel, compiler, or a whole distribution ISO
- running non-interactive HolyC programs (file processing, network services)

However, the primary goal is not "to bring HolyC to Linux". The TempleOS programming environment is heavily graphics-based, and too unique to blend in with anything else. Therefore, in the long run, we would like to reproduce the complete, authentic TempleOS experience, in Linux user-space, without a clunky VM. 

## Building

glibc definitely does not appreciate the way we handle memory.
For this reason, to avoid crashing, you must use a libc that is more "accomodating", such as musl

### Build musl libc

    git clone git://git.musl-libc.org/musl musl-src
    cd musl-src
    ./configure --prefix=$PWD/../build/musl --disable-shared
    make install
    cd ..
    set PATH $PWD/build/musl/bin $PATH    # if you use a different shell than fish, the syntax will be diferent

### Build PhysFS

    git submodule update --init --recursive
    cd physfslt-3.0.2
    env CC=musl-gcc cmake -DCMAKE_INSTALL_PREFIX:PATH=$PWD/../build/physfs -DPHYSFS_BUILD_SHARED=OFF . && make install
    cd ..

### Build templeos-loader

    mkdir cmake-build-debug
    cd cmake-build-debug
    env CC=musl-gcc PHYSFSDIR=$PWD/../build/physfs/ cmake ..
    cmake --build .

One advantage of the described approach is that the resulting binary is linked statically and thus portable to any x86-64 Linux system

## Run

    # checkout shrine-v6 branch VKernel
    cd $SHRINEV6_DIR

    # Recompile the kernel (example)
    # templeos-loader <vkernel> <C:/> <C:/ writable> <D:/> <D:/ writable>
    mkdir -p Writable/Compiler Writable/Kernel
    env COMPILER=D:/Compiler STARTOS=CompileKernel.HC $LOADER_DIR/build/templeos-loader \
        $LOADER_DIR/bootstrap/VKernel.BIN \
        . Writable \
        $LOADER_DIR/bootstrap /dev/null

    # now do the same, but using the newly compiled kernel to verify that it works
    env COMPILER=D:/Compiler STARTOS=CompileKernel.HC $LOADER_DIR/build/templeos-loader \
        Writable/Kernel/VKernel.BIN.C \
        . Writable \
        $LOADER_DIR/bootstrap /dev/null

### Building TempleOS ISO from source

    # check out the TempleOS source tree
    git checkout https://github.com/cia-foundation/TempleOS.git
    cd TempleOS

    # bootstrap the necessary programs
    mkdir -p Writable/Compiler Writable/Kernel
    cp $LOADER_DIR/bootstrap/Compiler.BIN Writable/Compiler/
    cp $LOADER_DIR/bootstrap/VKernel.BIN VKernel-good.BIN
    cp $LOADER_DIR/examples/StartDoDistro.HC Writable/

    # build it!
    env STARTOS=StartDoDistro $LOADER_DIR/build/templeos-loader VKernel-good.BIN . Writable

    # look at the result
    ls -l Writable/Tmp/MyDistro.ISO.C

## Environment variables

loader is actually agnostic to these, but for now we document them here:

- `COMPILER` (default _Compiler_) - compiler binary path relative to /Compiler/
- `STARTOS` (default _StartOS_) - StartOS.HC path relative to /

Note that these files must be accessible through one of the virtualized drives!

## Other tips

- To not break into GDB every time a SEGV is trapped (e.g. when encountering an unpatched CLI instruction), use the GDB command `handle SIGSEGV nostop`

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
