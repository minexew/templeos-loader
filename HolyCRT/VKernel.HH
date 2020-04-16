// TODO replace syscalls with Host.HH

#define VSYSCALL_MEMSIZE    1
#define VSYSCALL_SETFS      2
#define VSYSCALL_SETGS      3
#define VSYSCALL_USLEEP     4
#define VSYSCALL_PUTCHAR    5
#define VSYSCALL_STAT       6
#define VSYSCALL_STATCLUS   7
#define VSYSCALL_FREAD      8
#define VSYSCALL_READ       9
#define VSYSCALL_ADDSYM     10
#define VSYSCALL_FWRITE     11
#define VSYSCALL_EXIT       12
#define VSYSCALL_OPENDIR    13
#define VSYSCALL_READDIR    14
#define VSYSCALL_CLOSEDIR   15
#define VSYSCALL_MKDIR      16
#define VSYSCALL_UNLINK     17

// neutralize CLI instructions
#define CLI

class CHostFsStat {
	U16 attr, pad[3];
    I64 clus, size;
    U8* abs_path;
    U8* name;
    CDateStruct datetime;
};

// TODO: not public
public _extern _VKSTART64 U0 VKStart64();
public _extern _VSYSCALL I64 VSysCall(I64 num, I64 arg1, I64 arg2, I64 arg3, I64 arg4);
