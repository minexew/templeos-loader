U0 SysGlblsInit()
{
  I64 i,j;
  //CRAXRBCRCXRDX ee;

  //CPUId(0x1,&ee);
  //sys_cache_line_width=ee.rbx.u8[1]*8;
  sys_cache_line_width=64;

  sys_focus_task=Fs;
  QueInit(&sys_macro_head);

  blkdev.dft_iso_filename	=AStrNew(DFT_ISO_FILENAME);
  blkdev.dft_iso_c_filename	=AStrNew(DFT_ISO_C_FILENAME);
  blkdev.tmp_filename		=AStrNew("~/Tmp.DD.Z");
  blkdev.dvd_boot_is_good=TRUE;
  #exe {
    if (!kernel_cfg->mount_ide_auto_hd_let)
      kernel_cfg->mount_ide_auto_hd_let='C';
    if (!kernel_cfg->mount_ide_auto_cd_let)
      kernel_cfg->mount_ide_auto_cd_let='T';
    StreamPrint("blkdev.first_hd_drv_let=%d;",
	  kernel_cfg->mount_ide_auto_hd_let);
    StreamPrint("blkdev.first_dvd_drv_let=%d;",
	  kernel_cfg->mount_ide_auto_cd_let);
  }

  DbgMode(ON);
  rev_bits_table=CAlloc(256);
  set_bits_table=CAlloc(256);
  for (i=0;i<256;i++)
    for (j=0;j<8;j++) {
      if (Bt(&i,7-j))	LBts(rev_bits_table+i,j);
      if (Bt(&i,j))	set_bits_table[i]++;
    }

  ext=CAlloc(EXT_EXTS_NUM*sizeof(U8 *));
  fp_getstr2=&SysGetStr2;
  KeyDevInit;

  #exe {
    StreamPrint("blkdev.boot_drv_let='%C';",kernel_cfg->boot_drv_let);
    StreamPrint("#exe{Option(OPTf_WARN_PAREN,OFF);}");
    StreamPrint("DskCacheInit(%s);",kernel_cfg->dsk_cache_size_exp);
    StreamPrint("#exe{Option(OPTf_WARN_PAREN,ON);}");
  };

  pow10_I64=CAlloc(sizeof(F64)*(308+308+1));
  for (i=-308;i<309;i++)
    pow10_I64[i+309]=Pow10(i);

  QueInit(&scrncast.snd_head);
  scrncast.t0_now=Now;
  scrncast.t0_tS=tS;
  scrncast.ona=scrncast.snd_head.ona=0;

  ProgressBarsRst;

  QueInit(&dev.pci_head);
  dev.mem64_ptr=mem_mapped_space;

  dbg.fun_seg_cache=CAlloc(FUN_SEG_CACHE_SIZE*sizeof(CFunSegCache));
  dbg.int_fault_code=IntFaultHndlrsNew;
}

U0 SysGrInit()
{
  text.font=sys_font_std;
  text.aux_font=sys_font_cyrillic;
  text.vga_alias	=dev.uncached_alias+VGAM_GRAPHICS;
  if (!Bt(&sys_run_level,RLf_VGA)) { //if text mode
    text.cols=80;
    text.rows=25;
    text.vga_text_alias = MAlloc(text.rows*text.cols<<1);
    MemSet(text.vga_text_alias,0,text.rows*text.cols<<1);
    text.border_chars[2] (I64)='�ͳ��ɿ�';
    text.border_chars[10](U32)='��ټ';
  } else { //if 640x480 16 color
    text.cols=GR_WIDTH/FONT_WIDTH;
    text.rows=GR_HEIGHT/FONT_HEIGHT;
    // OutU8(VGAP_IDX,VGAR_MAP_MASK);
    // OutU8(VGAP_DATA,0x0F);
    // MemSet(text.vga_alias,0,GR_WIDTH*GR_HEIGHT>>3);
    text.raw_scrn_image=CAlloc(GR_WIDTH*GR_HEIGHT/8);
    text.border_chars[2] (I64)=0x0908070605040302;
    text.border_chars[10](U32)=0x0D0C0B0A;
  }
}

U0 TimersInit()
{
  I64 i,*_q;
  U32 *_d;

  OutU8(0x43,0x34);
  OutU8(0x40,SYS_TIMER0_PERIOD);
  OutU8(0x40,SYS_TIMER0_PERIOD>>8);

  //High Precision Event Timer
  if (PCIReadU16(0,31,0,0)==0x8086) {//Intel?
//D31 F0, cfg 0xF0=RCBA of PCI-LPC Bridge
    _d=PCIReadU32(0,31,0,0xF0)(U8 *)&~0x3FFF+0x3404; //HPET cfg
//7	enable
    //1:0 HPET is at 0xFED00000,0xFED01000, 0xFED02000 or 0xFED03000.
    *_d=*_d&3|0x80;
  }

  _q=dev.uncached_alias+HPET_GCAP_ID;
  i=*_q; //i.u32[1]= period in femtoS (10e-15)
  if (100000<i.u32[1]<1000000000) {
    cnts.HPET_freq    =1000000000000000/i.u32[1];
    cnts.HPET_kHz_freq=1000000000000/i.u32[1];
    _q=dev.uncached_alias+HPET_GEN_CONF;
    *_q|=1; //Enable counting
    cnts.HPET_initial=HPET;
  } else {
    cnts.HPET_freq=0;
    cnts.HPET_kHz_freq=0;
    cnts.HPET_initial=0;
  }
}

