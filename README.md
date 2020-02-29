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
    # Ensure dirs exist
    mkdir -p Writable/Compiler Writable/Kernel
    # Recompile the kernel (example)
    # templeos-loader <vkernel> <rootfs> <rw-overlay> <my-startos.hc>
    $LOADER_DIR/build/templeos-loader Kernel/VKernel.BIN . Writable CompileKernel.HC

# TODO

- Explicit handling of StartOS.HC + Compiler in VKernel (syscall to retrieve env configuration)
- Proper commandline parsing
- bug: PhysFS will not create necessary directories that exist in RO but not in RW overlay!!
- implement fopen, fread, fwrite as syscalls
- Implement date & time syscalls
- Return date & time in HostFsStat_t
- basedir() call is questionable
- Clean up syscalls
- Fix CI -- needs to also use musl libc
