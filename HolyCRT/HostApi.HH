#exe {Option(OPTf_EXTERNS_TO_IMPORTS,ON);};
public extern U32   HostGetApiVersion();
public extern U8*   HostGetEnv(U8* name);
public extern U0    HostGetLocalTime(CDateStruct* ds_out);
#exe {Option(OPTf_EXTERNS_TO_IMPORTS,OFF);};
