
#ifndef _MALWFIND_DATA_TYPES_H_
#define _MALWFIND_DATA_TYPES_H_

#include <ntifs.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <ntstrsafe.h>
#include "..\Common\MalwFind_Comm.h"

#pragma warning(disable:4214)
#pragma warning(disable:4995)
#pragma warning(disable:4996)

#ifdef __cplusplus
extern "C"
{
#endif

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
// #define DWORD	unsigned long
#define unsigned long   DWORD	
#endif

#ifndef BYTE
#define BYTE    unsigned char
#endif 

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



//**************************************************************************************************************
//  Define Flag 
//**************************************************************************************************************

#ifndef FlagOn
#define FlagOn(_F,_SF)        ((_F) & (_SF))
#endif

#ifndef BooleanFlagOn
#define BooleanFlagOn(F,SF)   ((BOOLEAN)(((F) & (SF)) != 0))
#endif

#ifndef SetFlag
#define SetFlag(_F,_SF)       ((_F) |= (_SF))
#endif

#ifndef ClearFlag
#define ClearFlag(_F,_SF)     ((_F) &= ~(_SF))
#endif


#ifndef Min
#define Min(a,b) (((a) < (b)) ? (a) : (b))
#endif


#ifndef Max
#define Max(a,b) (((a) > (b)) ? (a) : (b))
#endif


	

	

typedef struct _PO_DEVICE_NOTIFY
{
	LIST_ENTRY      Link;
	PDEVICE_OBJECT  TargetDevice;
	UCHAR           WakeNeeded;
	UCHAR           OrderLevel;
	PDEVICE_OBJECT  DeviceObject;
	PVOID           Node;
	USHORT*         DeviceName;
	USHORT*         DriverName;
	UINT            ChildCount;
	UINT            ActiveChild;
} PO_DEVICE_NOTIFY, *PPO_DEVICE_NOTIFY;


typedef enum _PNP_DEVNODE_STATE
{
	DeviceNodeUnspecified = 768,
	DeviceNodeUninitialized = 769,
	DeviceNodeInitialized = 770,
	DeviceNodeDriversAdded = 771,
	DeviceNodeResourcesAssigned = 772,
	DeviceNodeStartPending = 773,
	DeviceNodeStartCompletion = 774,
	DeviceNodeStartPostWork = 775,
	DeviceNodeStarted = 776,
	DeviceNodeQueryStopped = 777,
	DeviceNodeStopped = 778,
	DeviceNodeRestartCompletion = 779,
	DeviceNodeEnumeratePending = 780,
	DeviceNodeEnumerateCompletion = 781,
	DeviceNodeAwaitingQueuedDeletion = 782,
	DeviceNodeAwaitingQueuedRemoval = 783,
	DeviceNodeQueryRemoved = 784,
	DeviceNodeRemovePendingCloses = 785,
	DeviceNodeRemoved = 786,
	DeviceNodeDeletePendingCloses = 787,
	DeviceNodeDeleted = 788

} PNP_DEVNODE_STATE, *PPNP_DEVNODE_STATE;


typedef struct _DEVICE_NODE
{
	struct _DEVICE_NODE*  Sibling;
	struct _DEVICE_NODE*  Child;
	struct _DEVICE_NODE*  Parent;
	struct _DEVICE_NODE*  LastChild;

	UINT                  Level;
	PPO_DEVICE_NOTIFY     Notify;

	PNP_DEVNODE_STATE    State;
	PNP_DEVNODE_STATE    PreviousState;
	PNP_DEVNODE_STATE    StateHistory[20];

	UINT				  StateHistoryEntry;
	int				      CompletionStatus;
	PIRP                  PendingIrp;

	UINT                  Flags;
	UINT                  UserFlags;
	UINT                  Problem;

	PDEVICE_OBJECT        PhysicalDeviceObject;
	PCM_RESOURCE_LIST     ResourceList;
	PCM_RESOURCE_LIST     ResourceListTranslated;
	
	UNICODE_STRING        InstancePath;
	UNICODE_STRING        ServiceName;
	PDEVICE_OBJECT        DuplicatePDO;

	PVOID                 ResourceRequirements;
	
	INTERFACE_TYPE        InterfaceType;
	UINT                  BusNumber;
	INTERFACE_TYPE        ChildInterfaceType;
	UINT                  ChildBusNumber;
	USHORT                ChildBusTypeIndex;
	UCHAR                 RemovalPolicy; 
	UCHAR                 HardwareRemovalPolicy;
	LIST_ENTRY            TargetDeviceNotify;
	LIST_ENTRY            DeviceArbiterList;
	LIST_ENTRY            DeviceTranslatorList;
	USHORT                NoTranslatorMask;
	USHORT                QueryTranslatorMask;
	USHORT                NoArbiterMask;
	USHORT                QueryArbiterMask;
	UINT				  OverUsed1;
	UINT				  OverUsed2;
	PCM_RESOURCE_LIST     BootResources;
	UINT                  CapabilityFlags;
	UCHAR                 DocInfo[16];
	UINT                  DisableableDepends;
	LIST_ENTRY            PendedSetInterfaceState;
	LIST_ENTRY            LegacyBusListEntry;
	UINT                  DriverUnloadRetryCount;
} DEVICE_NODE, *PDEVICE_NODE;

typedef struct  _DEVOBJ_EXT 
{
	CSHORT          Type;
    USHORT          Size;
    PDEVICE_OBJECT  DeviceObject;               // owning device object
	UINT            PowerFlags;
	//PDEVICE_OBJECT_POWER_EXTENSION  Dope;
	PVOID           Dope;
	UINT            ExtensionFlags;	
	PVOID           DeviceNode;
	PDEVICE_OBJECT  AttachedTo;
	int             StartIoCount;
	int             StartIoKey;
	UINT            StartIoFlags;
	PVPB            Vpb;

} DEVOBJ_EXT, *PDEVOBJ_EXT;




NTSYSAPI 
NTSTATUS 
NTAPI
ZwQueryInformationProcess (
  IN HANDLE           ProcessHandle,
  IN PROCESSINFOCLASS ProcessInformationClass,
  OUT PVOID           ProcessInformation,
  IN ULONG            ProcessInformationLength,
  OUT PULONG          ReturnLength OPTIONAL
);



//  Undocumented Native Api--Written by taehwauser  
NTKERNELAPI
NTSTATUS
NTAPI
ObOpenObjectByName( IN POBJECT_ATTRIBUTES ObjectAttributes,
				    IN POBJECT_TYPE ObjectType,
					IN KPROCESSOR_MODE AccessMode,
					IN PACCESS_STATE PassedAccessState,
					IN ACCESS_MASK DesiredAccess,
					IN OUT PVOID ParseContext,
					OUT PHANDLE Handle    );

/*******************************************************************************/
// dt _EPROCESS::ImageName  // Process Name Offset 	
#define  PNAME_NT4              0x1DC
/*******************************************************************************/
#define  PNAME_W2K              0x1FC
#define  PNAME_WXP              0x174

#define  PNAME_WNET             0x154
#define  PNAME_WNET_SP1	        0x164    
#define  PNAME_WNET_SP2         0x164

#define  PNAME_WNET64           0x270
#define  PNAME_WNET64_SP1       0x268   
#define  PNAME_WNET64_SP2       0x268

#define  PNAME_WLH              0x14C
#define  PNAME_WLH_SP1          0x14C
#define  PNAME_WLH_SP2          0x14C

#define  PNAME_WLH64            0x238
#define  PNAME_WLH64_SP1        0x238
#define  PNAME_WLH64_SP2        0x238

#define  PNAME_WIN7	            0x16c
#define  PNAME_WIN7_64          0x2e0

#define  PNAME_WIN8             0x170
#define  PNAME_WIN8_64          0x438

/*******************************************************************************/
#define  PID_W2K                0x09C
#define  PID_WXP                0x084

#define  PID_WNET               0x084
#define  PID_WNET_SP1           0x094
#define  PID_WNET_SP2           0x094

#define  PID_WNET64             0x0E0
#define  PID_WNET64_SP1         0x0D8    
#define  PID_WNET64_SP2         0x0D8    

#define  PID_WLH                0x09C
#define  PID_WLH_SP1            0x09C
#define  PID_WLH_SP2            0x09C

#define  PID_WLH64              0x0E0 
#define  PID_WLH64_SP1          0x0E0 
#define  PID_WLH64_SP2          0x0E0 

#define  PID_WIN7               0x0b4 
#define  PID_WIN7_64            0x180 

#define  PID_WIN8               0x0b4
#define  PID_WIN8_64            0x2e0


/*******************************************************************************/
// dt _EPROCESS  ActiveProcessLinks
#define  PROC_LINKS_W2K         0x0A0
#define  PROC_LINKS_WXP         0x088

#define  PROC_LINKS_WNET        0x088
#define  PROC_LINKS_WNET_SP1    0x098
#define  PROC_LINKS_WNET_SP2    0x098

#define  PROC_LINKS_WNET64      0x0E8
#define  PROC_LINKS_WNET64_SP1  0x0E0
#define  PROC_LINKS_WNET64_SP2  0x0E0

#define  PROC_LINKS_WLH         0x0A0
#define  PROC_LINKS_WLH_SP1     0x0A0
#define  PROC_LINKS_WLH_SP2     0x0A0

#define  PROC_LINKS_WLH64       0x0E8
#define  PROC_LINKS_WLH64_SP1   0x0E8
#define  PROC_LINKS_WLH64_SP2   0x0E8

#define  PROC_LINKS_WIN7        0x0b8 
#define  PROC_LINKS_WIN7_64     0x188

#define  PROC_LINKS_WIN8        0x0b8
#define  PROC_LINKS_WIN8_64     0x2e8

/*******************************************************************************/


/*******************************************************************************/
// dt _EPROCESS::_PEB // Process PEB Offset 
#define  PEB_NT4                0x18C
/*******************************************************************************/
#define  PEB_W2K                0x1B0
#define  PEB_WXP                0x1B0

#define  PEB_WNET               0x190
#define  PEB_WNET_SP1           0x1A0    
#define  PEB_WNET_SP2           0x1A0

#define  PEB_WNET64             0x2C8
#define  PEB_WNET64_SP1         0x2C0    
#define  PEB_WNET64_SP2         0x2C0

#define  PEB_WLH     		    0x188
#define  PEB_WLH_SP1   		    0x188
#define  PEB_WLH_SP2   		    0x188

#define  PEB_WLH64              0x290
#define  PEB_WLH64_SP1          0x290
#define  PEB_WLH64_SP2          0x290

#define  PEB_WIN7     		    0x1A8
#define  PEB_WIN7_64            0x338 
 
#define  PEB_WIN8               0x140
#define  PEB_WIN8_64            0x3e8


/*******************************************************************************/

#define OS_VER_W2K   0x0500
#define OS_VER_WXP   0x0501
#define OS_VER_WNET  0x0502
#define OS_VER_WLH   0x0600
#define OS_VER_WIN7  0x0601
#define OS_VER_WIN8  0x0602


/*******************************************************************************/
// NonPagedPool
/*******************************************************************************/
#define ALLOCATE_N_POOL_PROC( ProcPath ) \
		 ProcPath.pBuffer = (PWCHAR)ExAllocateFromNPagedLookasideList( &g_MalwFind.ProcNPagedList )

	
#define FREE_N_POOL_PROC( ProcPath ) \
		 ExFreeToNPagedLookasideList( &g_MalwFind.ProcNPagedList, ProcPath.pBuffer ); \
		 ProcPath.pBuffer   = NULL; \
		 ProcPath.pFilePos  = NULL; \
		 ProcPath.ulLength  = 0; \
		 ProcPath.ulMaxLength = 0
	
/************************************************************************************************************/
// Pool Init
/************************************************************************************************************/
#define SET_POOL_ZERO_PROC( ProcPath )  \
	ProcPath.pFilePos    = NULL; \
	ProcPath.ulFile      = MFOBJ_NONE; \
	ProcPath.ulMaxLength = MAX_PROCESS_LEN; \
	ProcPath.ulLength    = 0; \
	RtlZeroMemory( ProcPath.pBuffer, (MAX_PROCESS_LEN << 1) )
/************************************************************************************************************/




/*******************************************************************************/
// NonPagedPool
/*******************************************************************************/
#define ALLOCATE_N_POOL( FullPath ) \
		 FullPath.pBuffer = (PWCHAR)ExAllocateFromNPagedLookasideList( &g_MalwFind.FsdNPagedList )

	
#define FREE_N_POOL( FullPath ) \
		 ExFreeToNPagedLookasideList( &g_MalwFind.FsdNPagedList, FullPath.pBuffer ); \
		 FullPath.pBuffer   = NULL; \
		 FullPath.pFilePos  = NULL; \
		 FullPath.ulLength  = 0; \
		 FullPath.ulMaxLength = 0
	
/************************************************************************************************************/
// Pool Init
/************************************************************************************************************/
#define SET_POOL_ZERO( FullPath )  \
	FullPath.pFilePos    = NULL; \
	FullPath.ulFile      = MFOBJ_NONE; \
	FullPath.ulMaxLength = MAX_POOL_LEN >> 1 ; \
	FullPath.ulLength    = 0; \
	RtlZeroMemory( FullPath.pBuffer, MAX_POOL_LEN )
/************************************************************************************************************/

#define READ_FLAG  ( FILE_READ_DATA | \
				     FILE_READ_ATTRIBUTES | \
					 FILE_READ_EA | \
					 GENERIC_READ | \
					 FILE_EXECUTE | \
					 GENERIC_ALL  )

