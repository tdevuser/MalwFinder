#ifndef _MALWFIND_FUNC_H_
#define _MALWFIND_FUNC_H_

#include  <ntifs.h>

#ifdef __cplusplus
extern "C"
{
#endif
/******************************************************************************************************************/

ULONG  
GetAccessMode( ACCESS_MASK  AccessMask );

NTSTATUS CallDriverSkip( IN PFLT_EXTENSION, IN PIRP pIrp );

NTSTATUS CallDriverCopy( IN PFLT_EXTENSION pDevExt, 
						 IN PIRP pIrp, 
						 IN PIO_COMPLETION_ROUTINE pCompletionRoutine, 
						 IN PVOID pContext );

NTSTATUS VALID_CHECK( IN PDEVICE_OBJECT pDeviceObject, 
					  IN PIRP pIrp, 
					  IN PPFLT_EXTENSION ppDevExt, 
					  PIO_STACK_LOCATION*  ppIrpStack );

ULONG  
ISFileWithCreateOption( IN PIO_STACK_LOCATION  pIrpStack );

ULONG   
ISFileWithFileObject( IN PIO_STACK_LOCATION pIrpStack, IN PFLT_EXTENSION pDevExt, IN PFILE_OBJECT pFileObject );

BOOLEAN  
ISQueryDirectory( IN PDEVICE_OBJECT pDeviceObject, IN PFILE_OBJECT pFileObject, PFILE_STANDARD_INFORMATION  pFileStd );

/******************************************************************************************************************/

NTSTATUS  
PostQueryFileComplete( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext);

NTSTATUS  
PreQueryFile( IN PDEVICE_OBJECT  pDeviceObject, 
			  IN PFILE_OBJECT    pFileObject,
			  IN PVOID           FileQueryBuffer,
			  IN ULONG           FileQueryBufferLength, 
			  IN FILE_INFORMATION_CLASS  FileInfoClass );

NTSTATUS  
PostSetFileComplete( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext );

NTSTATUS 
PreSetFile( IN PDEVICE_OBJECT  pDeviceObject, 
		    IN PFILE_OBJECT    pFileObject,			
			IN PVOID           FileSetBuffer,
			IN ULONG           FileSetBufferLength,
			IN FILE_INFORMATION_CLASS  FileInfoClass );

BOOLEAN      
SetFileUpdate_AllocSize( IN PFILE_OBJECT, IN PFSRTL_COMMON_FCB_HEADER pFcb );
BOOLEAN 
SetFileUpdate_AllocSizeEx( IN PDEVICE_OBJECT pDeviceObject, IN PFILE_OBJECT pFileObject,   IN PFSRTL_COMMON_FCB_HEADER pFcb );
BOOLEAN 
SetFileUpdate_EndOfFileSize( IN PDEVICE_OBJECT pDeviceObject, IN PFILE_OBJECT pFileObject, IN LARGE_INTEGER  ullEndOfFile );


NTSTATUS  
File_PostClose( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext );
NTSTATUS  
File_PostCreate( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext );

// 파일확장자제어 사용안함
NTSTATUS  
File_PostCreate_FileExtCtrl( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext );
NTSTATUS  
File_PreCreate( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );

NTSTATUS  
File_PreClose( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );


NTSTATUS  
File_PreRead( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );
NTSTATUS  
File_PreWrite( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );


// IRP_MJ_QUERY_INFORMATION
NTSTATUS  
File_PostQueryInformation( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext );
NTSTATUS  
File_PreQueryInfomation( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );

// IRP_MJ_SET_INFORMATION
NTSTATUS  
File_PreSetInformation ( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );

// IRP_MJ_DEVICE_CONTROL
NTSTATUS  
File_PreDeviceControl( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );

// IRP_MJ_SYSTEM_CONTROL
NTSTATUS  
File_PreSystemControl( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );

// IRP_MJ_FILE_SYSTEM_CONTROL
NTSTATUS  
File_PreFileSystemControl( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );

// IRP_MJ_DIRECTORY_CONTROL
NTSTATUS  File_PreDirControl( IN PDEVICE_OBJECT  pDeviceObject, IN PIRP pIrp );
//
// NTSTATUS  File_PostDirControlComplete( IN PDEVICE_OBJECT, IN PIRP, IN PVOID pContext );								
//
NTSTATUS  File_PostDirControlComplete_WXP( IN PDEVICE_OBJECT, IN PIRP, IN PVOID pContext );								
NTSTATUS  File_PostDirControlComplete_WLH( IN PDEVICE_OBJECT, IN PIRP, IN PVOID pContext );								



// 동적으로 구하기
void   
FltFs_Notification( IN PDEVICE_OBJECT pDeviceObject, IN BOOLEAN bFsActive );

// 파일시스템에서 올리기
NTSTATUS  
FsdAttachToFileSystem( IN PDEVICE_OBJECT pDeviceObject, IN PUNICODE_STRING  pDeviceName );

void     
FsdDetachFromFileSystem( IN PDEVICE_OBJECT pDeviceObject, IN PUNICODE_STRING pDeviceName );

// IoCreateDevice 대체용
NTSTATUS  
FsdCreateDevice( IN PDEVICE_OBJECT pDeviceObject, IN OUT PDEVICE_OBJECT*  ppNewDeviceObject );

// ATTACH DETACH 관련함수
NTSTATUS  
FsdAttachToDeviceStack( IN PDEVICE_OBJECT pDevObjSrc, 
					    IN PDEVICE_OBJECT pDevObjDest, 
						IN OUT PDEVICE_OBJECT* ppLowerDeviceObject );

// 유효성 체크
void FsdCleanupMountDevice( IN PDEVICE_OBJECT pDeviceObject );

// Object 이름구하기
void FsdGetObjectName( IN PVOID pObject, IN OUT PUNICODE_STRING pObjectName );

// 필터하려는 Device 인지 알아오는 함수
BOOLEAN  
TrueFltDevice( IN PDEVICE_OBJECT pDeviceObject, IN PUNICODE_STRING pDeviceName );

// pDeviceObject : FileSystem DeviceOBject
BOOLEAN  
FsdDetachDeleteDevice( IN PDEVICE_OBJECT pDeviceObject );

// Device 의 볼륨명을 구한다.
BOOLEAN  
GetDeviceVolumeDosName( IN PDEVICE_OBJECT pDeviceObject,  
					    IN OUT PWCHAR pBufferDosName, 
						IN ULONG nBufferMaxLen );

// 일시적으로 풀할당하여 Volume 정보를 가지고 있는다.
NTSTATUS  GetRegMountedDevices(IN OUT PPMOUNTED_DEVICE ppMntDevice );

// 수동으로 위의 기능을 한다.
//void SetMountDriveNames();
// 일시적으로 할당된 Volume 정보를 지운다.
VOID  ClearRegMountedDevices();


// 완료루틴
NTSTATUS  FsdCtrlCompletion( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext );

// IRP_MN_MOUNT_VOLUME --> Call
NTSTATUS  FsdCtrlMountVolume( IN PDEVICE_OBJECT pDeviceObject, PFLT_EXTENSION pDevExt, IN PIRP pIrp );

// WorkItem --> WorkThread Routine Win2000 만 호출
void FsdCtrlMountVolumeRoutine( IN PVOID pContext );

// 실제로 어태치 하는 함수
NTSTATUS 
FsdCtrlMountVolumeComplete( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PDEVICE_OBJECT pNewDeviceObject );

/****************************************************************************************************************************/
/****************************************************************************************************************************/

// 캐싱된 데이터를 Flush 한다.
BOOLEAN  
File_FlushCacheRequest( IN PFILE_OBJECT pFileObject, IN PWCHAR pwzFullPath );
// 캐싱된 데이터를 Flush 한다.
BOOLEAN  
File_FlushCacheRequest_NoWait( IN PFILE_OBJECT pFileObject );

void Parsing_ReparsePoint_DataBuffer( PREPARSE_DATA_BUFFER pDataBuffer, ULONG ulBufferLength );
void Parsing_ReparsePoint_GuidDataBuffer( PREPARSE_GUID_DATA_BUFFER pDataBuffer, ULONG ulBufferLength );

BOOLEAN  
ISNtfsMetaFileName( IN PNAME_BUFFER pFullPath );


// BOOLEAN  
// FileEncrypt_RequestWorkItem( IN ULONG ulMajorFunc, IN PDEVICE_OBJECT pDeviceObject, IN PWCHAR pwzFullPath, IN PVOID pFcb );



extern const WCHAR* g_pExpProcName[];
extern const WCHAR* g_pNtfsMetaFileName[];

/****************************************************************************************************************************/


/****************************************************************************************************************************/




#ifdef __cplusplus
}
#endif

#endif