#include "MalwFind_DataTypes.h"
#include "MalwFind.h"
#include "MalwFind_FastIoFunc.h"
#include "MalwFind_FileFunc.h"
#include "MalwFind_Process.h"
#include "MalwFind_Command.h"
#include "MalwFind_Acctl.h"
#include "MalwFind_LogFunc.h"
#include "MalwFind_RegFunc.h"
#include "MalwFind_Util.h"
#include "MalwFind_DispHook.h"


extern const WCHAR*      g_pEXE_MALWFIND_AGENT;
extern const WCHAR*      g_pFsdName[FSD_MAX+1];
extern FAST_IO_DISPATCH  g_FastIoDispatch;

G_MALW_FIND  g_MalwFind = { 0 };


NTSTATUS  
CompleteRequest( PIRP pIrp, NTSTATUS Status, ULONG_PTR Information )
{
	pIrp->IoStatus.Status      = Status;
	pIrp->IoStatus.Information = Information;
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	return Status;
}


/*************************************************************************************/
// Author      : Written by taehwauser
// Date        : 2011.10.17
// Description : DeviceType 마다 분기처리
/*************************************************************************************/
NTSTATUS  
DriverDispatch( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
	NTSTATUS       Status  = STATUS_SUCCESS;
	PFLT_EXTENSION pDevExt = NULL;	
	
	if( !pIrp || (PVOID)pIrp < (PVOID)0x80000000 )
	{
		Status = STATUS_INVALID_PARAMETER;
		return CompleteRequest( pIrp, Status, 0 );
	}

	if( !pDeviceObject || (PVOID)pDeviceObject < (PVOID)0x80000000 )
	{
		Status = STATUS_INVALID_PARAMETER;
		KdPrint(("STATUS_INVALID_PARAMETER >> DriverDispatch >> pDeviceObject=0x%08x \n", pDeviceObject )); 
		return CompleteRequest( pIrp, Status, 0 );
	}

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;	
	if(!pDevExt)
	{
		KdPrint(("DriverDispatch Extension IS NULL !\n"));
		Status = STATUS_UNSUCCESSFUL;
		return CompleteRequest( pIrp, Status, 0 );
	}

	Status = CallDriverSkip( pDevExt, pIrp );
	return Status;

}


void DriverUnload(IN PDRIVER_OBJECT  pDriverObject )
{
	// RegFunction Call Unregister
#if (NTDDI_VERSION >= NTDDI_WINXP)
	RegFuncUnregister();
#endif

	if(pDriverObject) 
	{
		IoUnregisterFsRegistrationChange( pDriverObject, FltFs_Notification );
	}
	
	g_MalwFind_UnInit();

}


void FltDrvSFolder_Init( IN PFLT_DRV_SFOLDER pDrvSFolder )
{
	ASSERT( pDrvSFolder );
	if(!pDrvSFolder) return;

	RtlZeroMemory( pDrvSFolder, sizeof(FLT_DRV_SFOLDER) );

	InitializeListHead( &pDrvSFolder->ListHead_ExceptDir  );
	InitializeListHead( &pDrvSFolder->ListHead_ExceptProc );
	ExInitializeFastMutex( &pDrvSFolder->LockSFolder );

	// 공유폴더 Hooking 플래그
	pDrvSFolder->bSFolderHook     = FALSE;
	pDrvSFolder->pSrvsvc_Dispatch = NULL;

	pDrvSFolder->Policy.Vol.ulDrives = ZERO_VOLUME;
	pDrvSFolder->Policy.ulAcctl      = MALWF_ACCESS;
	pDrvSFolder->Policy.bLog         = LOG_OFF;

}



void FltDrvSFolder_UnInit( IN PFLT_DRV_SFOLDER  pDrvSFolder )
{
	PLIST_ENTRY   pListEntry = NULL;
	PCONTROL_OBJ  pObject    = NULL;

	if(!pDrvSFolder) 
	{
		return;
	}

	ExAcquireFastMutex( &g_MalwFind.DrvSFolder.LockSFolder );
	// 예외폴더 삭제
	while(!IsListEmpty( &pDrvSFolder->ListHead_ExceptDir ))
	{	
		pListEntry = RemoveTailList( &pDrvSFolder->ListHead_ExceptDir );
		pObject = (PCONTROL_OBJ)pListEntry;
		ASSERT( pObject ); 
		if(pObject) ControlObjDelete( pObject );
	}
	// 예외프로세스 삭제
	while(!IsListEmpty( &pDrvSFolder->ListHead_ExceptProc ))
	{	
		pListEntry = RemoveTailList( &pDrvSFolder->ListHead_ExceptProc );
		pObject = (PCONTROL_OBJ)pListEntry;
		ASSERT( pObject ); 
		if(pObject) ControlObjDelete( pObject );
	}
	ExReleaseFastMutex( &g_MalwFind.DrvSFolder.LockSFolder );	

}



