U0 HostFsDirEntryInit(CDirEntry* de, CHostFsStat* st)
{
  // TODO: very confused about these
  // de->next = de->parent = de->sub = NULL;
  // de->full_name = NULL;
  // de->user_data = de->user_data2 = 0;

  de->attr = FileAttr(st->name, st->attr);
  StrCpy(de->name, st->name);   // FIXME: bounds etc
  de->clus = st->clus;
  de->size = st->size;
  de->datetime = Struct2Date(&st->datetime);
}

// BUG: if dir == "/", there will be duplicate slash. But loader doesn't care.
U8* HostFsMakeAbsPath(U8* dir, U8* name) {
  U8* abs_path = MAlloc(StrLen(dir)+1+StrLen(name)+1);
  StrCpy(abs_path, dir);
  CatPrint(abs_path, "/");
  CatPrint(abs_path, name);
  return abs_path;
}

Bool HostFsFileFind(CDrv *dv,I64 cur_dir_clus,U8 *name,
        CDirEntry *_res,I64 fuf_flags=0)
{//$LK,"FUF_JUST_DIRS",A="MN:FUF_JUST_DIRS"$, $LK,"FUF_JUST_FILES",A="MN:FUF_JUST_FILES"$
  CDirEntry *abs_path;
  U8 dname[CDIR_FILENAME_LEN];
  Bool res=FALSE;
  if (fuf_flags&~FUG_FILE_FIND)
    throw('FUF');
  MemSet(_res,0,sizeof(CDirEntry));
  DrvChk(dv);
  if (dv->fs_type!=FSt_HOSTFS)
    PrintErr("Not HostFS Drv\n");
  else if (!CFileNameTo(dname,name))
    PrintErr("Invalid FileName: \"%s\".\n",name);
  else {
    //unlock=DrvLock(dv);

    CHostFsStat stat;

    // Build absolute path, if needed
    if (cur_dir_clus) {
      if (VSysCall(VSYSCALL_STATCLUS, cur_dir_clus, &stat, dv->prt_num, 0) == 0) {
        abs_path = HostFsMakeAbsPath(stat.abs_path, name);
      }
      else {
        PrintErr("Invalid directory (clus %d)", cur_dir_clus);
        abs_path = 0;
      }
    }
    else {
      abs_path = HostFsMakeAbsPath("/", name);
    }

    if (abs_path && VSysCall(VSYSCALL_STAT, abs_path, &stat, dv->prt_num, 0) == 0) {
      if (!(fuf_flags&FUF_JUST_DIRS && !(stat.attr & RS_ATTR_DIR)) &&
          !(fuf_flags&FUF_JUST_FILES && stat.attr & RS_ATTR_DIR)) {
        HostFsDirEntryInit(_res, &stat);
        res++;
      }
    }

    Free(abs_path);

    //if (unlock)
    //  DrvUnlock(dv);
  }
  return res;
}

U8 *HostFsFileRead(CDrv *dv,U8 *cur_dir,U8 *filename,I64 *_size,I64 *_attr)
{
  U8 *buf=NULL, *path=NULL;

  DrvChk(dv);
  *_size=0;
  *_attr=0;
  if (dv->fs_type!=FSt_HOSTFS)
    PrintErr("Not HostFS Drv\n");
  else {
    path = MAlloc(StrLen(cur_dir) + 1 + StrLen(filename) + 1);
    path[0] = 0;
    StrCpy(path, cur_dir);
    CatPrint(path,"/");
    CatPrint(path,filename);
    // DrvLock(dv);

    CHostFsStat stat;

    if (VSysCall(VSYSCALL_STAT, path, &stat, dv->prt_num, 0) == 0) {
      buf = MAlloc(stat.size+1);
      *_size = VSysCall(VSYSCALL_FREAD, path, buf, stat.size, dv->prt_num);
      *_attr=FileAttr(stat.name,stat.attr);
      buf[*_size] = 0;
    }

    // DrvUnlock(dv);
    Free(path);
  }
  return buf;
}

Bool HostFsCd(U8 *name,I64 cur_dir_clus)
{
  CDirEntry de;
  if (Fs->cur_dv->fs_type!=FSt_HOSTFS)
    PrintErr("Not HostFS Drv\n");
  else if (HostFsFileFind(Fs->cur_dv,cur_dir_clus,name,&de,FUF_JUST_DIRS))
    return TRUE;
  else
    PrintErr("File not found: \"%s\".\n",name);
  return FALSE;
}

