#ifndef _NHCAFLT_COMM_H_
#define _NHCAFLT_COMM_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define SVCNAME_AGENT   L"\\services\\MalwFinder"
#define SVCNAME_DRIVER  L"\\services\\MalwFind"
	
//**************************************************************************************************************
//  Define LCEvent 
//**************************************************************************************************************

#define MALWFIND_PROC_LC_EVENT         L"\\BaseNamedObjects\\MalwFind_Process_Request"
// Log Kernel Event
#define MALWFIND_KERNEL_EVENT          L"MalwFind_LogFileRead_EventObject"
#define MALWFIND_BASE_KERNEL_EVENT     L"\\BaseNamedObjects\\MalwFind_LogFileRead_EventObject"
#define MALWFIND_PROCKILL_KERNEL_EVENT L"\\BaseNamedObjects\\MalwFind_ProcessKill_EventObject"

//**************************************************************************************************************
//  Define DataType
//**************************************************************************************************************

#ifndef NULL
#define NULL  0
#endif

#ifndef TRUE 
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef ULONG 
#define ULONG   unsigned long
#endif 
	
#ifndef DWORD 
#define DWORD	unsigned long
#endif

#ifndef BYTE
#define BYTE    unsigned char
#endif 

typedef BYTE*   PBYTE;
typedef BYTE**  PPBYTE;


#ifndef WORD
#define WORD    unsigned short
#endif

#ifndef PWORD 
#define PWORD   unsigned short*
#endif 

#ifndef PPVOID 
#define PPVOID  void **
#endif 

#ifndef HANDLE 
#define HANDLE  void* 
#endif

#ifndef ULONGLONG
#define ULONGLONG unsigned __int64
#endif


#ifndef UINT
#define UINT  unsigned int
#endif


#define  OBJ_NONE        0x00
#define  OBJ_FILE        0x01
#define  OBJ_DIR         0x02


#define  MAX_LOGFILE_SIZE  (10*1024*1024)


#define METHOD_DEFAULT_IRP   0x00
#define METHOD_ALLOCATE_IRP  0x01

#define  SECTOR_SIZE         512

// PoolTag
#define  MALWFIND_NAME_TAG     'wlaM'
#define  MAX_DEVICE_LEN       80

#define  MAX_POOL_HEADER       8
#define  MAX_POOL_LEN       1024     // Lookaside 

#define  MAX_FILE_LEN        260 
#define  MAX_BUFFER_LEN      256
#define  MAX_PROCESS_LEN     128
#define  MAX_IMAGE_NAME_LEN   16
#define  MAX_FS_VOL           26  
#define  MAX_VOL_NUMS         26

#define  MAX_DOSDEVICE_NAME   32
#define  MAX_VOLUME_NAME       8
#define  MAX_PREFIX_NAME      32
#define  MAX_PATH            260

#define  MAX_DIR_NAME        128

#define  MAX_128_PATH        128
#define  MAX_256_PATH        256
#define  MAX_260_PATH        260
#define  MAX_512_PATH        512
#define  MAX_1024_PATH      1024
#define  MAX_2048_PATH      2048
#define  MAX_4096_PATH      4096
#define  MAX_8192_PATH      8192

//**********************************************************************************************************************************/

#define  MAX_ENCKEY_LEN     16
#define  MAX_RNDKEY_LEN     32

//**********************************************************************************************************************************/

#define FLAG_MAX  2

#define CTRL_FULL_ACCESS	0x01
#define CTRL_ACCESS_DENY	0x02
#define CTRL_READ_ONLY		0x04
#define CTRL_WRITE_ONLY     0x08
#define CTRL_LOG            0x10
#define CTRL_RESERVED       0x20


#define LOG_OFF      0x00
#define LOG_ON       0x01

#define ZERO_VOLUME  0x00
	
#define FLAG_OFF     0x00
#define FLAG_ON      0x01


#define MALWF_ACCESS       0x00000000
#define MALWF_CREATE       (2  <<  1)
#define MALWF_OPEN         (2  <<  2)
#define MALWF_READ         (2  <<  3)
#define MALWF_WRITE        (2  <<  4)
#define MALWF_EXECUTE      (2  <<  5)
#define MALWF_DELETE       (2  <<  6)
#define MALWF_RENAME       (2  <<  7)
#define MALWF_REPLACE      (2  <<  8)
#define MALWF_REPARSE      (2  <<  9) 
#define MALWF_SET_SECURITY (2  << 10) 
#define MALWF_SET_LINK     (2  << 11) 
#define MALWF_HIDE         (2  << 12)
#define MALWF_MAX          0xFFFFFFFF

#define WRITE_ACCESS     (MALWF_CREATE | MALWF_WRITE | MALWF_DELETE | MALWF_RENAME)

