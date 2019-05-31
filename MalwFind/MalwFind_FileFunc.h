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

// ����Ȯ�������� ������
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



// �������� ���ϱ�
void   
FltFs_Notification( IN PDEVICE_OBJECT pDeviceObject, IN BOOLEAN bFsActive );

// ���Ͻý��ۿ��� �ø���
NTSTATUS  
FsdAttachToFileSystem( IN PDEVICE_OBJECT pDeviceObject, IN PUNICODE_STRING  pDeviceName );

void     
FsdDetachFromFileSystem( IN PDEVICE_OBJECT pDeviceObject, IN PUNICODE_STRING pDeviceName );

// IoCreateDevice ��ü��
NTSTATUS  
FsdCreateDevice( IN PDEVICE_OBJECT pDeviceObject, IN OUT PDEVICE_OBJECT*  ppNewDeviceObject );

// ATTACH DETACH �����Լ�
NTSTATUS  
FsdAttachToDeviceStack( IN PDEVICE_OBJECT pDevObjSrc, 
					    IN PDEVICE_OBJECT pDevObjDest, 
						IN OUT PDEVICE_OBJECT* ppLowerDeviceObject );

// ��ȿ�� üũ
void FsdCleanupMountDevice( IN PDEVICE_OBJECT pDeviceObject );

// Object �̸����ϱ�
void FsdGetObjectName( IN PVOID pObject, IN OUT PUNICODE_STRING pObjectName );

// �����Ϸ��� Device ���� �˾ƿ��� �Լ�
BOOLEAN  
TrueFltDevice( IN PDEVICE_OBJECT pDeviceObject, IN PUNICODE_STRING pDeviceName );

// pDeviceObject : FileSystem DeviceOBject
BOOLEAN  
FsdDetachDeleteDevice( IN PDEVICE_OBJECT pDeviceObject );

// Device �� �������� ���Ѵ�.
BOOLEAN  
GetDeviceVolumeDosName( IN PDEVICE_OBJECT pDeviceObject,  
					    IN OUT PWCHAR pBufferDosName, 
						IN ULONG nBufferMaxLen );

// �Ͻ������� Ǯ�Ҵ��Ͽ� Volume ������ ������ �ִ´�.
NTSTATUS  GetRegMountedDevices(IN OUT PPMOUNTED_DEVICE ppMntDevice );

// �������� ���� ����� �Ѵ�.
//void SetMountDriveNames();
// �Ͻ������� �Ҵ�� Volume ������ �����.
VOID  ClearRegMountedDevices();


// �Ϸ��ƾ
NTSTATUS  FsdCtrlCompletion( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext );

// IRP_MN_MOUNT_VOLUME --> Call
NTSTATUS  FsdCtrlMountVolume( IN PDEVICE_OBJECT pDeviceObject, PFLT_EXTENSION pDevExt, IN PIRP pIrp );

// WorkItem --> WorkThread Routine Win2000 �� ȣ��
void FsdCtrlMountVolumeRoutine( IN PVOID pContext );

// ������ ����ġ �ϴ� �Լ�
NTSTATUS 
FsdCtrlMountVolumeComplete( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PDEVICE_OBJECT pNewDeviceObject );

/****************************************************************************************************************************/
/****************************************************************************************************************************/

// ĳ�̵� �����͸� Flush �Ѵ�.
BOOLEAN  
File_FlushCacheRequest( IN PFILE_OBJECT pFileObject, IN PWCHAR pwzFullPath );
// ĳ�̵� �����͸� Flush �Ѵ�.
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