/***************************************************************************************************/
// Log
/***************************************************************************************************/

NTSTATUS FltDrvLog_Init( IN PFLT_DRV_LOG pDrvLog )
{
	NTSTATUS        Status = STATUS_SUCCESS;
	UNICODE_STRING  CommEventName = {0};

	if(!pDrvLog)  
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> FltDrvLog_Init >> pDrvLog == NULL \n")); 
		return STATUS_INVALID_PARAMETER;
	}

	RtlZeroMemory( pDrvLog, sizeof(FLT_DRV_LOG) );
	pDrvLog->ulLogStart = LOG_OFF;

	//LogCommEventHandle;
	pDrvLog->hLogCommEventHandle = NULL;
	pDrvLog->pLogCommEvent = NULL;
		
	ExInitializeFastMutex( &pDrvLog->LogLock );

	// 로그리스트
	InitializeListHead( &pDrvLog->LogListHead );
		
	RtlInitUnicodeString( &CommEventName, MALWFIND_BASE_KERNEL_EVENT );
	pDrvLog->pLogCommEvent = IoCreateSynchronizationEvent( &CommEventName, &pDrvLog->hLogCommEventHandle );
	if(pDrvLog->pLogCommEvent)
	{
		KdPrint(("%wZ \n", &CommEventName ));
		KeClearEvent( pDrvLog->pLogCommEvent ); // Non-Signal State Set
	}
	
	// 최근로그 리스트
	pDrvLog->nRecentIndex = 0;
	RtlZeroMemory( &pDrvLog->RecentTable, sizeof(LOG_RECENT)*MAX_RECENT );

	return Status;

}



// PFLT_DRV_LOG  Uninit
NTSTATUS FltDrvLog_UnInit( IN PFLT_DRV_LOG  pDrvLog )
{
	if(!pDrvLog)
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> FltDrvLog_UnInit >> pDrvLog == NULL \n")); 
		return STATUS_INVALID_PARAMETER;
	}

	pDrvLog->ulLogStart = LOG_OFF;
	if(pDrvLog->hLogCommEventHandle)
	{
		ZwClose( pDrvLog->hLogCommEventHandle );
		pDrvLog->hLogCommEventHandle = NULL;
		pDrvLog->pLogCommEvent = NULL;
	}

	// 최근로그 리스트
	pDrvLog->nRecentIndex = 0;
	RtlZeroMemory( &pDrvLog->RecentTable, sizeof(LOG_RECENT)*MAX_RECENT );

	LOG_DeleteAll();

	return STATUS_SUCCESS;

}



/******************************************************************************************************************/
// FLT_DRV_CONFIG 
/******************************************************************************************************************/

void FltDrvConfig_Init( IN PFLT_DRV_CONFIG pDrvConfig )
{	
	NTSTATUS  Status = STATUS_SUCCESS;
	
	if(!pDrvConfig) 
	{
		return;
	}

	// 자체보호 플래그
	pDrvConfig->bGlobalCtrl = FLAG_OFF;

	pDrvConfig->pLowerDeviceObject   = NULL;
	pDrvConfig->pLogDiskDeviceObject = NULL;

	// SystemRoot 
	pDrvConfig->ulSystemRoot_Length = 0;
	RtlZeroMemory( pDrvConfig->wzSystemRoot, MAX_260_PATH*sizeof(WCHAR) );

	// SetupDir Path 
	pDrvConfig->ulSetupDir_Length = 0;
	RtlZeroMemory( pDrvConfig->wzSetupDir,   MAX_260_PATH*sizeof(WCHAR) );
	RtlZeroMemory( pDrvConfig->wzUpperDir,   MAX_260_PATH*sizeof(WCHAR) );
	RtlZeroMemory( pDrvConfig->wzHideFolder, MAX_DIR_NAME*sizeof(WCHAR) );

	// SystemProcess
	pDrvConfig->ulSysProc_Length = 0;
	RtlZeroMemory( pDrvConfig->wzSysProc, MAX_260_PATH*sizeof(WCHAR) );
	
	// 자체제어 예외프로세스
	InitializeListHead( &pDrvConfig->ListHead_ExceptProc );
	// FastMutex
	ExInitializeFastMutex( &pDrvConfig->LockConfig );

	Status = GetSystemRootPath( pDrvConfig->wzSystemRoot, MAX_260_PATH );
	if(NT_SUCCESS( Status ))
	{	
		pDrvConfig->ulSystemRoot_Length = wcslen( pDrvConfig->wzSystemRoot );
		if(pDrvConfig->ulSystemRoot_Length > 1) 
		{
			pDrvConfig->ulSystemRoot_Length++;
		}		

		pDrvConfig->wzSetupDir[0] = pDrvConfig->wzSystemRoot[0];
		pDrvConfig->wzSetupDir[1] = pDrvConfig->wzSystemRoot[1];
		pDrvConfig->wzSetupDir[2] = pDrvConfig->wzSystemRoot[2];
		pDrvConfig->wzSetupDir[3] = L'\0';

		pDrvConfig->wzUpperDir[0] = pDrvConfig->wzSystemRoot[0];
		pDrvConfig->wzUpperDir[1] = pDrvConfig->wzSystemRoot[1];
		pDrvConfig->wzUpperDir[2] = pDrvConfig->wzSystemRoot[2];
		pDrvConfig->wzUpperDir[3] = L'\0';

		pDrvConfig->ulSetupDir_Length = wcslen( pDrvConfig->wzSetupDir );
		if(pDrvConfig->ulSetupDir_Length > 1) 
		{
			pDrvConfig->ulSetupDir_Length++;
		}

		Status = Config_SetDeviceObject( pDrvConfig );
		if(NT_SUCCESS( Status )) 
		{
			KdPrint(("Config_SetDeviceObject: SuccessFully. \n"));
		}
	}

}


