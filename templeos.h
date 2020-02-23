
#include <stdint.h>

//$LK,"CAOTImportExport",A="MN:CAOTImportExport"$ Types. Used in PatchTable.
#define IET_END                 0
//reserved
#define IET_REL_I0              2 //Fictitious
#define IET_IMM_U0              3 //Fictitious
#define IET_REL_I8              4
#define IET_IMM_U8              5
#define IET_REL_I16             6
#define IET_IMM_U16             7
#define IET_REL_I32             8
#define IET_IMM_U32             9
#define IET_REL_I64             10
#define IET_IMM_I64             11
#define IEF_IMM_NOT_REL         1
//reserved
#define IET_REL32_EXPORT        16
#define IET_IMM32_EXPORT        17
#define IET_REL64_EXPORT        18 //Not implemented
#define IET_IMM64_EXPORT        19 //Not implemented
#define IET_ABS_ADDR            20
#define IET_CODE_HEAP           21 //Not really used
#define IET_ZEROED_CODE_HEAP    22 //Not really used
#define IET_DATA_HEAP           23
#define IET_ZEROED_DATA_HEAP    24 //Not really used
#define IET_MAIN                25

struct CBinFile {
    uint16_t    jmp;
    uint8_t     module_align_bits,
                reserved;
    uint32_t    signature;
    int64_t     module_org,
                patch_table_offset,
                file_size;
};

struct CHash
{//See $LK,"Hash",A="HI:Hash"$
    struct CHash	*next;
    char const	*str;
    uint32_t	type,
            use_cnt; // inc'ed every time search found, never dec'ed.
};

typedef struct CHash* CHash10[1<<10];

struct CHashTable10 {
    struct CHashTable10 *next;
    uint64_t mask,locked_flags;
    CHash10 *body;
};

//struct CPatchTableAbsAddr
//{
//    uint8_t	    eit_abs_addr;	//$LK,"IET_ABS_ADDR",A="MN:IET_ABS_ADDR"$
//    uint32_t	abs_addres_cnt;
//    uint8_t	    zero;
//    uint32_t	abs_addres[1];
//} __attribute__((packed));

//Red Sea Attributes
//See $LK,"ST_FILE_ATTRS",A="MN:ST_FILE_ATTRS"$
#define RS_ATTR_READ_ONLY       0x01 //R
#define RS_ATTR_HIDDEN          0x02 //H
#define RS_ATTR_SYSTEM          0x04 //S
#define RS_ATTR_VOL_ID          0x08 //V
#define RS_ATTR_DIR             0x10 //D
#define RS_ATTR_ARCHIVE         0x20 //A
#define RS_ATTR_LONG_NAME       (RS_ATTR_READ_ONLY|RS_ATTR_HIDDEN|\
                                RS_ATTR_SYSTEM|RS_ATTR_VOL_ID)
#define RS_ATTR_LONG_NAME_MASK  (RS_ATTR_LONG_NAME|RS_ATTR_DIR|RS_ATTR_ARCHIVE)
#define RS_ATTR_DELETED         0x100 //X
#define RS_ATTR_RESIDENT        0x200 //T
#define RS_ATTR_COMPRESSED      0x400 //Z
#define RS_ATTR_CONTIGUOUS      0x800 //C
#define RS_ATTR_FIXED           0x1000 //F
