#ifndef TEMPLEOS_LOADER_THUNK_H
#define TEMPLEOS_LOADER_THUNK_H

// "mov 64(%rsp), %rsi"                    "\r\n"
// "mov 72(%rsp), %rdx"                    "\r\n"
// "mov 80(%rsp), %rcx"                    "\r\n"
// "mov 88(%rsp), %r8"                     "\r\n"
// "mov 96(%rsp), %r9"                     "\r\n"

#define MAKE_THUNK0(func_)\
void thunk_##func_();\
    asm(\
    "thunk_" #func_ ":"                         "\r\n"\
        "push %rdi"                             "\r\n"\
        "push %rsi"                             "\r\n"\
        "push %rdx"                             "\r\n"\
        "push %rcx"                             "\r\n"\
        "push %r8"                              "\r\n"\
        "push %r9"                              "\r\n"\
        "push %r10"                             "\r\n"\
        "push %r11"                             "\r\n"\
\
        "call " #func_                          "\r\n"\
\
        "pop %r11"                              "\r\n"\
        "pop %r10"                              "\r\n"\
        "pop %r9"                               "\r\n"\
        "pop %r8"                               "\r\n"\
        "pop %rcx"                              "\r\n"\
        "pop %rdx"                              "\r\n"\
        "pop %rsi"                              "\r\n"\
        "pop %rdi"                              "\r\n"\
\
        "retq"                                  "\r\n"\
    );

#define MAKE_THUNK1(func_)\
void thunk_##func_();\
    asm(\
    "thunk_" #func_ ":"                         "\r\n"\
        "push %rdi"                             "\r\n"\
        "push %rsi"                             "\r\n"\
        "push %rdx"                             "\r\n"\
        "push %rcx"                             "\r\n"\
        "push %r8"                              "\r\n"\
        "push %r9"                              "\r\n"\
        "push %r10"                             "\r\n"\
        "push %r11"                             "\r\n"\
        "mov 72(%rsp), %rdi"                    "\r\n"\
\
        "call " #func_                          "\r\n"\
\
        "pop %r11"                              "\r\n"\
        "pop %r10"                              "\r\n"\
        "pop %r9"                               "\r\n"\
        "pop %r8"                               "\r\n"\
        "pop %rcx"                              "\r\n"\
        "pop %rdx"                              "\r\n"\
        "pop %rsi"                              "\r\n"\
        "pop %rdi"                              "\r\n"\
\
        "retq $8"                               "\r\n"\
    );

#endif