#define WRITE_FLAG ( FILE_WRITE_DATA | \
				     FILE_WRITE_ATTRIBUTES | \
					 FILE_WRITE_EA | \
					 FILE_APPEND_DATA |\
					 GENERIC_WRITE | \
					 GENERIC_ALL )    

#define  RtlInitEmptyUnicodeString(_ucStr,_buf,_bufSize) \
			((_ucStr)->Buffer = (_buf), \
			(_ucStr)->Length = 0, \
			(_ucStr)->MaximumLength = (USHORT)(_bufSize))

#define IS_MALFIND_DEVICE_OBJECT( _devObj )  \
       		 (((_devObj) != NULL) &&     \
       		 ((_devObj)->DriverObject == g_MalwFind.pDriverObject) &&  \
       		 ((_devObj)->DeviceExtension != NULL))


#define IS_NETWORK_DEVICE_TYPE( _type ) \
				((_type) == FILE_DEVICE_NETWORK_FILE_SYSTEM) 

#define IS_VOLUME_DEVICE_TYPE( _type )  \
				(((_type) == FILE_DEVICE_DISK_FILE_SYSTEM) || ((_type) == FILE_DEVICE_CD_ROM_FILE_SYSTEM))

#define IS_FLT_DEVICE_TYPE( _type )  \
				(((_type) == FILE_DEVICE_DISK_FILE_SYSTEM) || \
				((_type) == FILE_DEVICE_CD_ROM_FILE_SYSTEM) || ((_type) == FILE_DEVICE_NETWORK_FILE_SYSTEM))  

