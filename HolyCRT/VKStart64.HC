// This is now in ASM, but there is no particular reason to keep it that way

asm {
USE64
_VKSTART64::

/*IMPORT MEM_HEAP_BASE;
IMPORT SYS_CODE_BP;
IMPORT SYS_DATA_BP;
IMPORT SYS_RUN_LEVEL;*/

	// SYS_INIT_16MEG_SYS_CODE_BP
  MOV U32 [SYS_CODE_BP],SYS_FIXED_AREA+CSysFixedArea.sys_code_bp
  MOV U32 [SYS_CODE_BP+4],0

  MOV U32 [SYS_DATA_BP],0
  MOV U32 [SYS_DATA_BP+4],0


    XOR RAX,RAX
  MOV RAX, 0x1000000    // FIXME
  //SHL EAX,10
 SHR RAX,MEM_PAG_BITS

XOR RDI, RDI
XOR RSI, RSI
 MOV EDI, U32 [MEM_HEAP_BASE]

  MOV ESI,SYS_FIXED_AREA+CSysFixedArea.sys_code_bp

  MOV EBX,U32 CBlkPool.mem_free_lst[RSI]
  MOV U32 CMemBlk.next[RDI],EBX
  MOV U32 CMemBlk.next+4[RDI],0
  MOV U32 CBlkPool.mem_free_lst[RSI],EDI
  MOV U32 CBlkPool.mem_free_lst+4[RSI],0
  MOV U32 CMemBlk.mb_signature[RDI],MBS_UNUSED_SIGNATURE_VAL
  MOV U32 CMemBlk.pags[RDI],EAX
  SHL RAX,MEM_PAG_BITS
  ADD U32 CBlkPool.alloced_u8s[RSI],EAX

  BTS U32 [SYS_RUN_LEVEL],RLf_16MEG_SYS_CODE_BP

  // KStart64
  //IMPORT SYS_RUN_LEVEL;
  //IMPORT SET_FS_BASE;
  //IMPORT SET_GS_BASE;
  BTS U32 [SYS_RUN_LEVEL],RLf_64BIT

	MOV	RAX,SYS_FIXED_AREA+CSysFixedArea.init_fpu_mmx
	FXSAVE	U64 [RAX]

//Init CPU0 Struct
  PUSH  SYS_FIXED_AREA+CSysFixedArea.adam
  PUSH  SYS_FIXED_AREA+CSysFixedArea.boot_cpu
  PUSH  0
  CALL  &CPUStructInit
  CALL  SET_GS_BASE

//Init Adam HeapCtrl
  MOV EDI,U32 SYS_FIXED_AREA+CSysFixedArea.adam_hc
  MOV EAX,U32 SYS_FIXED_AREA+CSysFixedArea.sys_code_bp
  MOV U64 CHeapCtrl.bp[RDI],RAX
  MOV U32 CHeapCtrl.hc_signature[RDI],HEAP_CTRL_SIGNATURE_VAL
  
  LEA RAX,U64 CHeapCtrl.next_um-CMemUsed.next[RDI]
  MOV U64 CHeapCtrl.next_um[RDI],RAX
  MOV U64 CHeapCtrl.last_um[RDI],RAX

  MOV EAX,U32 SYS_FIXED_AREA+CSysFixedArea.adam
  MOV U64 CHeapCtrl.mem_task[RDI],RAX
  MOV U64 CTask.code_heap[RAX],RDI
  MOV U64 CTask.data_heap[RAX],RDI
  MOV U32 CTask.task_signature[RAX],TASK_SIGNATURE_VAL
  
  BTS U32 [SYS_RUN_LEVEL],RLf_16MEG_ADAM_HEAP_CTRL

  PUSH  MEM_ADAM_STK
  PUSH  U32 SYS_FIXED_AREA+CSysFixedArea.adam
  CALL  &TaskInit
  CALL  SET_FS_BASE

  // TODO: this -- or the other way around -- needs to be done eventually for multitasking
  //MOV RSP,U64 CTask.rsp[RAX]

  RET
}