I64 HostFsFilesDel(CDrv *dv,U8 *cur_dir,U8 *files_find_mask,I64 fuf_flags,
                     Bool del_dir,Bool print_msg)
{
  no_warn cur_dir; // TODO: shouldn't we be using this?

  I64 res=0,ch, dirp;

  dirp = VSysCall(VSYSCALL_OPENDIR, Fs->cur_dir, dv->prt_num, 0, 0);

  if (!dirp)
    return NULL;

  CHostFsStat st;

  while (VSysCall(VSYSCALL_READDIR, dirp, &st, dv->prt_num, 0) == 0) {
    ch=*st.name;

    if (ch!='.' && (del_dir || !(st.attr & RS_ATTR_DIR)) &&
          FilesFindMatch(st.name,files_find_mask,fuf_flags)) {
      if (!(st.attr & RS_ATTR_DIR)) res++;
      if (print_msg)
        "Del %s\n",st.name;
      VSysCall(VSYSCALL_UNLINK, st.abs_path, dv->prt_num, 0, 0);
    }
  }
  VSysCall(VSYSCALL_CLOSEDIR, dirp, dv->prt_num, 0, 0);
  return res;
}

I64 HostFsFileWrite(CDrv *dv,U8 *cur_dir,U8 *name,U8 *buf,I64 size,
        CDate cdt,I64 attr)
{
  no_warn cdt;    // TODO?
  no_warn attr;   // TODO?

  CDirEntry de;
  I64 c=0;

  MemSet(&de,0,sizeof(CDirEntry));

  if (size<0) size=0;
  if (dv->fs_type!=FSt_HOSTFS)
    PrintErr("Not HostFS Drv\n");
  else if (!CFileNameTo(de.name,name))
    PrintErr("Invalid FileName: \"%s\".\n",name);
  else {
    U8* path = MAlloc(StrLen(cur_dir) + 1 + StrLen(de.name) + 1);
    path[0] = 0;
    StrCpy(path, cur_dir);
    CatPrint(path,"/");
    CatPrint(path,de.name);

    c = VSysCall(VSYSCALL_FWRITE, path, buf, size, dv->prt_num);

    Free(path);
  }
  return c;
}

CDirEntry *HostFsFilesFind(U8 *files_find_mask,I64 fuf_flags,
        CDirEntry *parent=NULL)
{
  CDrv *dv=Fs->cur_dv;
  CDirEntry *res=NULL,*tmpde;
  I64 dirp;
  if (fuf_flags&~FUG_FILES_FIND)
    throw('FUF');

  dirp = VSysCall(VSYSCALL_OPENDIR, Fs->cur_dir, dv->prt_num, 0, 0);

  if (!dirp)
    return NULL;

  CHostFsStat st;

  while (VSysCall(VSYSCALL_READDIR, dirp, &st, dv->prt_num, 0) == 0) {
    // alloc a CDirEntry and init it from the stat
    tmpde = CAlloc(sizeof(CDirEntry));
    HostFsDirEntryInit(tmpde, &st);

    // link to parent if needed
    tmpde->parent=parent;

    if (Bt(&fuf_flags,FUf_RECURSE) && tmpde->attr&RS_ATTR_DIR &&
          *tmpde->name!='.') {
      // if child directory, recurse into it
      tmpde->next=res;
      res=tmpde;
      tmpde->full_name=DirNameAbs(tmpde->name);

      if (Cd(tmpde->name)) {
        tmpde->sub=HostFsFilesFind(files_find_mask,fuf_flags,tmpde);
        Cd("..");
      }
    } else {
      // not recursing...
      tmpde->full_name=FileNameAbs(tmpde->name);
      if ((tmpde->attr&RS_ATTR_DIR ||
            !Bt(&fuf_flags,FUf_JUST_DIRS)) &&
            !(Bt(&fuf_flags,FUf_RECURSE) && *tmpde->name=='.' &&
            tmpde->attr&RS_ATTR_DIR) &&
            FilesFindMatch(tmpde->full_name,files_find_mask,fuf_flags)) {
        tmpde->next=res;
        res=tmpde;
      } else
        DirEntryDel(tmpde);
    }
  }

  VSysCall(VSYSCALL_CLOSEDIR, dirp, dv->prt_num, 0, 0);
  return res;
}

Bool HostFsMkDir(CDrv *dv,U8 *cur_dir,U8 *name,I64 entry_cnt)
{
  no_warn entry_cnt;

  U8* abs_path = HostFsMakeAbsPath(cur_dir, name);
  Bool ok = VSysCall(VSYSCALL_MKDIR, abs_path, dv->prt_num, 0, 0) == 0;
  Free(abs_path);
  return ok;
}