// Log 종류의 아이디
// LogID
#define MFLOG_NONE      0x00
#define MFLOG_FDD       0x01 
#define MFLOG_USB       0x02
#define MFLOG_DLP       0x04
#define MFLOG_PROCESS   0x08 

#define MFLOG_CDROM     0x10
#define MFLOG_SFOLDER   0x20


#define MFOBJ_NONE      0x00
#define MFOBJ_FILE      0x01
#define MFOBJ_DIR       0x02
#define MFOBJ_PROCESS   0x04
#define MFOBJ_SFOLDER   0x08
#define MFOBJ_REGISTRY  0x10
#define MFOBJ_ENCRYPT   0x20



#define MFFILE_SUPERSEDE     FILE_SUPERSEDE
#define MFFILE_OPEN          FILE_OPEN
#define MFFILE_CREATE        FILE_CREATE
#define MFFILE_OPEN_IF       FILE_OPEN_IF
#define MFFILE_OVERWRITE     FILE_OVERWRITE
#define MFFILE_OVERWRITE_IF  FILE_OVERWRITE_IF

#define MFFILE_RENAME        0x00000006
#define MFFILE_DELETE        0x00000007
#define MFFILE_LINK          0x00000008
#define MFFILE_MAX           0x00000008


// File Access Control
// Bit Field
#define BIT_FILE_CREATE          0x0    
#define BIT_FILE_READ            0x1
#define BIT_FILE_WRITE           0x2
#define BIT_FILE_DELETE          0x3
#define BIT_FILE_RENAME          0x4
#define BIT_FILE_COPY            0x5
#define BIT_FILE_MOVE            0x6
#define BIT_FILE_MAX             0x7

/************************************************************************************************************************************/
//  IOCTL Code Definition  >>  FILE_DEVICE_NHCAFLT
/************************************************************************************************************************************/

#define FILE_DEVICE_MALWFIND  0x00008200
#define IOCTL_MALWF_CODE(_CODE, _Access)  CTL_CODE( FILE_DEVICE_MALWFIND, _CODE, METHOD_BUFFERED, _Access)

// 드라이버 버전
// 로그 취합시작/정지
// 로그패치
// 자체 폴더제어-자체폴더하이드 
#define IOCTL_MALWF_GET_VERSION		  IOCTL_MALWF_CODE(0x800, FILE_ANY_ACCESS)
#define IOCTL_MALWF_LOG_START         IOCTL_MALWF_CODE(0x801, FILE_ANY_ACCESS)
#define IOCTL_MALWF_GET_LOGFETCH      IOCTL_MALWF_CODE(0x802, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_SETUPDIRCTRL  IOCTL_MALWF_CODE(0x805, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_SETUPDIR      IOCTL_MALWF_CODE(0x806, FILE_ANY_ACCESS)
//FILE 예외폴더 설정
//FILE 예외프로세스 설정
#define IOCTL_MALWF_SET_EXCEPT_FILEDIR   IOCTL_MALWF_CODE(0x811, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_FILEDIR   IOCTL_MALWF_CODE(0x812, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_EXCEPT_FILEPROC  IOCTL_MALWF_CODE(0x813, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_FILEPROC  IOCTL_MALWF_CODE(0x814, FILE_ANY_ACCESS)
//파일이름 제어
//FDD 제어
//이동저장장치 제어
//파일확장자 제어
#define IOCTL_MALWF_SET_FILENAMECTRL  IOCTL_MALWF_CODE(0x817, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_FILENAMECTRL  IOCTL_MALWF_CODE(0x818, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_FDDVOLCTRL    IOCTL_MALWF_CODE(0x821, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_USBVOLCTRL    IOCTL_MALWF_CODE(0x822, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_FILEEXTCTRL   IOCTL_MALWF_CODE(0x823, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_FILEEXTCTRL   IOCTL_MALWF_CODE(0x824, FILE_ANY_ACCESS)

//공유폴더 제어
#define IOCTL_MALWF_SET_SFCTRL        IOCTL_MALWF_CODE(0x825, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_SFCTRL        IOCTL_MALWF_CODE(0x826, FILE_ANY_ACCESS)

//공유폴더 예외폴더 설정
//공유폴더 예외프로세스 설정
#define IOCTL_MALWF_SET_EXCEPT_SFDIR    IOCTL_MALWF_CODE(0x827, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_SFDIR    IOCTL_MALWF_CODE(0x828, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_EXCEPT_SFPROC   IOCTL_MALWF_CODE(0x829, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_SFPROC   IOCTL_MALWF_CODE(0x82A, FILE_ANY_ACCESS)