#define IS_SFOLDER_ACCESS( _Type )  \
				(ExGetPreviousMode() == KernelMode) || IS_NETWORK_DEVICE_TYPE( _Type )


//**************************************************************************************************************
//  String Constant
//**************************************************************************************************************
#define  FSD_MAX      		   23
#define  FSD_FAT      		   L"\\Fat"
#define  FSD_EXFAT             L"\\exfat"
#define  FSD_FASTFAT  		   L"\\Fastfat"
#define  FSD_NTFS     		   L"\\Ntfs"
#define  FSD_CDFS     		   L"\\Cdfs"
#define  FSD_UDFS     		   L"\\Udfs"
#define  FSD_FAT_CDROM         L"\\FatCdRom"
#define  FSD_FAT_DISK          L"\\FatDisk"
#define  FSD_UDFS_DISK         L"\\UdfsDisk"
#define  FSD_UDFS_CDROM        L"\\UdfsCdRom"
#define  FSD_LANMAN_SERVER     L"\\Device\\LanmanServer"
#define  FSD_LANMAN   		   L"\\Device\\LanmanRedirector"
#define  FSD_FSREC_EXFAT       L"\\FileSystem\\ExFatRecognizer"
#define  FSD_FSREC_FAT_CDROM   L"\\FileSystem\\FatCdRomRecognizer"
#define  FSD_FSREC_UDFS_CDROM  L"\\FileSystem\\UdfsCdRomRecognizer" 
#define  FSD_FSREC_FAT_DISK    L"\\FileSystem\\FatDiskRecognizer" 
#define  FSD_FSREC_UDFS_DISK   L"\\FileSystem\\UdfsDiskRecognizer" 
#define  FSD_FSREC_CDFS        L"\\FileSystem\\CdfsRecognizer" 
#define  FSD_FSREC             L"\\FileSystem\\Fs_Rec"
// Windows Vista
#define  FSD_FSREC_EXFAT       L"\\FileSystem\\ExFatRecognizer"
#define  FSD_RDPDR             L"\\Device\\RdpDr"
#define  FSD_HGFS              L"\\Device\\HGFS"
#define  FSD_MUP               L"\\Device\\Mup"

