#ifndef UPDATER_RECOVERY_START
#define UPDATER_RECOVERY_START

typedef		unsigned char		uint8;
typedef		signed char			int8;
typedef		unsigned short	    uint16;
typedef		signed short	    int16;
typedef		unsigned int		uint32;
typedef		signed int			int32;
typedef		unsigned long long	uint64;
typedef		signed long long	int64;
typedef 	unsigned char		BYTE;

#define MODE_UPDATER	0xF000
#define MODE_GENERATE	0x0000

#define RK_PARTITION_TAG (0x50464B52)

typedef enum {
    PART_VENDOR = 1 << 0,
    PART_IDBLOCK = 1 << 1,
    PART_KERNEL = 1 << 2,
    PART_BOOT = 1 << 3,
    PART_USER = 1 << 31
} ENUM_PARTITION_TYPE;

typedef struct {
    uint16	year;
    uint8	month;
    uint8	day;
    uint8	hour;
    uint8	min;
    uint8	sec;
    uint8	reserve;
} STRUCT_DATETIME, *PSTRUCT_DATETIME;

typedef struct {
    uint32	uiFwTag;            //"RKFP"
    STRUCT_DATETIME	dtReleaseDataTime;
    uint32	uiFwVer;
    uint32	uiSize;             //size of sturct,unit of uint8
    uint32	uiPartEntryOffset;  //unit of sector
    uint32	uiBackupPartEntryOffset;
    uint32	uiPartEntrySize;    //unit of uint8
    uint32	uiPartEntryCount;
    uint32	uiFwSize;           //unit of uint8
    uint8	reserved[464];
    uint32	uiPartEntryCrc;
    uint32	uiHeaderCrc;
} STRUCT_FW_HEADER, *PSTRUCT_FW_HEADER;

typedef struct {
    uint8	szName[32];
    ENUM_PARTITION_TYPE emPartType;
    uint32	uiPartOffset;   //unit of sector
    uint32	uiPartSize;     //unit of sector
    uint32	uiDataLength;   //unit of uint8
    uint32	uiPartProperty;
    uint8	reserved[76];
} STRUCT_PART_ENTRY, *PSTRUCT_PART_ENTRY;

typedef struct {
    STRUCT_FW_HEADER hdr;       //0.5KB
    STRUCT_PART_ENTRY part[12]; //1.5KB
} STRUCT_PART_INFO, *PSTRUCT_PART_INFO;


typedef struct {
    //release date
    unsigned int update_version;

    //firmware.img path.
    unsigned char update_path[200];

    /* update mode
     * 	 0x0000 -> generate mode.
     *	 0xF000 -> updater mode.
     */
    unsigned short update_mode;
} UpdaterInfo;

#define VERDOR_DEVICE "/dev/vendor_storage"

#define VENDOR_REQ_TAG	0x56524551
#define VENDOR_READ_IO	_IOW('v', 0x01, unsigned int)
#define VENDOR_WRITE_IO	_IOW('v', 0x02, unsigned int)

#define VENDOR_UPDATER_ID	14

#define VENDOR_DATA_SIZE (3 * 1024)

typedef struct _RK_VERDOR_REQ {
    uint32 tag;
    uint16 id;
    uint16 len;
    uint8  data[VENDOR_DATA_SIZE];
} RK_VERDOR_REQ;

int updater_recovery_start(char* path);

#endif