//프로세스 콜백//프로세스 살고 죽는것 가지고 오기
#define IOCTL_MALWF_GET_PROC_STAT       IOCTL_MALWF_CODE(0x841, FILE_ANY_ACCESS)
//PROCESS 예외폴더 설정
#define IOCTL_MALWF_SET_EXCEPT_PROCDIR   IOCTL_MALWF_CODE(0x842, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_PROCDIR   IOCTL_MALWF_CODE(0x843, FILE_ANY_ACCESS)
//프로세스 제어
#define IOCTL_MALWF_SET_PROC_CTRL   IOCTL_MALWF_CODE(0x844, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_PROC_CTRL   IOCTL_MALWF_CODE(0x845, FILE_ANY_ACCESS)

// 악성코드분석
#define IOCTL_MALWF_POLICY_REG      IOCTL_MALWF_CODE(0x871, FILE_ANY_ACCESS)
#define IOCTL_MALWF_POLICY_FILE     IOCTL_MALWF_CODE(0x872, FILE_ANY_ACCESS)
#define IOCTL_MALWF_POLICY_NETWORK  IOCTL_MALWF_CODE(0x873, FILE_ANY_ACCESS)
#define IOCTL_MALWF_ANALYSIS_START  IOCTL_MALWF_CODE(0x874, FILE_ANY_ACCESS) 


/************************************************************************************************************************************/
//   App Communication DataType 
/************************************************************************************************************************************/

#define MAX_VERSION       32
#define MAX_ENCRYPT_KEY  128

#define MF_SIGNATURE    'wlaM'
#define MF_VERSION	   0x00000001;


typedef struct _DRIVE_SYMBOL
{
	int  Drv_A : 1;
	int  Drv_B : 1;
	int  Drv_C : 1;
	int  Drv_D : 1;
	int  Drv_E : 1;
	int  Drv_F : 1;
	int  Drv_G : 1;
	int  Drv_H : 1;
	int  Drv_I : 1;
	int  Drv_J : 1;
	int  Drv_K : 1;
	int  Drv_L : 1;
	int  Drv_M : 1;
	int  Drv_N : 1;
	int  Drv_O : 1;
	int  Drv_P : 1;
	int  Drv_Q : 1;
	int  Drv_R : 1;
	int  Drv_S : 1;
	int  Drv_T : 1;
	int  Drv_U : 1;
	int  Drv_V : 1;
	int  Drv_W : 1;
	int  Drv_X : 1;
	int  Drv_Y : 1;
	int  Drv_Z : 1;
	int  Drv_Reserved : 6;
} DRIVE_SYMBOL, *PDRIVE_SYMBOL;



typedef union _VOL_DATA
{
	ULONG         ulDrives;
	DRIVE_SYMBOL  DrvSym;
} VOL_DATA, *PVOL_DATA;


typedef struct _NodeProcess
{
	ULONG  hProcID;
	char   czProcName[ MAX_PROCESS_LEN  ];
} NodeProcess, *PNodeProcess;

typedef struct _NODE_POLICY
{	
	BOOLEAN     bLog;     // 로그발생할지 여부 
	ULONG       ulAcctl;   // 제어어 플래그 RWXCDN 
	VOL_DATA    Vol;
} NODE_POLICY, *PNODE_POLICY;

// Header
typedef struct _DRVCMD_Header
{
	ULONG  ulSignature;
	ULONG  ulVersion;
	ULONG  ulTotalSize;
} DRVCMD_Header, *PDRVCMD_Header;

// Version
typedef struct _DRVCMD_Version
{
	DRVCMD_Header Hdr;
} DRVCMD_Version, *PDRVCMD_Version;

// 로그 Fetch 
typedef struct _DRVCMD_LogFetch
{	
	DRVCMD_Header  Hdr;
	ULONG          ulDataLength;
	char           Data[2];
} DRVCMD_LogFetch, *PDRVCMD_LogFetch;

// 자체제어
typedef struct _DRVCMD_SetupDir
{
	DRVCMD_Header  Hdr;
	BOOLEAN        bGlobalCtrl;
	WCHAR          wzSystemDir[ MAX_260_PATH ];
	WCHAR          wzSetupDir [ MAX_260_PATH ];
	WCHAR          wzHideFolder[ MAX_DIR_NAME ];
} DRVCMD_SetupDir, *PDRVCMD_SetupDir;

// FILE 예외폴더 설정/해제
typedef struct _DRVCMD_ExceptFileDir
{
	DRVCMD_Header Hdr;
	WCHAR         wzExceptDir[ MAX_260_PATH ];
} DRVCMD_ExceptFileDir, *PDRVCMD_ExceptFileDir;