/**********************************************************************************************/

#define  FSD_DRIVER_SRV        L"\\FileSystem\\Srv"
// Vista
#define  FSD_DRIVER_SRV2       L"\\FileSystem\\Srv2"
#define  FSD_DRIVER_SRVNET     L"\\FileSystem\\Srvnet"

// Volume Mounted Devices
#define  FSD_MOUNTED_DEVICES   L"\\REGISTRY\\MACHINE\\SYSTEM\\MountedDevices\\"
#define  FSD_GLOBAL_DOS        L"\\??\\"
#define  FSD_GLOBAL            L"\\GLOBAL??\\" 
#define  FSD_DOSDEVICES        L"\\DosDevices\\"
#define  DRIVER_FLPY           L"\\Driver\\FlpyDisk"  // Floopy--RealDevice
#define  DRIVER_FIXED          L"\\Driver\\FtDisk"    // Fixed/CDRom--RealDevice
#define  DRIVER_DISK           L"\\Driver\\Disk"     //  Removable--RealDevice 
// #define  DRIVER_CDROM          L"\\Driver\\Cdrom"    // CD--RealDevice 
// WLH / WLH64
#define  DRIVER_VOLMGR         L"\\Driver\\volmgr"

#define  MALWFIND_DEV_NAME      L"\\Device\\MalwFind"
#define  MALWFIND_SYM_NAME      L"\\DosDevices\\MalwFind"

#define  FDD_A                 L"\\DosDevices\\A:"
#define  FDD_B                 L"\\DosDevices\\B:"

#define  FSD_WINDOWS_CURRENT_VERSION     L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
#define  FSD_WINDOWS_NT_CURRENT_VERSION  L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"

#define  FSD_IMAGE_PATH             L"ImagePath"

#define  SYS_PROC_NAME    "System"
#define  FSD_SYSTEM_ROOT  L"SystemRoot"
#define  LOG_FILE_KERNEL  L"KernelLog.nhf"
#define  LOG_FILE_USER    L"UserLog.nhf"
#define  LOG_FILE_TEMP    L"TempLog.nhf"
#define  DIR_FLAG         L"\\"