void FltDrvConfig_UnInit( IN PFLT_DRV_CONFIG pDrvConfig )
{
	PLIST_ENTRY   pListEntry = NULL;
	PCONTROL_OBJ  pObject    = NULL;

	if(!pDrvConfig) 
	{
		return;
	}
	
	pDrvConfig->pLowerDeviceObject   = NULL;
	pDrvConfig->pLogDiskDeviceObject = NULL;
	ExAcquireFastMutex( &pDrvConfig->LockConfig );
	// 예외프로세스 리스트
	while(!IsListEmpty( &pDrvConfig->ListHead_ExceptProc ))
	{
		pListEntry = RemoveTailList( &pDrvConfig->ListHead_ExceptProc );
		if(!pListEntry) continue;

		pObject = (PCONTROL_OBJ)pListEntry;
		if(pObject) 
		{
			ControlObjDelete( pObject );
		}
	}
	ExReleaseFastMutex( &pDrvConfig->LockConfig );
}



// /******************************************************************************************************************
//  g_MalwFind 
// /******************************************************************************************************************


BOOLEAN g_MalwFind_Init(void)
{	
	ULONG  ulOSMajorVer=0, ulOSMinorVer=0;
	RtlZeroMemory( &g_MalwFind, sizeof(g_MalwFind) );

	PsGetVersion( &ulOSMajorVer, &ulOSMinorVer, NULL, NULL );	
	g_MalwFind.ulOSVer = ( (ulOSMajorVer << 8) | ulOSMinorVer );
	KdPrint(("<< [ OSVerInfo ] >> OSVer=0x%04x \n", g_MalwFind.ulOSVer )); 

	// g_MalwFind.pDriverUnload = DriverUnload;	
	ExInitializeResourceLite( &g_MalwFind.LockResource );	
	ExInitializeFastMutex( &g_MalwFind.LockDevice );
	
	// Policy
	FltDrvFile_Init   (  &g_MalwFind.DrvFile    ); // File, Fdd, UsbStor, 
	FltDrvSFolder_Init(  &g_MalwFind.DrvSFolder ); // SFolder 
	FltDrvProcess_Init(  &g_MalwFind.DrvProc    ); // Process 
			
	// NonPagedPool
	ExInitializeNPagedLookasideList( &g_MalwFind.FsdNPagedList, NULL, NULL, 0, MAX_POOL_LEN, MALWFIND_NAME_TAG, 0 );
	// NonPagedPoool
	ExInitializeNPagedLookasideList( &g_MalwFind.RegNPagedList, NULL, NULL, 0, MAX_POOL_LEN, MALWFIND_NAME_TAG, 0 );

	// NonPagedPoool
	ExInitializeNPagedLookasideList( &g_MalwFind.ProcNPagedList, NULL, NULL, 0, (MAX_PROCESS_LEN << 1), MALWFIND_NAME_TAG, 0 );


	
	// Attach Distinguish
	g_MalwFind.bLoadMode = TRUE;
	// MountedDevice Pointer
	g_MalwFind.pMountedDevice = NULL;
	
	g_MalwFind.RefProc.ulImageName = 0;
	g_MalwFind.RefProc.ulPeb       = 0;
	g_MalwFind.RefProc.ulPId       = 0;

	g_MalwFind.bEntryInit = FALSE; 


	return TRUE;
}


