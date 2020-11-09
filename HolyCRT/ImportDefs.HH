// CompilerB.HH (linked statically)
// import U0 ClassMemberLstDel(CHashClass *c);
// import U0 CmpCtrlDel(CCmpCtrl *cc);
// import CCmpCtrl *CmpCtrlNew(U8 *buf=NULL,I64 flags=0,U8 *filename=NULL);
// import I64 ExeFile(U8 *name,I64 ccf_flags=0);
// import I64 ExePrint(U8 *fmt,...);
// import I64 Lex(CCmpCtrl *cc);
// import U8 *LexStmt2Bin(CCmpCtrl *cc,I64 *_type,I64 cmp_flags=CMPF_PRS_SEMICOLON);
// import U8 *U(U8 *rip,I64 cnt=20,I64 seg_size=64);
// import U0 Ui(U8 *buf,U8 **_rip,I64 seg_size=64, I64 *_jmp_dst=NULL,Bool just_ins=FALSE);

// KExts
import U8 *DocSave(CDoc *doc,I64 *_size=NULL);
import Bool DocUnlock(CDoc *doc);
import Bool Ed(U8 *link_st,I64 edf_dof_flags=0);

// Host API (linked statically)
U32   HostGetApiVersion();
U8*   HostGetEnv(U8* name);
U0    HostGetLocalTime(CDateStruct* ds_out);
U0    HostInjectStaticallyLinkedSymbols();