//**************************************************************************************************************/
//  Device Access Control Driver 
//**************************************************************************************************************/

#define  DRIVER_CDROM      L"\\Driver\\Cdrom"    // CD--RealDevice 

// redbook
#define  DRIVER_REDBOOK    L"\\Driver\\redbook"   // redbook  
// CopyToDvd
#define  DRIVER_COPYTODVD  L"\\Driver\\pcouffin"
// PxHelp20  Sonic
#define  DRIVER_PXHELP20   L"\\Driver\\PxHelp20"
// imapi
#define  DRIVER_IMAPI      L"\\Driver\\imapi"   

// imapi
#define  DRIVER_IMAPI_2    L"\\Driver\\imapi2"   

// atapi
#define  DRIVER_ATAPI      L"\\Driver\\atapi"   


#define PROCESS_TERMINATE                  (0x0001)  
#define PROCESS_CREATE_THREAD              (0x0002)  
#define PROCESS_SET_SESSIONID              (0x0004)  
#define PROCESS_VM_OPERATION               (0x0008)  
#define PROCESS_VM_READ                    (0x0010)  
#define PROCESS_VM_WRITE                   (0x0020)  
#define PROCESS_DUP_HANDLE                 (0x0040)  
#define PROCESS_CREATE_PROCESS             (0x0080)  
#define PROCESS_SET_QUOTA                  (0x0100)  
#define PROCESS_SET_INFORMATION            (0x0200)  
#define PROCESS_QUERY_INFORMATION          (0x0400)  
#define PROCESS_SUSPEND_RESUME             (0x0800)  
#define PROCESS_QUERY_LIMITED_INFORMATION  (0x1000)  

#if (NTDDI_VERSION >= NTDDI_LONGHORN)
	#define PROCESS_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xFFFF)
#else
	#define PROCESS_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xFFF)
#endif


//**************************************************************************************************************
//  Defined Structure
//**************************************************************************************************************

typedef struct _DEF_BUFFER
{
	USHORT    usBufferLength;
	USHORT    usBufferMaxLength;
	WCHAR     Buffer[1];
} DEF_BUFFER, *PDEF_BUFFER, **PPDEF_BUFFER;

typedef struct _CONTROL_BUF
{
	PWCHAR   pObjPos;
	USHORT   usFile; // MalwFind_OBJECT_NONE 0: None  MalwFind_OBJECT_FILE 1 : file  MalwFind_OBJECT_DIR  2: dir
	USHORT   usMaxLength; 
	USHORT   usLength;  
	WCHAR    Buffer[1];
} CONTROL_BUF, *PCONTROL_BUF;

// Object NameBuffer
// File Reg Process SFolder .. etc..
typedef struct _NAME_BUFFER
{
	PWCHAR  pFilePos;
	ULONG   ulFile; 
	ULONG   ulMaxLength; 
	ULONG   ulLength;  
	PWCHAR  pBuffer;
} NAME_BUFFER, *PNAME_BUFFER, **PPNAME_BUFFER;

// Mounted DeviceObject 
typedef struct _MOUNTED_DEVICE
{	
	struct _MOUNTED_DEVICE*  pNext;
	PDEVICE_OBJECT       pDeviceObject;	
	USHORT               Flags;    // VPB_MOUNTED
	USHORT               usDosDeiveNameLen;
	WCHAR                DosDeviceName[ MAX_DOSDEVICE_NAME ];
} MOUNTED_DEVICE, *PMOUNTED_DEVICE, **PPMOUNTED_DEVICE;

// IRP_MJ_FILE_SYSTEM_CONTROL Completion Context
typedef struct _FSCTL_CONTEXT
{
	WORK_QUEUE_ITEM  WorkItem;
    PIRP             pIrp;
	PDEVICE_OBJECT   pDeviceObject;
	PDEVICE_OBJECT   pNewDeviceObject; // VPB->RealDevice;
} FSCTL_CONTEXT, *PFSCTL_CONTEXT, **PPFSCTL_CONTEXT;

// _EPROCESS
typedef struct _REF_PROCESS
{
	ULONG  ulImageName;          //  ImageName 까지의 Distance
	ULONG  ulPeb;                //  _PEB Peb 까지의 디스턴스 	
	ULONG  ulActiveProcessLinks;
	ULONG  ulPId;
} REF_PROCESS, *PREF_PROCESS;


typedef struct _CONTROL_HDR
{
	LIST_ENTRY   ListEntry;    // Control Object List
	BYTE         ucType;       // Object Type : File/Registry/Process/SFolder 
	BYTE         ucAcctl;      // Access Privilige: FULL_ACCESS : READ_ONLY : ACCESS_DENIED 
	USHORT       usRef;       // Reference Count: Reserved
} CONTROL_HDR, *PCONTROL_HDR;