// FILE 예외프로세스 설정/해제
typedef struct _DRVCMD_ExceptFileProc
{
	DRVCMD_Header Hdr;
	WCHAR         wzExceptProc[ MAX_260_PATH ];
} DRVCMD_ExceptFileProc, *PDRVCMD_ExceptFileProc;

// FDD 제어
typedef struct _DRVCMD_FddVolCtrl
{
	DRVCMD_Header Hdr;
	NODE_POLICY   FddPolicy; // L"A:\\|B:\\"
} DRVCMD_FddVolCtrl, *PDRVCMD_FddVolCtrl;


// 이동저장장치 제어
typedef struct _DRVCMD_USBVolCtrl
{
	DRVCMD_Header Hdr;
	NODE_POLICY   UsbPolicy; // L"A:\\|B:\\"
} DRVCMD_USBVolCtrl, *PDRVCMD_USBVolCtrl;


#define FEXT_ALLOW_LIST  0x00
#define FEXT_DENY_LIST   0x01

// 파일이름 제어 / 해제 
typedef struct _DRVCMD_FileNameCtrl
{
	DRVCMD_Header  Hdr;
	BOOLEAN        bFileNameDeny; 
	NODE_POLICY    FileNamePolicy;	
	WCHAR          FileName[ MAX_260_PATH ];
} DRVCMD_FileNameCtrl, *PDRVCMD_FileNameCtrl;

// 확장자제어 / 해제
typedef struct _DRVCMD_FileExtCtrl
{
	DRVCMD_Header Hdr;
	// TRUE:확장자제어 차단리스트  FALSE:확장자제어 차단 예외리스트
	BOOLEAN       bFileExtDeny; 
	NODE_POLICY   FileExtPolicy;	
	WCHAR         FileExt[ MAX_260_PATH ];
} DRVCMD_FileExtCtrl, *PDRVCMD_FileExtCtrl;


//Process제어 설정/해제
typedef struct _DRVCMD_ProcCtrl
{
	DRVCMD_Header Hdr;
	// TRUE:프로세스제어 차단리스트  FALSE: 프로세스제어 차단 예외리스트
	BOOLEAN       bProcDeny;
	NODE_POLICY   Policy;
	WCHAR         Proc[ MAX_260_PATH ];
} DRVCMD_ProcCtrl, *PDRVCMD_ProcCtrl;

// 공유폴저제어 / 해제
typedef struct _DRVCMD_SFolderCtrl
{
	DRVCMD_Header Hdr;	
	NODE_POLICY   Policy;
} DRVCMD_SFolderCtrl, *PDRVCMD_SFolderCtrl;

//프로세스 죽고/사는것 가져오기
typedef struct _DRVCMD_ProcStat
{
	DRVCMD_Header Hdr;	
} DRVCMD_ProcStat, *PDRVCMD_ProcStat;

//Process 예외폴더 설정/해제
typedef struct _DRVCMD_ExceptProcDir
{
	DRVCMD_Header Hdr;	
	WCHAR         wzExceptDir[ MAX_260_PATH ];
} DRVCMD_ExceptProcDir, *PDRVCMD_ExceptProcDir;

typedef struct _DRVCMD_LogStart
{
	DRVCMD_Header  Hdr;
	ULONG          ulLogStart;
} DRVCMD_LogStart, *PDRVCMD_LogStart;

// /*********************************************************************************************************
// Log  Associated
// /*********************************************************************************************************



typedef struct _MY_CTIME 
{
    short Year;        // range [1601...]
    short Month;       // range [1..12]
    short Day;         // range [1..31]
    short Hour;        // range [0..23]
    short Minute;      // range [0..59]
    short Second;      // range [0..59]
    short Milliseconds;// range [0..999]
    short Weekday;     // range [0..6] == [Sunday..Saturday]
} MY_CTIME, *PMY_CTIME;


typedef struct _LOG_HDR
{
	ULONG     ulSignature;
	ULONG     ulLogID;
	ULONG     ulAccess;
	ULONG     ulProcessID;
	ULONG     ulTotalSize;
	MY_CTIME  cTime;
} LOG_HDR, *PLOG_HDR;


typedef struct _LOG_RECORD
{
	LIST_ENTRY  ListEntry;
	LOG_HDR     Hdr;	
	char        cProcName[MAX_PROCESS_LEN];
	ULONG       ulFileLength;
	char        cFileName[2];
} LOG_RECORD, *PLOG_RECORD;


typedef struct _LOG_RECENT
{
	ULONG       ulLogID;
	ULONG       ulAccess;
	ULONG       ulProcessID;
	MY_CTIME    cTime;
	char        cFileName[ MAX_260_PATH ];
} LOG_RECENT, *PLOG_RECENT;


#ifdef __cplusplus
}
#endif

#endif


















