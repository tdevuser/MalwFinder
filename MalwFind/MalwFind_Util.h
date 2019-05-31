#ifndef _MALWFIND_UTIL_H_
#define _MALWFIND_UTIL_H_

#include  <ntifs.h>

#ifdef __cplusplus
extern "C"
{
#endif



WCHAR ConvertUpper(WCHAR wChar);

BOOLEAN  
Search_WildcardToken(const WCHAR* pwzBuffer, const WCHAR* pwzWildcardToken );


// /********************************************************************************************************************************/
//  MANAGER_CONFIG 
// /********************************************************************************************************************************/

// C:\Windows 구하기 -- SystemRoot Get
PWCHAR Config_GetSystemRoot(void);

// SetupDir Get
PWCHAR Config_GetSetupDir(void);


// pLogDiskDeviceObject // pLowerDeviceObject
NTSTATUS Config_SetDeviceObject( IN PFLT_DRV_CONFIG pConfig );

NTSTATUS GetSymbolicLink( IN  PUNICODE_STRING  SymbolicLinkName,
			              OUT PUNICODE_STRING  OutSymbolicLink,
			              OUT PHANDLE   pOutLinkHandle  );

NTSTATUS ExtractDriveString_Dir3( IN OUT PUNICODE_STRING pSource );

NTSTATUS ExtractDriveString_Dir4( IN OUT PUNICODE_STRING pSource );


NTSTATUS GetSystemRootPath( IN OUT PWCHAR pwzSystemRootPath, IN ULONG ulMaxSystemRootPath );




// /********************************************************************************************************************************/
//  MANAGER_CONFIG 
// /********************************************************************************************************************************/

// Rename DestPath
ULONG GetFileRenameDestPath( IN PIO_STACK_LOCATION pIrpStack, 
	                         IN PFILE_RENAME_INFORMATION  pRenameInfo, 
							 IN OUT PNAME_BUFFER pFileName );
// FileObject
int GetFileFullPath( IN PDEVICE_OBJECT pDeviceObject, IN PFILE_OBJECT pFileObject, IN PFLT_EXTENSION pDevExt, IN OUT PNAME_BUFFER pFileName );

// IO_STATUS_BLOCK, 
NTSTATUS GetFileFullPathEx( IN PIO_STATUS_BLOCK, IN PFLT_EXTENSION pDevExt, IN OUT PNAME_BUFFER pFileName );

NTSTATUS GetDosNameToLinkTarget( IN PWCHAR pzSrcNameStr, IN OUT PWCHAR pzOutTarget, ULONG ulMaxTarget );

NTSTATUS GetDosNameToLinkTarget_Ex( IN PWCHAR pzSrcNameStr, IN OUT PWCHAR pzOutTarget, ULONG ulMaxTarget );

// Process Name
BOOLEAN GetProcessName( IN PEPROCESS pProcess, IN OUT WCHAR*  pProcessName, IN ULONG ulProcessLen );

ULONG GetProcessFullNameEx( IN HANDLE hPid, IN OUT PNAME_BUFFER pRefProc );



// Written by taehwauser \\Driver\\Disk
// FIXED / Removable / Cdrom
BOOLEAN ChkMnt_Device( IN PWCHAR pDriverName, IN PWCHAR pDosName );

BOOLEAN IS_DeviceStack_Mounted( IN PDEVICE_OBJECT pDevObj, IN OUT PDEVICE_OBJECT* ppDeviceObject );

// Floopy Mounted DeviceCheck
BOOLEAN FloppyCheckMounted(IN PWCHAR pDosName );

// Driver 이름으로 Object \찾기
PDRIVER_OBJECT FindDriverObject( PUNICODE_STRING pUnicodeName );

// 동일한 디바이스스택 찾기
BOOLEAN  
FsdEqualDeviceStack( IN PDEVICE_OBJECT pBaseDeviceObject, IN PDEVICE_OBJECT pFindDeviceObject );

// NHCAFLT Device Object 가 어태치 되어있는지 검사 하는 함수
// pVolDevObj : Volume Device Object
// TRUE  : 어태치 되어있음
// FALSE : 어태치 되어있지 않음 
BOOLEAN  
Attached_MalwFind_DeviceObject( IN PDEVICE_OBJECT pVolumeDeviceObject );

// IRP_MJ_FILE_SYSTEM_CONTROL : IRP_MN_MOUNT_VOLUME 으로 마운트된 Volume 의 드라이브 명을 구한다.
NTSTATUS  
GetVolumeDosName( IN PFLT_EXTENSION pDevExt );






#if (NTDDI_VERSION < NTDDI_WINXP)

typedef struct _EXTENDED_DEVOBJ_EXTENSION
{
    CSHORT  Type;
    USHORT  Size;
    PDEVICE_OBJECT DeviceObject;
    ULONG   PowerFlags;
    struct DEVICE_OBJECT_POWER_EXTENSION*  Dope;
    ULONG   ExtensionFlags;
    struct _DEVICE_NODE* DeviceNode;
    PDEVICE_OBJECT  AttachedTo;
    LONG    StartIoCount;
    LONG    StartIoKey;
    ULONG   StartIoFlags;
    struct _VPB* Vpb;
} EXTENDED_DEVOBJ_EXTENSION, *PEXTENDED_DEVOBJ_EXTENSION;


#define IoGetDevObjExtension(DeviceObject) ((PEXTENDED_DEVOBJ_EXTENSION)(DeviceObject->DeviceObjectExtension))   


NTSTATUS
NTAPI
IoGetDiskDeviceObject_2K( IN PDEVICE_OBJECT pFileSystemDeviceObject, OUT PDEVICE_OBJECT* ppDiskDeviceObject );

#endif








#ifdef __cplusplus
}
#endif

#endif