U0 Reboot()
{//Hardware reset.
  CLI
  if (mp_cnt>1)
    MPHalt;
  *0x472(U16 *)=0;
  OutU8(0x70,0x8F);
  OutU8(0x71,0x00);
  OutU8(0x70,0x00);
  OutU8(0x92,InU8(0x92)|1);
  SysHlt;
}

U0 HashAddSysSym(U8* name, I64 addr) {
  CHashExport* tmpex;
	tmpex=CAlloc(sizeof(CHashExport));
	tmpex->str=StrNew(name);
	tmpex->type=HTT_EXPORT_SYS_SYM|HTF_IMM;
	tmpex->val=addr;
	HashAdd(tmpex,Fs->hash_table);
	SysSymImportsResolve(name,LDF_NO_ABSS|LDF_SILENT);
}

U8* ResolveJitImport(U8* thunk_addr, U8* func_name) {
  CHashExport* tmpex;
  // "ResolveJitImport(%08X, %08X => %s)\n", self_addr, func_name, func_name;
  tmpex=HashFind(func_name, Fs->hash_table,HTG_ALL-HTT_IMPORT_SYS_SYM);

  if (tmpex) {
    // emit JMP rel32 (0xE9 XX YY ZZ WW)

    // we need to generate the 32-bit value at (thunk_addr + 1)
    // with a rel32 operand relative to (thunk_addr + 5)

    U32* operand_addr = (thunk_addr + 1)(U32*);
    I64 rel_base = (thunk_addr + 5)(I64);
    I32 rel_value = tmpex(CHashFun *)->exe_addr - rel_base;  // TODO verify
    // "  %08X - %08X - %08X\n", tmpex(CHashFun *)->exe_addr, rel_base, rel_value;
    *thunk_addr = 0xE9;
    *operand_addr = rel_value;
    return tmpex(CHashFun *)->exe_addr;
  }
  else {
    // "  ERROR\n";
    // FIXME
    return 0;
  }
}

// Temporary...
U0 KMain() {}

// Entry point for the loader
U0 InitRuntime()
{
  // KStart16

  // We ignore this and instead override BIOSTotalMem
  //MEM_E801 = ;
  //MEM_E820 = ;

  // Not needed after page table setup
  //MEM_PHYSICAL_SPACE = ;

  sys_boot_src = BOOT_SRC_HARDDRV;
  sys_boot_blk = 0;     // unused for HDD boot AFAIK

  // (mem_boot_base initialized in loader.c)
  if (mem_boot_base) {
    // Loading a BIN file, save pointer to patch table
    CBinFile *bfh=mem_boot_base-sizeof(CBinFile);
    sys_boot_patch_table_base = mem_boot_base-sizeof(CBinFile)+bfh->patch_table_offset;
  }
  else {
    // Statically linked runtime, no BIN header present
    sys_boot_patch_table_base = 0;

    // TODO: We still need to fake mem_boot_base & the kernel header for Adam/ADefine.HC
  }

  mem_mapped_space = VSysCall(VSYSCALL_MEMSIZE, 0, 0, 0, 0); // may not be entirely correct ?

  // KStart32

  sys_code_bp = &(SYS_FIXED_AREA(CSysFixedArea*))->sys_code_bp;
  sys_code_bp->alloced_u8s = mem_mapped_space;   // coorect? incorrect? who knows?

  LBts(&sys_run_level,RLf_PATCHED); // TODO explain

  // FIXME: is this ok? won't it be overwritten by something?
  mem_heap_base = 0x1000000;
}

#exe {Option(OPTf_EXTERNS_TO_IMPORTS,ON);};
extern U0 CompilerInit();
#exe {Option(OPTf_EXTERNS_TO_IMPORTS,OFF);};