void g_MalwFind_UnInit(void)
{
	// Log UnInit
	FltDrvLog_UnInit( &g_MalwFind.DrvLog );
	
	// Policy
	FltDrvSFolder_UnInit( &g_MalwFind.DrvSFolder ); // SFolder
	FltDrvProcess_UnInit( &g_MalwFind.DrvProc ); // Proess 
	FltDrvFile_UnInit( &g_MalwFind.DrvFile );       // File, Fdd, UsbStor
	
	// Config
	FltDrvConfig_UnInit( &g_MalwFind.DrvConfig );
	// Resource Delete
	ExDeleteResourceLite( &g_MalwFind.LockResource );
		
	// NonPagedPool
	ExDeleteNPagedLookasideList( &g_MalwFind.FsdNPagedList );
	ExDeleteNPagedLookasideList( &g_MalwFind.RegNPagedList );

	ExDeleteNPagedLookasideList( &g_MalwFind.ProcNPagedList );


	if(g_MalwFind.pMountedDevice) ClearRegMountedDevices();

	// REF_PROCESS
	g_MalwFind.RefProc.ulImageName  = 0;
	g_MalwFind.RefProc.ulPeb        = 0;
	g_MalwFind.RefProc.ulPId        = 0;

	// Setup Dir Control
	g_MalwFind.DrvConfig.bGlobalCtrl = FALSE;

	g_MalwFind.pDriverUnload = NULL;
	g_MalwFind.pDeviceObject = NULL;
	g_MalwFind.pDriverObject = NULL;

	// System Callback 제거
	PsSetCreateProcessNotifyRoutine( FltDrv_ProcessNotify_Routine, TRUE );
	
}


/******************************************************************************************************************/
// FLT_DRV_FILE
/******************************************************************************************************************/

void FltDrvFile_Init( IN PFLT_DRV_FILE pDrvFile )
{
	if(!pDrvFile) return;

	RtlZeroMemory( pDrvFile, sizeof(FLT_DRV_FILE) );

	// TRUE:  (BlackList) 리스트 차단함 
    // FALSE: (WhiteList) 리스트만 허용함 나머지 차단
	pDrvFile->bExtDeny = TRUE;
	InitializeListHead( &pDrvFile->ListHead_FileExt       ); // 1. 확장자차단 리스트
	InitializeListHead( &pDrvFile->ListHead_FileExtExcept ); // 2. 확장자차단 예외리스트

	InitializeListHead( &pDrvFile->ListHead_ExceptDir     ); // 2. 예외폴더
	InitializeListHead( &pDrvFile->ListHead_ExceptProc    ); // 3. 예외프로세스

	ExInitializeFastMutex( &pDrvFile->LockFileExt );	
	// FDD
	pDrvFile->FddPolicy.ulAcctl      = MALWF_ACCESS;
	pDrvFile->FddPolicy.bLog         = LOG_OFF;
	pDrvFile->FddPolicy.Vol.ulDrives = ZERO_VOLUME;	
	// UsbStor
	pDrvFile->UsbPolicy.ulAcctl      = MALWF_ACCESS;
	pDrvFile->UsbPolicy.bLog         = LOG_OFF;
	pDrvFile->UsbPolicy.Vol.ulDrives = ZERO_VOLUME;
	// FileExt
	pDrvFile->FileExtPolicy.ulAcctl      = MALWF_ACCESS;
	pDrvFile->FileExtPolicy.bLog         = LOG_OFF;
	pDrvFile->FileExtPolicy.Vol.ulDrives = ZERO_VOLUME;		

}


void FltDrvFile_UnInit( IN PFLT_DRV_FILE pDrvFile )
{
	PLIST_ENTRY   pListEntry = NULL;
	PCONTROL_OBJ  pObject    = NULL;
	PFILE_EXT     pFileExt   = NULL;
	
	ASSERT( pDrvFile );
	if(!pDrvFile) return;

	ExAcquireFastMutex( &pDrvFile->LockFileExt  );
	
	// 확장자제어 차단리스트
	while(!IsListEmpty( &pDrvFile->ListHead_FileExt ))
	{
		pListEntry = RemoveTailList( &pDrvFile->ListHead_FileExt );
		pFileExt   = (PFILE_EXT)pListEntry;
		if(pFileExt) ControlFileExtDelete( pFileExt );
	}
	
	// 예외폴더 리스트
	while(!IsListEmpty( &pDrvFile->ListHead_ExceptDir ))
	{
		pListEntry = RemoveTailList( &pDrvFile->ListHead_ExceptDir );
		pObject = (PCONTROL_OBJ)pListEntry;
		ASSERT( pObject );
		if(pObject) ControlObjDelete( pObject );
	}
	// 예외프로세스 리스트
	while(!IsListEmpty( &pDrvFile->ListHead_ExceptProc ))
	{
		pListEntry = RemoveTailList( &pDrvFile->ListHead_ExceptProc );
		pObject = (PCONTROL_OBJ)pListEntry;
		ASSERT( pObject );
		if(pObject) ControlObjDelete( pObject );
	}
	
	ExReleaseFastMutex( &pDrvFile->LockFileExt );

}




// /******************************************************************************************************************
//  FLT_DRV_PROCESS
// /******************************************************************************************************************