// Access Control Object
typedef struct _CONTROL_OBJ
{
	CONTROL_HDR  Hdr;
	CONTROL_BUF  Obj;  // ObjectName
} CONTROL_OBJ, *PCONTROL_OBJ, **PPCONTROL_OBJ;


//PROC_ENTRY 
typedef struct _DRV_P_ENTRY
{
	LIST_ENTRY    ListEntry;
	BOOLEAN       bCreate;
	ULONG         ulProcID;
} DRV_P_ENTRY, *PDRV_P_ENTRY;


typedef struct _ProcessHide
{
	struct _ProcessHide* pNext;
	PEPROCESS  pProcess;
} ProcessHide, *PProcessHide;

typedef struct _ProcessInfo
{
	struct _ProcessInfo* pNext;
	HANDLE  hProcessId;
	WCHAR   wzProcessName[ MAX_PROCESS_LEN ];
} ProcessInfo, *PProcessInfo;





// 공유폴더제어
typedef struct _FLT_DRV_SFOLDER
{ 	
	LIST_ENTRY  ListHead_ExceptDir;  // 공유폴더 예외 폴더
	LIST_ENTRY  ListHead_ExceptProc; // 공유폴더 예외 프로세스	
	FAST_MUTEX  LockSFolder;

	BOOLEAN           bSFolderHook;
	// Srv.sys  SrvNet.sys IRP_MJ_FILE_SYSTEM_CONTROL
	PDRIVER_DISPATCH  pSrvsvc_Dispatch;

/**********************************************************************/	
	// 로그발생할지 여부 
	// // MALWF_CREATE  MALWF_READ  MALWF_WRITE  FLT_EXECUTE  MALWF_RENAME  MALWF_DELETE 
	NODE_POLICY Policy;
/**********************************************************************/

} FLT_DRV_SFOLDER, *PFLT_DRV_SFOLDER;


typedef struct _FILE_EXT
{
	LIST_ENTRY  ListEntry;
	BYTE        ucAcctl;
	USHORT      usFileExtLength;
	USHORT      usFileExtMaxLength;
	WCHAR       FileExt[1];
} FILE_EXT, *PFILE_EXT;


// 확장자제어
typedef struct _FLT_DRV_FILE
{
	// TRUE:  (BlackList) 리스트 차단함 
    // FALSE: (WhiteList) 리스트만 허용함 나머지 차단
	BOOLEAN    bExtDeny;              
	LIST_ENTRY ListHead_FileExt;       // 1. 차단리스트 / 허용리스트
	LIST_ENTRY ListHead_FileExtExcept; // 2. 예외리스트 (2013.07.15 사용안함 )
	
	LIST_ENTRY ListHead_ExceptDir;	   // 3. 예외 폴더
	LIST_ENTRY ListHead_ExceptProc;    // 4. 예외 프로세스

	FAST_MUTEX  LockFileExt; 
	
	// MALWF_CREATE  MALWF_READ  MALWF_WRITE  FLT_EXECUTE  MALWF_RENAME  MALWF_DELETE 
	NODE_POLICY FddPolicy;
	NODE_POLICY UsbPolicy;
	NODE_POLICY FileExtPolicy;
		
} FLT_DRV_FILE, *PFLT_DRV_FILE;

// TRUE:프로세스제어 차단리스트  FALSE: 프로세스제어 차단 예외리스트
	BOOLEAN       bProcDeny;

typedef struct _FLT_DRV_PROCESS
{
	// TRUE:  (BlackList) 리스트 차단함 
    // FALSE: (WhiteList) 리스트만 허용함 나머지 차단
	BOOLEAN       bProcDeny;              
	LIST_ENTRY    ListHead_ProcCtrl;  // 1. 차단리스트 / 허용리스트

	LIST_ENTRY    ListHead_ExceptDir; // 예외폴더
	FAST_MUTEX    LockProc;	
///**********************************************************************/
	NODE_POLICY   Policy;
///**********************************************************************/
	LIST_ENTRY    ListHead_ProcLCEntry;
	PKEVENT       pProcLC_Event;
	HANDLE        hProcLC_EventHandle;	
///**********************************************************************/
} FLT_DRV_PROCESS, *PFLT_DRV_PROCESS;


#define MAX_RECENT   8

typedef struct _FLT_DRV_LOG
{
	LIST_ENTRY  LogListHead;	
		
	FAST_MUTEX  LogLock;

/**************************************************************************/
	ULONG       ulLogStart;  // 0 로그발생안함  // 1 로그발생
/**************************************************************************/
	PKEVENT     pLogCommEvent;  // Agent Communication;
	HANDLE      hLogCommEventHandle;
/**************************************************************************/
	int         nRecentIndex;
	LOG_RECENT  RecentTable[ MAX_RECENT ];

} FLT_DRV_LOG, *PFLT_DRV_LOG;


