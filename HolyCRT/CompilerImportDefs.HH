// CExts.HC
import U8 *Doc2PlainText(CDoc *doc,CDocEntry *doc_e);
import U0 DocBinsValidate(CDoc *doc);
import U0 DocDel(CDoc *doc);
import U0 DocLoad(CDoc *doc,U8 *src2,I64 size);
import CDoc *DocNew(U8 *filename=NULL,CTask *task=NULL);
import CDocEntry *DocPrint(CDoc *doc=NULL,U8 *fmt,...);
import CDocEntry *DocPutLine(CDoc *doc,CDocEntry *doc_e);
import I64 DocSize(CDoc *doc);
import Bool DocWrite(CDoc *doc,Bool prompt=FALSE);
