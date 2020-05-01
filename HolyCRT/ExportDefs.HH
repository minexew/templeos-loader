U32 mem_boot_base;
U64 _VSYSCALL_DISPATCHER;

U0 InitRuntime();
U0 _VKSTART64();
U0 InitRuntime3();
U0 RuntimeStartOS();

U32 sys_run_level;
Bool Cd(U8 *dirname, Bool make_dirs);
U0 HashAddSysSym(U0* name, I64 addr);