typedef struct _FLT_DRV_CONFIG 
{
	PDEVICE_OBJECT  pLowerDeviceObject;
	PDEVICE_OBJECT  pLogDiskDeviceObject;
	ULONG           ulSystemRoot_Length;
	ULONG           ulSetupDir_Length;
	ULONG           ulSysProc_Length;

	WCHAR           wzSystemRoot[ MAX_260_PATH ];
	WCHAR           wzSysProc   [ MAX_260_PATH ];	
	WCHAR           wzSetupDir  [ MAX_260_PATH ];
	WCHAR           wzUpperDir  [ MAX_260_PATH ];
	WCHAR           wzHideFolder[ MAX_DIR_NAME ];
	
	// 자체보호 유무 플래그
	BOOLEAN         bGlobalCtrl;
	// 자체제어 예외 프로세스
	LIST_ENTRY      ListHead_ExceptProc;
	FAST_MUTEX      LockConfig;

} FLT_DRV_CONFIG, *PFLT_DRV_CONFIG; 


// /*********************************************************************************************************
// CDCtrl
// /*********************************************************************************************************

typedef struct _DP_FUNC
{
	PDRIVER_DISPATCH  pCreate;
	PDRIVER_DISPATCH  pRead;
	PDRIVER_DISPATCH  pWrite;
	PDRIVER_DISPATCH  pDevControl;
	PDRIVER_DISPATCH  pIntDevControl;
	PDRIVER_DISPATCH  pPnp;
	PDRIVER_UNLOAD    pDriverUnload;
} DP_FUNC, *PDP_FUNC;


typedef struct _DeviceHook
{
	BOOLEAN  bCdom_Hook;
	BOOLEAN  bCdromUF_Hook;
	BOOLEAN  bCdromLF_Hook;

} DeviceHook, *PDeviceHook;

typedef struct _FLT_DRV_CDCTRL
{
	NODE_POLICY CdCtrlPolicy;
	FAST_MUTEX  LockCdromCtrl;
	
	DeviceHook  DriverHook;

	DP_FUNC     DrvCdrom;
	DP_FUNC     DrvCdrom_UF;
	DP_FUNC     DrvCdrom_LF;

} FLT_DRV_CDCTRL, *PFLT_DRV_CDCTRL;


/**************************************************************************************************************/
// Encrypt - Start
/**************************************************************************************************************/

#define FILE_NONE       0x00
#define FILE_ENCRYPTED  0xEC

#define FILE_CREATE_TAIL_BUFFER     0x01
#define FILE_WRITE_TAIL_BUFFER      0x02
#define FILE_CLOSE_TAIL_BUFFER      0x03
#define FILE_CHECK_TAIL_BUFFER      0x04

#define FILE_ENCRYPT_DELETE_VOLUME  0x05
#define FILE_ENCRYPT_DELETE_FILE    0x06


typedef struct _ENCRYPT_CTX
{
/**************************************************************************************************************/
	ULONG     ulRefCnt;         // Reference Count
	ULONG     ulEncFlag;        // 암호화 여부
/**************************************************************************************************************/
	ULONG     ulOrgFileSize;    // 파일원본사이즈
	ULONG     ulEncFileSize;    // 파일 암호화 사이즈
	ULONG     ulEncPaddingSize; // 패딩사이즈
/**************************************************************************************************************/
	// 암호화시 저장 플래그
	BOOLEAN   bDataSofWrite;   // 암호화시 파일 처음   썼는지 여부판단
	BOOLEAN   bDataEofWrite;   // 암호화시 파일 마지막 썼는지 여부판단
/**************************************************************************************************************/
	BOOLEAN   bTailEofWrite;  // 테일쓰여진 여부
	ULONG     ulTailPos;
/**************************************************************************************************************/
} ENCRYPT_CTX, *PENCRYPT_CTX;


typedef struct _HASH_NODE* PHASH_NODE;
typedef struct _HASH_NODE
{
	PHASH_NODE   pNext;
	PVOID        pFcb;	
	ENCRYPT_CTX  FileCtx;
	WCHAR        wBuffer[1];
} HASH_NODE, *PHASH_NODE;


#define NUM_HASH  0x100
#define HASH_OBJECT( _Context ) (((ULONG)_Context)>>5)%NUM_HASH




typedef struct _FLT_DRV_ENCRYPT
{
/**************************************************************************************************************/
	BYTE        ucSeedKey     [ MAX_ENCKEY_LEN +1 ];
	DWORD       dwSeedRoundKey[ MAX_RNDKEY_LEN +1 ];
/**************************************************************************************************************/	
	PHASH_NODE  EncHashTable[ NUM_HASH ];
	ERESOURCE   EncResource;
/**************************************************************************************************************/
	BYTE        ucEncType; // USB볼륨 볼륨암호화, USB볼륨 폴더암호화 
	BYTE        ucAcctl;   // USB암호화 접근제어
/**************************************************************************************************************/
	ULONG       ulVolCount;
	VOL_DATA    EncVolume;
	VOL_DATA    EncPolicy;
	BYTE        ucEncKey   [ MAX_ENCKEY_LEN +1 ];
	WCHAR       wzEncFolder[ MAX_260_PATH   ]; // Usb 암호화 폴더
/**************************************************************************************************************/	
} FLT_DRV_ENCRYPT, *PFLT_DRV_ENCRYPT;