void FltDrvProcess_UnInit( IN PFLT_DRV_PROCESS pDrvProc )
{	
	PCONTROL_OBJ  pObject      = NULL;
	PLIST_ENTRY   pListEntry   = NULL;
	PDRV_P_ENTRY  pProcLCEntry = NULL;

	ASSERT( pDrvProc );
	if(!pDrvProc) return;


	ExAcquireFastMutex( &pDrvProc->LockProc );

	// 라이프 사이클 
	while(!IsListEmpty( &pDrvProc->ListHead_ProcLCEntry ))
	{
		pListEntry = RemoveTailList( &pDrvProc->ListHead_ProcLCEntry );
		pProcLCEntry = (PDRV_P_ENTRY)pListEntry;
		if(pProcLCEntry) Drv_ProcLC_Delete( pProcLCEntry );
	}

	// 예외폴더 리스트
	while(!IsListEmpty( &pDrvProc->ListHead_ExceptDir ))
	{
		pListEntry = RemoveTailList( &pDrvProc->ListHead_ExceptDir );
		pObject = (PCONTROL_OBJ)pListEntry;
		ASSERT( pObject );
		if(pObject) ControlObjDelete( pObject );
	}

	// 제어프로세스 리스트
	while(!IsListEmpty( &pDrvProc->ListHead_ProcCtrl ))
	{
		pListEntry = RemoveTailList( &pDrvProc->ListHead_ProcCtrl );
		pObject = (PCONTROL_OBJ)pListEntry;
		ASSERT( pObject );
		if(pObject) ControlObjDelete( pObject );
	}

	Drv_ProcLC_CloseEvent();
	ExReleaseFastMutex( &pDrvProc->LockProc );

}


void FltDrvProcess_Init( IN PFLT_DRV_PROCESS pDrvProc )
{
	NTSTATUS        Status = STATUS_SUCCESS;
//	UNICODE_STRING  ProcKillEvent;

	ASSERT( pDrvProc );
	if(!pDrvProc) return;
/*****************************************************************************************************************************/
	InitializeListHead( &pDrvProc->ListHead_ProcCtrl  ); // 프로세스 파일제어
	InitializeListHead( &pDrvProc->ListHead_ExceptDir ); // 예외 폴더
	ExInitializeFastMutex( &pDrvProc->LockProc );
/*****************************************************************************************************************************/	
	
	pDrvProc->Policy.Vol.ulDrives = ZERO_VOLUME;
	pDrvProc->Policy.ulAcctl = MALWF_ACCESS;
	pDrvProc->Policy.bLog    = LOG_OFF;

/*****************************************************************************************************************************/
	//프로세스 LifeCycle
	InitializeListHead( &pDrvProc->ListHead_ProcLCEntry ); // LifeCycle 
	pDrvProc->pProcLC_Event       = NULL;
	pDrvProc->hProcLC_EventHandle = NULL;

	Status = Drv_ProcLC_CreateEvent();

}




// /******************************************************************************************************************
//  DEVICE_EXTENSION 
// /******************************************************************************************************************

BOOLEAN 
FltDeviceExtension_Init( IN PFLT_EXTENSION pDevExt )
{
	ASSERT( pDevExt );
	if(!pDevExt) return FALSE;

	pDevExt->wcVol               = L'';
	pDevExt->ullReserved         = 0;
	pDevExt->pLowerDeviceObject  = NULL;
	pDevExt->pVolumeDeviceObject = NULL;
	pDevExt->pRealDeviceObject   = NULL;

	return TRUE;

}


void FltDeviceExtension_UnInit( IN PFLT_EXTENSION pDevExt )
{
	ASSERT( pDevExt );
	if(!pDevExt) return;

	pDevExt->wcVol          = L'';
	pDevExt->ullReserved    = 0;
	pDevExt->pLowerDeviceObject  = NULL;
	pDevExt->pVolumeDeviceObject = NULL;
	pDevExt->pRealDeviceObject   = NULL; 


}








/*
Author      : Written by taehwauser
Date        : 2008.06.20
Description : 드라이버의 EntryPoint  
*/