U0 InitRuntime3()
{
  CBlkDev *bd;
  //OutU8(0x61,InU8(0x61)&~3); //Snd;
  adam_task=Fs;
  BlkPoolsInit;
  SysGlblsInit;
  Mem32DevInit;
  // UncachedAliasAlloc; -- TODO: will we miss this?
  if (sys_boot_patch_table_base) {
    LoadKernel;
  }
  HostInjectStaticallyLinkedSymbols;
  SysGrInit;
  StrCpy(Fs->task_name,"Adam Task CPU00");
  StrCpy(Fs->task_title,Fs->task_name);
  Fs->title_src=TTS_TASK_NAME;
  Fs->win_right=text.cols-2;
  Fs->win_top++;
  TaskDerivedValsUpdate;

  SysDefinesLoad;
  Core0Init;
  //IntInit1;

  U32 host_api_version = HostGetApiVersion();

  //Before this point use $LK,"Snd",A="MN:Snd"$() and $LK,"Busy",A="MN:Busy"$()
  //to debug.  After this point, use $LK,"RawPrint",A="MN:RawPrint"$()
  LBts(&sys_run_level,RLf_RAW);
  "TempleOS V%5.3f\t%D %T\tHost API %d.%d\n\n",
	sys_os_version,sys_compile_time,sys_compile_time,
  host_api_version >> 16, host_api_version & 0xffff;

  if (host_api_version < 0x30000 || host_api_version > 0x3ffff) {
    Panic("Host API version 3.0+ required");
  }

  // Now that we have MAlloc, also create fake mem_boot_base if needed
  if (!mem_boot_base) {
    CBinFile* bfh=MAlloc(sizeof(CBinFile));
    bfh->file_size = 0x100000;    // TODO: we can put in something more meaningful?
    mem_boot_base = bfh(U8*) + sizeof(CBinFile);
  }

  //TimersInit;
  //if (BIOSTotalMem<ToI64(0.95*MEM_MIN_MEG*0x100000))
  //  RawPrint(4000,"!!! Requires $TX,"512Meg",D="DD_MEM_MIN_MEG"$ of RAM Memory !!!");

  // IntsInit;
  // "Enable IRQ's\n";
  SetRFlags(RFLAGG_NORMAL);
  // Busy(2000);
  // IntInit2;
  LBts(&sys_run_level,RLf_INTERRUPTS);

  TimeCal;
  BlkDevsInitHostFS;
  "DskChg(':');\n";
  DskChg(':');
  #exe {
    StreamPrint("HomeSet(\"%s\");\n"
	  "blkdev.ins_base0=%d;blkdev.ins_base1=%d;blkdev.ins_unit =%d;\n",
	  kernel_cfg->home_dir,
	  368, 0, 0/*blkdev.ins_base0,blkdev.ins_base1,blkdev.ins_unit*/);
  }
  Gs->idle_task->cur_dv=blkdev.let_to_drv[*blkdev.home_dir-'A'];
  DrvRep;
  if (blkdev.dvd_boot_is_good) {
    bd=Let2BlkDev(':');
    if (bd->type==BDT_ATAPI) {
      blkdev.ins_base0=bd->base0;
      blkdev.ins_base1=bd->base1;
      blkdev.ins_unit =bd->unit;
    }
  }
  LBts(&sys_run_level,RLf_BLKDEV);

    /*
  #exe {
    if (!kernel_cfg->opts[CFG_NO_MP])
      StreamPrint("\"MultiCore Start\\n\\n\";"
	    "Core0StartMP;"
	    "LBts(&sys_run_level,RLf_MP);");
  };
    */

  KbdMsInit;
  MsInit;
  //KbdInit;
  //Spawn(&MsHardDrvrInstall);
}

U0 RuntimeLoadCompiler()
{
  U8* compiler_path = HostGetEnv("COMPILER");

  if (!compiler_path) {
    compiler_path = "Compiler";
  }

  "Loading %s\n", compiler_path;
  Cd("/Compiler");
  Load(compiler_path,LDF_SILENT);
  LBts(&sys_run_level,RLf_COMPILER);
}

U0 RuntimeStartOS()
{
  DbgMode(OFF);
  cnts.time_stamp_freq_initial=TimeCal;
  Cd("/");

  U8* startos_path = HostGetEnv("STARTOS");

  if (!startos_path) {
    startos_path = "StartOS";
  }

  try ExeFile(startos_path); //Continues $LK,"/StartOS.HC",A="FL:/StartOS.HC,1"$
  catch {
    Raw(ON);
    Silent(OFF);
    GetOutOfDollar;
    PutExcept;
    Dbg;
  }

  VSysCall(VSYSCALL_EXIT, 0, 0, 0, 0);

  LBts(&sys_run_level,RLf_ADAM_SERVER);
  SrvTaskCont; //Never to return
}

asm {	ALIGN	16,OC_NOP
SYS_KERNEL_END::
#exe {
  if (kernel_cfg->opts[CFG_DBG_DISTRO])
    StreamPrint("DU8 0x%X-(SYS_KERNEL_END-SYS_KERNEL+"
	"BOOT_RAM_BASE+sizeof(CBinFile)) DUP (0);"
	"BINFILE \"%s\";",kernel_cfg->dbg_distro_start,
			  kernel_cfg->dbg_distro_file);
};
}
