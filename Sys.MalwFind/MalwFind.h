
#ifndef _MALWFIND_H_
#define _MALWFIND_H_

#include  <ntifs.h>
#include "MalwFind_DataTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif


NTSTATUS  DriverEntry(IN PDRIVER_OBJECT pDriverObject,  IN PUNICODE_STRING pRegPath );
NTSTATUS  DriverDispatch( IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp );
void      DriverUnload( IN PDRIVER_OBJECT  pDriverObject );

// 전역 구조체 초기화
BOOLEAN g_MalwFind_Init();
void    g_MalwFind_UnInit();


// Device Extension 초기화
BOOLEAN FltDeviceExtension_Init(  IN PFLT_EXTENSION pDeviceExt );
void    FltDeviceExtension_Uninit(IN PFLT_EXTENSION pDeviceExt );

// File Fdd UsbStor
void FltDrvFile_Init  ( IN PFLT_DRV_FILE pDrvFile );
void FltDrvFile_UnInit( IN PFLT_DRV_FILE pDrvFile );

// 공유폴더
void FltDrvSFolder_Init( IN PFLT_DRV_SFOLDER  pSFolder );
void FltDrvSFolder_UnInit( IN PFLT_DRV_SFOLDER  pSFolder );

// Config
void FltDrvConfig_Init  ( IN PFLT_DRV_CONFIG pConfig );
void FltDrvConfig_UnInit( IN PFLT_DRV_CONFIG pConfig );

// Process
void FltDrvProcess_Init( IN PFLT_DRV_PROCESS pManagerProc );
void FltDrvProcess_UnInit( IN PFLT_DRV_PROCESS pManagerProc );


// Log
NTSTATUS 
FltDrvLog_Init( IN PFLT_DRV_LOG pLog );

NTSTATUS 
FltDrvLog_UnInit( IN PFLT_DRV_LOG pLog );



///////////////////////////////////////////////////////////////////////////////

NTSTATUS CompleteRequest( PIRP pIrp, NTSTATUS Status, ULONG_PTR Information );
void     FsdMountedVolumeAttach(void);

NTSTATUS  
FsdAttachDeviceWithCreate( IN PDEVICE_OBJECT pDeviceObject, 
						   PUNICODE_STRING  pDeviceName, 
						   PWCHAR pVolumeName, 
						   PDEVICE_OBJECT*  ppNewDeviceObject );
/////////////////////////////////////////////////////////////////////////////
// Windows 2000 은 콜백이 오지 않는다.

NTSTATUS FsdManualAttach(void);
void     FsdManualDetach(void);

NTSTATUS    
Protect_SetupFolder_Control( IN PWCHAR pwzProcName, IN PNAME_BUFFER pNameBuffer, IN ULONG ulAccess );

// 폴더 숨기기
/*
#if (NTDDI_VERSION >= NTDDI_LONGHORN)

BOOLEAN  
Protect_FolderHide( IN PIRP pIrp, IN OUT PFILE_ID_BOTH_DIR_INFORMATION  pDirInfo, IN OUT PFILE_ID_BOTH_DIR_INFORMATION  pPrevDirInfo );

#else

BOOLEAN  
Protect_FolderHide( IN PIRP pIrp, IN OUT PFILE_BOTH_DIR_INFORMATION  pDirInfo, IN OUT PFILE_BOTH_DIR_INFORMATION  pPrevDirInfo );

#endif
*/


BOOLEAN  
Protect_FolderHide_WLH( IN PIRP pIrp, IN OUT PFILE_ID_BOTH_DIR_INFORMATION  pDirInfo, IN OUT PFILE_ID_BOTH_DIR_INFORMATION  pPrevDirInfo );

BOOLEAN  
Protect_FolderHide_WXP( IN PIRP pIrp, IN OUT PFILE_BOTH_DIR_INFORMATION  pDirInfo, IN OUT PFILE_BOTH_DIR_INFORMATION  pPrevDirInfo );








/*****************************************************************************************************************/




#ifdef __cplusplus
}
#endif



#endif