NTSTATUS 
DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegPath)
{
	NTSTATUS        Status = STATUS_SUCCESS;
	UNICODE_STRING  DeviceName, SymbolicName;
	PFLT_EXTENSION  pDevExt = NULL;
	PMOUNTED_DEVICE pMountDevice = NULL;
	PDEVICE_OBJECT  pDeviceObject = NULL;
	USHORT          i = 0;
	UNREFERENCED_PARAMETER( pRegPath );
	RtlInitUnicodeString( &DeviceName,   MALWFIND_DEV_NAME    );
	RtlInitUnicodeString(&SymbolicName, MALWFIND_SYM_NAME);

	PAGED_CODE();

	// Control Device Object 생성
	Status = IoCreateDevice( pDriverObject, sizeof(FLT_EXTENSION), &DeviceName, FILE_DEVICE_MALWFIND, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject );
	if(!NT_SUCCESS( Status ))  return Status;

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	if(!pDevExt) return STATUS_UNSUCCESSFUL;
	
	RtlZeroMemory( pDevExt, sizeof(FLT_EXTENSION)  );
	FltDeviceExtension_Init( pDevExt );

	Status = IoCreateSymbolicLink( &SymbolicName, &DeviceName );
	if(!NT_SUCCESS( Status ))
	{
		IoDeleteDevice( pDeviceObject );
		pDeviceObject  = NULL;
		return Status;
	}

	// 전역 컨텍스트 초기화
	g_MalwFind_Init();	

	// DriverEntry 초기화 관련 플래그
	g_MalwFind.bEntryInit    = FALSE;
	g_MalwFind.pDriverObject = pDriverObject;
	g_MalwFind.pDeviceObject = pDeviceObject;
	
//	pDriverObject->DriverUnload   = DriverUnload;		
    for(i=0; i<=IRP_MJ_MAXIMUM_FUNCTION; i++) 
	{
        pDriverObject->MajorFunction[ i ] = DriverDispatch;
    }

	pDriverObject->FastIoDispatch = &g_FastIoDispatch;
	pDriverObject->MajorFunction[ IRP_MJ_CLOSE   ] = File_PreClose;
	pDriverObject->MajorFunction[ IRP_MJ_CLEANUP ] = File_PreClose;
	pDriverObject->MajorFunction[ IRP_MJ_CREATE  ] = File_PreCreate;  
	pDriverObject->MajorFunction[ IRP_MJ_READ    ] = File_PreRead;
	pDriverObject->MajorFunction[ IRP_MJ_WRITE   ] = File_PreWrite;	
	pDriverObject->MajorFunction[ IRP_MJ_SET_INFORMATION    ] = File_PreSetInformation;
	pDriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL     ] = File_PreDeviceControl;
	pDriverObject->MajorFunction[ IRP_MJ_SYSTEM_CONTROL     ] = File_PreSystemControl;
	pDriverObject->MajorFunction[ IRP_MJ_FILE_SYSTEM_CONTROL] = File_PreFileSystemControl;

	// 폴더하이드
	pDriverObject->MajorFunction[ IRP_MJ_DIRECTORY_CONTROL  ] = File_PreDirControl;

	g_MalwFind.bLoadMode = TRUE;
	// Volume Mounted AllocatePool
	Status = GetRegMountedDevices( &pMountDevice );	

	Status = IoRegisterFsRegistrationChange( g_MalwFind.pDriverObject, FltFs_Notification );
	if(!NT_SUCCESS( Status )) 
	{
		KdPrint(("IoRegisterFsRegistrationChange Failed. \n"));
	}

	// Win2000 드라이버 올리기
	if(g_MalwFind.ulOSVer < OS_VER_WXP)
	{
		// FsdManualAttach();
		FsdMountedVolumeAttach();
	}

	// Process Offset Setting
	FsdSetRefProcess();
	
	// RegCallBack 	
	if(g_MalwFind.ulOSVer >= OS_VER_WNET)
	{
		// RegFuncRegister( pDriverObject );
	}

	// Process Callback 등록
	Status = PsSetCreateProcessNotifyRoutine( FltDrv_ProcessNotify_Routine, FALSE );
	// Config
	FltDrvConfig_Init( &g_MalwFind.DrvConfig );
	// Log
	FltDrvLog_Init( &g_MalwFind.DrvLog );
	
	g_MalwFind.bLoadMode = FALSE;	
	// DriverEntry 초기화 관련 플래그
	g_MalwFind.bEntryInit = TRUE;
	return Status;

}


// Reg 에 등록된 볼륨 올린다.
void  FsdMountedVolumeAttach()
{
	NTSTATUS         Status = STATUS_SUCCESS;
	PDEVICE_OBJECT   pDeviceObject = NULL;
	PDEVICE_OBJECT   pNewDeviceObject = NULL;
	PMOUNTED_DEVICE  pMntDevice    = NULL;
	WCHAR            wzVolume[ MAX_DOSDEVICE_NAME ] = L""; // 8
	
	// DriverEntry 에서 호출하는것은 Sikp
	if(g_MalwFind.bLoadMode == FALSE) // DriverEntry 에서 호출하는것이 아니다.
	{
		PMOUNTED_DEVICE pFetchMountDevice = NULL;
		GetRegMountedDevices( &pFetchMountDevice );
	}

	pMntDevice  = g_MalwFind.pMountedDevice;
	while(pMntDevice)
	{	// 관련되어 있는 스택인지 검사한다.
		pDeviceObject = pMntDevice->pDeviceObject;
		if(pDeviceObject)
		{
			RtlZeroMemory( wzVolume, sizeof(wzVolume) );
			RtlStringCchCopyW( wzVolume, (sizeof(wzVolume) >> 1), pMntDevice->DosDeviceName + wcslen( FSD_DOSDEVICES ) );

			pNewDeviceObject = NULL;
			Status = FsdAttachDeviceWithCreate( pDeviceObject, NULL, wzVolume, &pNewDeviceObject );
			if(NT_SUCCESS( Status ))
			{
				KdPrint(("Status=[%08x]--Volume:[ %ws ] Attached Successfully. \n", Status, wzVolume ));
			}
		}
		pMntDevice = pMntDevice->pNext;
	}

}