typedef struct _CONTEXT_WRITE
{
	BOOLEAN          bWriteToEof;
	BOOLEAN          bTailEofExist;

	PIRP             pOldIrp;
	
	IO_STATUS_BLOCK  IoStatus;
	KEVENT           Event;
} CONTEXT_WRITE, *PCONTEXT_WRITE;

typedef struct _CONTEXT_READ
{
	BOOLEAN          bWriteToEof;
	BOOLEAN          bTailEofExist;

	PIRP             pOldIrp;

	IO_STATUS_BLOCK  IoStatus;
	KEVENT           Event;
} CONTEXT_READ, *PCONTEXT_READ;


typedef struct _CONTEXT_DIR
{
	KEVENT           EventDir;
	IO_STATUS_BLOCK  IoStatus;
	PNAME_BUFFER     pFullPath;
} CONTEXT_DIR, *PCONTEXT_DIR;


#define TAIL_SIGNATURE_START 'XEZI'
#define TAIL_CERT_KEY        0xADBC0329
#define TAIL_MASK            0x50
#define TAIL_SIGNATURE_END   'JJ'



typedef struct _TAIL_SIGNATURE
{
	// Start-Signature
	ULONG   ulSignatureStart;
	// Encrypt-Key
	BYTE    ucEncKey[ MAX_ENCKEY_LEN ]; 	
	// 테일 인증키(고정)
	ULONG   ulFileCertKey;
	// 파일사이즈
	ULONG   ulFileOrgSize;
	// 패딩사이즈
	USHORT  usPaddingSize;	
	// End-Signature
	USHORT  usSignatureEnd;
	//
} TAIL_SIGNATURE, *PTAIL_SIGNATURE;



typedef struct _FILE_WORKITEM
{
	WORK_QUEUE_ITEM  WorkItem;
	PDEVICE_OBJECT   pDeviceObject;
	ULONG            ulMajorFuction;
	LARGE_INTEGER    ullByteOffset;
	PVOID            pFcb;
	WCHAR		     wzFilePath[ MAX_260_PATH ]; 	
} FILE_WORKITEM, *PFILE_WORKITEM;

/**************************************************************************************************************/
// Encrypt - End
/**************************************************************************************************************/



// /*********************************************************************************************************
//  Device Extension 
// /*********************************************************************************************************


// DeviceExtension
typedef struct _FLT_EXTENSION
{
	PDEVICE_OBJECT  pLowerDeviceObject;
	PDEVICE_OBJECT  pVolumeDeviceObject;
	PDEVICE_OBJECT  pRealDeviceObject;   // VPB-Real
	WCHAR           wcVol;	 // L'A'
	BYTE            ucAcctl;
	BYTE            ucReserved;
	ULONGLONG       ullReserved;
} FLT_EXTENSION, *PFLT_EXTENSION, **PPFLT_EXTENSION;


// /*********************************************************************************************************
// Global Context 
// /*********************************************************************************************************

// Global structure
typedef struct _G_MALW_FIND
{		
	ULONG            ulOSVer;	
	BOOLEAN          bEntryInit;
	PDRIVER_OBJECT   pDriverObject;
	PDEVICE_OBJECT   pDeviceObject;
	PDRIVER_UNLOAD   pDriverUnload;	
	FAST_MUTEX       LockDevice;  // Device Lock
	ERESOURCE        LockResource;  // ERESOURCE  Lock
	REF_PROCESS      RefProc;
	LARGE_INTEGER    lnRegCookie;
	BOOLEAN          bLoadMode;
	PMOUNTED_DEVICE  pMountedDevice;
	/***************************************************************************/
	// Config
	FLT_DRV_CONFIG   DrvConfig;
	/***************************************************************************/
	FLT_DRV_PROCESS  DrvProc;    // 프로세스 제어
	FLT_DRV_FILE     DrvFile;    // 확장자   제어 // USB //  FDD	
	FLT_DRV_SFOLDER  DrvSFolder; // 공유폴더 제어
	FLT_DRV_CDCTRL   DrvCdCrtl;  // CD 제어	
	/***************************************************************************/
	// Log
	FLT_DRV_LOG      DrvLog; 
	/***************************************************************************/
	NPAGED_LOOKASIDE_LIST  FsdNPagedList;	
	NPAGED_LOOKASIDE_LIST  RegNPagedList;
	NPAGED_LOOKASIDE_LIST  ProcNPagedList;

} G_MALW_FIND, *PG_MALW_FIND, **PPG_MALW_FIND;







#ifdef __cplusplus
};
#endif

#endif