// Windows 
// 수동으로 올린다.
NTSTATUS  FsdManualAttach()
{
	NTSTATUS        Status           = STATUS_SUCCESS;
	PFILE_OBJECT    pFileObject      = NULL;
	PDEVICE_OBJECT  pDeviceObject    = NULL;
	UNICODE_STRING  DeviceName       = {0};
	USHORT          usIndex          = 0;

	for(usIndex=0; usIndex<FSD_MAX; usIndex++)
	{		
		RtlInitUnicodeString( &DeviceName, g_pFsdName[ usIndex ] );

		Status = IoGetDeviceObjectPointer( &DeviceName, FILE_ANY_ACCESS, &pFileObject, &pDeviceObject); 
		if(NT_SUCCESS(Status))
		{
			KdPrint(("FsdManualAttach:IoGetDeviceObjectPointer:[%ws] \n", DeviceName.Buffer ));			
			ObDereferenceObject( pFileObject );

			Status = FsdAttachToFileSystem( pDeviceObject, &DeviceName );
			if(NT_SUCCESS( Status ))
			{
				KdPrint(("FsdManualAttach:FsdAttachDeviceWithCreate:[%ws] Successfully. \n", DeviceName.Buffer ));
			}
		}
	}

	return Status;
}





// Attach   DeviceObject
NTSTATUS  
FsdAttachDeviceWithCreate( IN PDEVICE_OBJECT pDeviceObject, PUNICODE_STRING pDeviceName, PWCHAR pVolumeName, PDEVICE_OBJECT*  ppNewDeviceObject )
{
	NTSTATUS       Status  = STATUS_SUCCESS;
	PFLT_EXTENSION pDevExt = NULL;

	UNREFERENCED_PARAMETER( pDeviceName );

	// Attach 되어있는지 검사
	if(Attached_MalwFind_DeviceObject( pDeviceObject )) 
	{	// 이미 디바이스에 올라가 있다.
		KdPrint(("DeviceStack  Already Exist! \n"));		
		return STATUS_UNSUCCESSFUL;
	}

	// NHCAFlt DeviceObject 생성
	Status = FsdCreateDevice( pDeviceObject, ppNewDeviceObject );
	if(!NT_SUCCESS( Status ))   
	{
		return Status;
	}

	pDevExt = (PFLT_EXTENSION)((*ppNewDeviceObject)->DeviceExtension);
	if(!pDevExt)  
	{
		IoDeleteDevice( *ppNewDeviceObject );
		*ppNewDeviceObject = NULL;
		return STATUS_NO_SUCH_DEVICE;
	}

	// DeviceObject 에 우리의 DeviceObject 를 Attach 한다.
	Status = FsdAttachToDeviceStack( *ppNewDeviceObject, pDeviceObject, &pDevExt->pLowerDeviceObject );
	if(!NT_SUCCESS( Status ))  
	{
		IoDeleteDevice( *ppNewDeviceObject );
		*ppNewDeviceObject = NULL;
		return Status;
	}


	if(pVolumeName)
	{
		pDevExt->wcVol = (WCHAR)toupper( pVolumeName[0] );
	}

	if(FlagOn( pDeviceObject->Flags,  DO_DIRECT_IO ))   
	{
		SetFlag( (*ppNewDeviceObject)->Flags, DO_DIRECT_IO   );
	}
	if(FlagOn( pDeviceObject->Flags, DO_BUFFERED_IO ))  
	{
		SetFlag( (*ppNewDeviceObject)->Flags, DO_BUFFERED_IO );
	}
	if(FlagOn( pDeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN )) 
	{
		SetFlag( (*ppNewDeviceObject)->Characteristics, FILE_DEVICE_SECURE_OPEN );
	}

	// 위에 디바이스가 어태치 할수 있도록 플래그를 클리어 한다.
	ClearFlag( (*ppNewDeviceObject)->Flags, DO_DEVICE_INITIALIZING );

	return Status;
}


NTSTATUS  
Protect_SetupFolder_Control( IN PWCHAR pwzProcName, IN PNAME_BUFFER pNameBuffer, IN ULONG ulAccess )
{
	ULONG  ulAcctl = 0;
	WCHAR  wzBuffer[ 100 ];

	if(!pNameBuffer || !pNameBuffer->pBuffer || !pwzProcName) 
	{
		return  STATUS_SUCCESS;
	}

	// SetupDir Control == FALSE:제어안함   TRUE: 제어함 
	if(!g_MalwFind.DrvConfig.bGlobalCtrl) return STATUS_SUCCESS;
	if(!_wcsicmp( pwzProcName, L"services.exe")) return STATUS_SUCCESS;

	// Write Access 인지 판단한다.
	ulAcctl = (WRITE_ACCESS & ulAccess);
	if(!ulAcctl) 
	{
		return STATUS_SUCCESS;
	}

	if(!_wcsnicmp( pNameBuffer->pBuffer, g_MalwFind.DrvConfig.wzSetupDir, wcslen(g_MalwFind.DrvConfig.wzSetupDir) ))
	{	
		RtlZeroMemory( wzBuffer, sizeof(WCHAR)*100 );
		if(ulAccess & MALWF_CREATE) RtlStringCchCopyW( wzBuffer, 100, L"MALWF_CREATE | ");
		if(ulAccess & MALWF_WRITE)  RtlStringCchCatW(  wzBuffer, 100, L"MALWF_WRITE  | "); 
		if(ulAccess & MALWF_DELETE) RtlStringCchCatW(  wzBuffer, 100, L"MALWF_DELETE | "); 
		if(ulAccess & MALWF_RENAME) RtlStringCchCatW(  wzBuffer, 100, L"MALWF_RENAME | "); 
		KdPrint(("[Protect_SetupFolder_Control] >> AccessMode=( %ws ), PID=%04d, ProcName=[%ws]-[%ws] \n", wzBuffer, PsGetCurrentProcessId(), pwzProcName, pNameBuffer->pBuffer ));

		return STATUS_ACCESS_DENIED;		
	}
	return STATUS_SUCCESS;		

}




// 자체보호 폴더 숨기기
// Windows W2k, XP, 2003
//
BOOLEAN  
Protect_FolderHide_WXP( IN PIRP pIrp, 
				        IN OUT PFILE_BOTH_DIR_INFORMATION  pDirInfo, 
					    IN OUT PFILE_BOTH_DIR_INFORMATION  pPrevDirInfo )
{
	PFILE_BOTH_DIR_INFORMATION  pNextDirInfo = NULL;

	if(!pIrp || !pDirInfo) 
	{
		return FALSE;
	}

	pNextDirInfo = (PFILE_BOTH_DIR_INFORMATION)((PUCHAR)pDirInfo + (ULONG)(pDirInfo->NextEntryOffset));	

	// 첫 엔트리일 경우
	if(!pNextDirInfo)
	{
		pDirInfo = pNextDirInfo;
		if(pDirInfo)
		{
			pIrp->IoStatus.Information -= pDirInfo->NextEntryOffset;
		}
	}
	else if(pPrevDirInfo && !pDirInfo->NextEntryOffset)   // 마지막 Entry일 경우
	{
		pPrevDirInfo->NextEntryOffset = 0;
		pIrp->IoStatus.Information -= pPrevDirInfo->NextEntryOffset;
	}
	else  // 중간에 포함될 경우
	{
		if(pPrevDirInfo)
		{
			pPrevDirInfo->NextEntryOffset += pDirInfo->NextEntryOffset;
			pIrp->IoStatus.Information -= pDirInfo->NextEntryOffset;
		}
	}

	return TRUE;
}



// 자체보호 폴더 숨기기
// Windows Vista 이상
//
BOOLEAN  
Protect_FolderHide_WLH( IN PIRP pIrp, 
					    IN OUT PFILE_ID_BOTH_DIR_INFORMATION  pDirInfo, 
						IN OUT PFILE_ID_BOTH_DIR_INFORMATION  pPrevDirInfo )
{
	PFILE_ID_BOTH_DIR_INFORMATION pNextDirInfo = NULL;

	if(!pIrp || !pDirInfo) 
	{
		return FALSE;
	}

	pNextDirInfo = (PFILE_ID_BOTH_DIR_INFORMATION)((PUCHAR)pDirInfo + (ULONG)(pDirInfo->NextEntryOffset));		
	if(!pNextDirInfo)  // 첫 엔트리일 경우
	{
		pDirInfo = pNextDirInfo;
		if(pDirInfo)
		{
			pIrp->IoStatus.Information -= pDirInfo->NextEntryOffset;
		}
	}
	else if(pPrevDirInfo && !pDirInfo->NextEntryOffset)   // 마지막 Entry일 경우
	{
		pPrevDirInfo->NextEntryOffset = 0;
		pIrp->IoStatus.Information -= pPrevDirInfo->NextEntryOffset;
	}
	else  // 중간에 포함될 경우
	{
		if(pPrevDirInfo)
		{
			pPrevDirInfo->NextEntryOffset += pDirInfo->NextEntryOffset;
			pIrp->IoStatus.Information -= pDirInfo->NextEntryOffset;
		}
	}
	return TRUE;
}