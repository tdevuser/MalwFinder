
#include "MalwFind_Command.h"
#include "MalwFind_Acctl.h"
#include "MalwFind_LogFunc.h"
#include "MalwFind_DispHook.h"
#include "MalwFind_Process.h"


extern G_MALW_FIND  g_MalwFind;


//����̹� ����
NTSTATUS DrvCmd_GetVersion(void)
{
	return STATUS_NOT_IMPLEMENTED;
}


//�α� ���ս���/����
NTSTATUS DrvCmd_LogStart( IN PVOID pInBuffer )
{
	PDRVCMD_LogStart  pCommLogStart = NULL;
	pCommLogStart = (PDRVCMD_LogStart)pInBuffer;
	if(!pCommLogStart) return STATUS_INVALID_PARAMETER;

	KdPrint(("DrvCmd_LogStart--pBuffer=%08x \n", pInBuffer ));

	g_MalwFind.DrvLog.ulLogStart = pCommLogStart->ulLogStart;
	return STATUS_SUCCESS;
}

//�α� Fetch 
NTSTATUS 
DrvCmd_GetLogFetch( OUT PVOID pOutBuffer, IN ULONG ulOutBufferLength, OUT PIO_STATUS_BLOCK pIoStatus )
{
	ULONG    ulOutCopySize = 0;
	NTSTATUS Status = STATUS_SUCCESS;
		
	Status = LOG_Fetch( (char*)pOutBuffer, &ulOutCopySize, ulOutBufferLength );	
	if(NT_SUCCESS(Status))
	{
		if(pIoStatus) pIoStatus->Information = ulOutCopySize;
	}

	return Status;
}

//��ġ ���丮 ����
NTSTATUS 
DrvCmd_SetupDirControl( IN PVOID pInBuffer )
{
	PDRVCMD_SetupDir  pDrvCmdSetupDir = NULL;
			
	pDrvCmdSetupDir = (PDRVCMD_SetupDir)pInBuffer;
	if(!pDrvCmdSetupDir) 
	{
		return STATUS_INVALID_PARAMETER;
	}

	KdPrint(("DrvCmd_SetupDirControl--pBuffer=%08x \n", pInBuffer ));

	if(pDrvCmdSetupDir->bGlobalCtrl)
	{   // ����� 
		g_MalwFind.DrvConfig.bGlobalCtrl = TRUE;
	}
	else
	{   // ����ȵ�
		g_MalwFind.DrvConfig.bGlobalCtrl = FALSE;
	}

	Drv_ProcLC_CreateEvent();
	return STATUS_SUCCESS;
}



// ��ü ��������-��ü�������̵� 
NTSTATUS 
DrvCmd_Set_SetupDir(IN PVOID pInBuffer)
{
	NTSTATUS           Status = 0;
	ULONG              ulLength = 0;
	PDRVCMD_SetupDir pCmdSetupDir = NULL;

	pCmdSetupDir = (PDRVCMD_SetupDir)pInBuffer;
	if(!pCmdSetupDir) 
	{
		return STATUS_INVALID_PARAMETER;
	}

	KdPrint((" \nDrvCmd_Set_SetupDir"));

	//
	// System Directory
	//
	ulLength = (ULONG)wcslen(pCmdSetupDir->wzSystemDir);
	if(ulLength >= 3)
	{
		RtlZeroMemory( g_MalwFind.DrvConfig.wzSystemRoot, MAX_260_PATH*sizeof(WCHAR) );
		RtlStringCchCopyNW( g_MalwFind.DrvConfig.wzSystemRoot, MAX_260_PATH, pCmdSetupDir->wzSystemDir, MAX_260_PATH );
		g_MalwFind.DrvConfig.ulSystemRoot_Length = (ULONG)wcslen(g_MalwFind.DrvConfig.wzSystemRoot);
	}

	//
	// SetupDir Directory
	//
	ulLength = (ULONG)wcslen( pCmdSetupDir->wzSetupDir );
	if(ulLength >= 3)
	{
		PWCHAR  pwzFind = NULL;

		RtlZeroMemory( g_MalwFind.DrvConfig.wzSetupDir, MAX_260_PATH*sizeof(WCHAR) );
		RtlStringCchCopyNW( g_MalwFind.DrvConfig.wzSetupDir, MAX_260_PATH, pCmdSetupDir->wzSetupDir, MAX_260_PATH );
		g_MalwFind.DrvConfig.ulSetupDir_Length = (ULONG)wcslen( g_MalwFind.DrvConfig.wzSetupDir );

		// ���̵� ���� ���丮
		RtlZeroMemory( g_MalwFind.DrvConfig.wzUpperDir, MAX_260_PATH );
		RtlStringCchCopyNW( g_MalwFind.DrvConfig.wzUpperDir, MAX_260_PATH, pCmdSetupDir->wzSetupDir, MAX_260_PATH );		
		pwzFind = wcsrchr( g_MalwFind.DrvConfig.wzUpperDir, L'\\' );
		if(pwzFind) 
		{
			(*pwzFind) = L'\0';
		}

		// HideFolder
		RtlZeroMemory( g_MalwFind.DrvConfig.wzHideFolder, MAX_DIR_NAME*sizeof(WCHAR) );
		RtlStringCchCopyNW( g_MalwFind.DrvConfig.wzHideFolder, MAX_DIR_NAME, pCmdSetupDir->wzHideFolder, wcslen(pCmdSetupDir->wzHideFolder) );
	}

	return STATUS_SUCCESS;

}




// FILE �������� ����
NTSTATUS 
DrvCmd_F_Set_ExceptDir(IN PVOID pInBuffer)
{
	BOOLEAN             bExist        = FALSE;
	ULONG               ulLength      = 0;
	PCONTROL_OBJ        pNewDirObj    = NULL;
	PDRVCMD_ExceptFileDir pCmdExceptDir = NULL;

	pCmdExceptDir = (PDRVCMD_ExceptFileDir)pInBuffer;
	if(!pCmdExceptDir) 
	{
		return STATUS_INVALID_PARAMETER;
	}

	ulLength = (ULONG)wcslen( pCmdExceptDir->wzExceptDir );
	if(ulLength <= 0) 
	{
		return STATUS_UNSUCCESSFUL;
	}

	// �������� �ִ��� �����Ѵ�.
	bExist = ISExist_F_ExceptDir_Equal( pCmdExceptDir->wzExceptDir );
	if(bExist) 
	{
		return STATUS_SUCCESS;
	}

	pNewDirObj = ControlObjAdd( ulLength );
	ASSERT( pNewDirObj && pNewDirObj->Obj.Buffer );
	if(!pNewDirObj || !pNewDirObj->Obj.Buffer) 
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pNewDirObj->Obj.usLength = (USHORT)ulLength;
	RtlStringCchCopyNW( pNewDirObj->Obj.Buffer, 
				        pNewDirObj->Obj.usMaxLength, 
						pCmdExceptDir->wzExceptDir, 
						ulLength );

	// ������ ��ġ �����ϱ�
	pNewDirObj->Obj.pObjPos = wcsrchr( pNewDirObj->Obj.Buffer, L'\\' );
	pNewDirObj->Obj.pObjPos++;
	
	ControlHeaderInit( pNewDirObj, &g_MalwFind.DrvFile.ListHead_ExceptDir, MFOBJ_DIR );	
	
	return STATUS_SUCCESS;

}


// FILE �������� ����
NTSTATUS 
DrvCmd_F_Clr_ExceptDir(IN PVOID pInBuffer)
{
	ULONG         ulLength  = 0;
	PLIST_ENTRY   pListNode = NULL;
	PCONTROL_OBJ  pDirObj   = NULL;
	PDRVCMD_ExceptFileDir  pCmdExceptDir = NULL;

	pCmdExceptDir = (PDRVCMD_ExceptFileDir)pInBuffer;
	if(!pCmdExceptDir) 
	{
		// KdPrint(("STATUS_INVALID_PARAMETER >> DrvCmd_F_Clr_ExceptDir pCmdExceptDir == NULLL \n"));
		return STATUS_INVALID_PARAMETER;
	}

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return STATUS_UNSUCCESSFUL;
	}

	ExAcquireFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	pListNode = g_MalwFind.DrvFile.ListHead_ExceptDir.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvFile.ListHead_ExceptDir)
	{
		pDirObj   = (PCONTROL_OBJ)pListNode;
		pListNode = pListNode->Flink;

		if(pDirObj && pDirObj->Obj.Buffer)	
		{			
			RemoveEntryList( (PLIST_ENTRY)pDirObj  );
			ControlObjDelete( pDirObj );			
		}	
	}	
	ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	return STATUS_SUCCESS;
}



// FILE �������μ��� ����
NTSTATUS 
DrvCmd_F_Set_ExceptProc(IN PVOID pInBuffer)
{
	BOOLEAN       bExist      = FALSE;
	ULONG         ulLength    = 0;
	PCONTROL_OBJ  pNewProcObj = NULL;
	PDRVCMD_ExceptFileProc  pCmdExceptProc = NULL;

	pCmdExceptProc = (PDRVCMD_ExceptFileProc)pInBuffer;
	if(!pCmdExceptProc) return STATUS_INVALID_PARAMETER;

	ulLength = (ULONG)wcslen( pCmdExceptProc->wzExceptProc );
	if(ulLength < 2) return STATUS_UNSUCCESSFUL;

	bExist = ISExist_F_ExceptProc( pCmdExceptProc->wzExceptProc );
	if(bExist)
	{
		return STATUS_SUCCESS;
	}
	
	pNewProcObj = ControlObjAdd( ulLength+2 );
	ASSERT( pNewProcObj && pNewProcObj->Obj.Buffer );
	if(!pNewProcObj || !pNewProcObj->Obj.Buffer) 
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	pNewProcObj->Obj.usLength = (USHORT)ulLength;
	RtlStringCchCopyNW( pNewProcObj->Obj.Buffer, pNewProcObj->Obj.usMaxLength, pCmdExceptProc->wzExceptProc, ulLength );	
	// ���μ����� ��ġ����Ѵ�.
	pNewProcObj->Obj.pObjPos = pNewProcObj->Obj.Buffer;

	ControlHeaderInit( pNewProcObj, &g_MalwFind.DrvFile.ListHead_ExceptProc, MFOBJ_PROCESS );
		
	return STATUS_SUCCESS;
}



// FILE �������μ��� ����
NTSTATUS 
DrvCmd_F_Clr_ExceptProc(IN PVOID pInBuffer)
{
	ULONG         ulLength  = 0;
	PLIST_ENTRY   pListNode = NULL;
	PCONTROL_OBJ  pProcObj  = NULL;
	PDRVCMD_ExceptFileProc  pCmdExceptProc = NULL;

	pCmdExceptProc = (PDRVCMD_ExceptFileProc)pInBuffer;
	if(!pCmdExceptProc) 
	{
		return STATUS_INVALID_PARAMETER;
	}

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return STATUS_UNSUCCESSFUL;
	}

	ExAcquireFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	pListNode = g_MalwFind.DrvFile.ListHead_ExceptProc.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvFile.ListHead_ExceptProc)
	{
		pProcObj  = (PCONTROL_OBJ)pListNode;
		pListNode = pListNode->Flink;

		if(pProcObj && pProcObj->Obj.Buffer)	
		{			
			RemoveEntryList( (PLIST_ENTRY)pProcObj  );
			ControlObjDelete( pProcObj );			
		}		
	}	
	ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	return STATUS_SUCCESS;
}



// FDD ����
NTSTATUS 
DrvCmd_F_Set_FddCtrl(IN PVOID pInBuffer)
{
	PDRVCMD_FddVolCtrl  pCmdFddCtrl = NULL;
	pCmdFddCtrl = (PDRVCMD_FddVolCtrl)pInBuffer;
	if(!pCmdFddCtrl) 
	{
		// KdPrint(("STATUS_INVALID_PARAMETER >> DrvCmd_F_Set_FddCtrl pCmdFddCtrl == NULLL \n"));
		return STATUS_INVALID_PARAMETER;
	}

	g_MalwFind.DrvFile.FddPolicy = pCmdFddCtrl->FddPolicy;	
	return STATUS_SUCCESS;
}

//�̵�������ġ ����
NTSTATUS 
DrvCmd_F_Set_UsbCtrl( IN PVOID pInBuffer )
{
	PDRVCMD_USBVolCtrl  pCmdUSBStorCtrl = NULL;
	pCmdUSBStorCtrl = (PDRVCMD_USBVolCtrl)pInBuffer;
	if(!pCmdUSBStorCtrl) 
	{
		// KdPrint(("STATUS_INVALID_PARAMETER >> DrvCmd_F_Set_UsbCtrl pCmdUSBStorCtrl == NULLL \n"));
		return STATUS_INVALID_PARAMETER;
	}

	g_MalwFind.DrvFile.UsbPolicy = pCmdUSBStorCtrl->UsbPolicy;
	return STATUS_SUCCESS;
}


// �����̸����� ����/����
NTSTATUS DrvCmd_F_Set_FileNameCtrl(IN PVOID pInBuffer)
{
	return STATUS_SUCCESS;
}

NTSTATUS DrvCmd_F_Clr_FileNameCtrl(IN PVOID pInBuffer)
{
	return STATUS_SUCCESS;
}

// TRUE: BlackList  FALSE: WhiteList
NTSTATUS 
DrvCmd_F_Set_FileExtCtrl( IN PVOID pInBuffer )
{
	ULONG         ulLength = 0;
	PCONTROL_OBJ  pNewFileExtObj  = NULL;
	PDRVCMD_FileExtCtrl  pCmdFileExtCtrl = NULL;

	pCmdFileExtCtrl = (PDRVCMD_FileExtCtrl)pInBuffer;
	if(!pCmdFileExtCtrl) 
	{
		// KdPrint(("STATUS_INVALID_PARAMETER >> DrvCmd_F_Set_FileExtCtrl pCmdFileExtCtrl == NULLL \n"));
		return STATUS_INVALID_PARAMETER;
	}
	
	// ��å����	
	g_MalwFind.DrvFile.FileExtPolicy = pCmdFileExtCtrl->FileExtPolicy;
	if(ISExist_F_FileExtCtrl( pCmdFileExtCtrl->FileExt ))
	{
		return STATUS_SUCCESS;
	}

	// ��å����Ʈ Add
	ulLength = (ULONG)wcslen( pCmdFileExtCtrl->FileExt );
	if(ulLength < 2) 
	{
		return STATUS_UNSUCCESSFUL;
	}

	pNewFileExtObj = ControlObjAdd( ulLength );
	if(!pNewFileExtObj || !pNewFileExtObj->Obj.Buffer) 
	{
		// ASSERT( pNewFileExtObj && pNewFileExtObj->Obj.Buffer );
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	pNewFileExtObj->Obj.usLength = (USHORT)ulLength;
	RtlStringCchCopyNW( pNewFileExtObj->Obj.Buffer, 
				        pNewFileExtObj->Obj.usMaxLength, 
						pCmdFileExtCtrl->FileExt, 
						ulLength   );
	// ����Ȯ���� ����Ѵ�.
	pNewFileExtObj->Obj.pObjPos = pNewFileExtObj->Obj.Buffer;
	

	// TRUE:  (BlackList) ����Ʈ ������ 
    // FALSE: (WhiteList) ����Ʈ�� ����� ������ ����
	g_MalwFind.DrvFile.bExtDeny = pCmdFileExtCtrl->bFileExtDeny;

	ControlHeaderInit( pNewFileExtObj, &g_MalwFind.DrvFile.ListHead_FileExt, MFOBJ_FILE );	

	return STATUS_SUCCESS;
}


//Ȯ���� ����
NTSTATUS 
DrvCmd_F_Clr_FileExtCtrl( IN PVOID pInBuffer )
{
	ULONG         ulLength    = 0;
	PLIST_ENTRY   pListNode   = NULL;
	PCONTROL_OBJ  pFileExtObj = NULL;
	PDRVCMD_FileExtCtrl pCmdFileExtCtrl = NULL;

	pCmdFileExtCtrl = (PDRVCMD_FileExtCtrl)pInBuffer;
	if(!pCmdFileExtCtrl) return STATUS_INVALID_PARAMETER;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) return STATUS_UNSUCCESSFUL;

	// Ȯ���� ��å Ŭ����
	g_MalwFind.DrvFile.FileExtPolicy.bLog = LOG_OFF;
	g_MalwFind.DrvFile.FileExtPolicy.ulAcctl = MALWF_ACCESS;
	g_MalwFind.DrvFile.FileExtPolicy.Vol.ulDrives = ZERO_VOLUME;

	// 
	// ������ ���� ������.
	ExAcquireFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	
	// Ȯ�������� ���ܸ���Ʈ ����
	pListNode = g_MalwFind.DrvFile.ListHead_FileExt.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvFile.ListHead_FileExt)
	{
		pFileExtObj = (PCONTROL_OBJ)pListNode;
		pListNode   = pListNode->Flink;
		if(pFileExtObj && pFileExtObj->Obj.Buffer)	
		{			
			RemoveEntryList( (PLIST_ENTRY)pFileExtObj  );
			ControlObjDelete( pFileExtObj );			
		}		
	}

	ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	return STATUS_SUCCESS;

}


//�������� ����
NTSTATUS 
DrvCmd_SF_Set_SFolderCtrl( IN PVOID pInBuffer )
{
	BOOLEAN       bExist   = FALSE;
	ULONG         ulLength = 0;
	PCONTROL_OBJ  pNewSFolderObj  = NULL;
	PDRVCMD_SFolderCtrl  pCmdSFolderCtrl = NULL;

	pCmdSFolderCtrl = (PDRVCMD_SFolderCtrl)pInBuffer;
	if(!pCmdSFolderCtrl) return STATUS_INVALID_PARAMETER;

	// ��å����	
	g_MalwFind.DrvSFolder.Policy = pCmdSFolderCtrl->Policy;
	return STATUS_SUCCESS;

}


//�������� ����
NTSTATUS DrvCmd_SF_Clr_SFolderCtrl( IN PVOID pInBuffer )
{
	ULONG         ulLength    = 0;
	PLIST_ENTRY   pListNode   = NULL;
	PCONTROL_OBJ  pSFolderObj = NULL;
	PDRVCMD_SFolderCtrl pCmdSFolderCtrl = NULL;

	pCmdSFolderCtrl = (PDRVCMD_SFolderCtrl)pInBuffer;
	if(!pCmdSFolderCtrl) return STATUS_INVALID_PARAMETER;

	// ������å Ŭ����
	g_MalwFind.DrvSFolder.Policy.Vol.ulDrives = ZERO_VOLUME;
	g_MalwFind.DrvSFolder.Policy.bLog         = LOG_OFF;
	g_MalwFind.DrvSFolder.Policy.ulAcctl      = MALWF_ACCESS;

	return STATUS_SUCCESS;
}



//�������� �������� ����
NTSTATUS 
DrvCmd_SF_Set_SFolderExceptDir( IN PVOID pInBuffer )
{
	BOOLEAN             bExist        = FALSE;
	ULONG               ulLength      = 0;
	PCONTROL_OBJ        pNewDirObj    = NULL;
	PDRVCMD_ExceptFileDir pCmdExceptDir = NULL;

	pCmdExceptDir = (PDRVCMD_ExceptFileDir)pInBuffer;
	if(!pCmdExceptDir) return STATUS_INVALID_PARAMETER;

	ulLength = (ULONG)wcslen( pCmdExceptDir->wzExceptDir );
	if(ulLength < 1) return STATUS_UNSUCCESSFUL;


	bExist = ISExist_SF_SFolderExceptDir( pCmdExceptDir->wzExceptDir );
	if(bExist) return STATUS_SUCCESS;


	pNewDirObj = ControlObjAdd( ulLength );
	ASSERT( pNewDirObj && pNewDirObj->Obj.Buffer );
	if(!pNewDirObj || !pNewDirObj->Obj.Buffer) return STATUS_INSUFFICIENT_RESOURCES;

	pNewDirObj->Obj.usLength = (USHORT)ulLength;
	RtlStringCchCopyNW( pNewDirObj->Obj.Buffer, pNewDirObj->Obj.usMaxLength, pCmdExceptDir->wzExceptDir, ulLength );

	// ������ ��ġ �����ϱ�
	pNewDirObj->Obj.pObjPos = wcsrchr( pNewDirObj->Obj.Buffer, L'\\' );
	pNewDirObj->Obj.pObjPos++;

	ControlHeaderInit( pNewDirObj, &g_MalwFind.DrvSFolder.ListHead_ExceptDir, MFOBJ_DIR );	
	
	return STATUS_SUCCESS;

}


//�������� �������� ����
NTSTATUS 
DrvCmd_SF_Clr_SFolderExceptDir( IN PVOID pInBuffer )
{
	ULONG         ulLength  = 0;
	PLIST_ENTRY   pListNode = NULL;
	PCONTROL_OBJ  pDirObj   = NULL;
	PDRVCMD_ExceptFileDir  pCmdExceptDir = NULL;

	pCmdExceptDir = (PDRVCMD_ExceptFileDir)pInBuffer;
	if(!pCmdExceptDir) return STATUS_INVALID_PARAMETER;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return STATUS_UNSUCCESSFUL;
	}


	ExAcquireFastMutex( &g_MalwFind.DrvSFolder.LockSFolder );
	pListNode = g_MalwFind.DrvSFolder.ListHead_ExceptDir.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvSFolder.ListHead_ExceptDir)
	{
		pDirObj   = (PCONTROL_OBJ)pListNode;
		pListNode = pListNode->Flink;

		if(pDirObj && pDirObj->Obj.Buffer)	
		{			
			RemoveEntryList( (PLIST_ENTRY)pDirObj  );
			ControlObjDelete( pDirObj );			
			pDirObj = NULL;
		}		
	}	
	ExReleaseFastMutex( &g_MalwFind.DrvSFolder.LockSFolder );
	return STATUS_SUCCESS;
}



//�������� �������μ��� ����
NTSTATUS 
DrvCmd_SF_Set_SFolderExceptProc( IN PVOID pInBuffer )
{
	BOOLEAN       bExist      = FALSE;
	ULONG         ulLength    = 0;
	PCONTROL_OBJ  pNewProcObj = NULL;
	PDRVCMD_ExceptFileProc  pCmdExceptProc = NULL;

	pCmdExceptProc = (PDRVCMD_ExceptFileProc)pInBuffer;
	if(!pCmdExceptProc) return STATUS_INVALID_PARAMETER;

	ulLength = (ULONG)wcslen( pCmdExceptProc->wzExceptProc );
	if(ulLength < 1) return STATUS_UNSUCCESSFUL;

	bExist = ISExist_SF_SFolderExceptProc( pCmdExceptProc->wzExceptProc );
	if(bExist)
	{
		return STATUS_SUCCESS;
	}

	pNewProcObj = ControlObjAdd( ulLength );
	ASSERT( pNewProcObj && pNewProcObj->Obj.Buffer );
	if(!pNewProcObj || !pNewProcObj->Obj.Buffer) return STATUS_INSUFFICIENT_RESOURCES;

	pNewProcObj->Obj.usLength = (USHORT)ulLength;
	RtlStringCchCopyNW( pNewProcObj->Obj.Buffer, pNewProcObj->Obj.usMaxLength, pCmdExceptProc->wzExceptProc, ulLength );	

	// ���μ����� ��ġ����Ѵ�.
	pNewProcObj->Obj.pObjPos = pNewProcObj->Obj.Buffer;

	ControlHeaderInit( pNewProcObj, &g_MalwFind.DrvSFolder.ListHead_ExceptProc, MFOBJ_PROCESS );

	return STATUS_SUCCESS;
}



//�������� �������μ��� ����		
NTSTATUS 
DrvCmd_SF_Clr_SFolderExceptProc( IN PVOID pInBuffer )
{
	ULONG         ulLength  = 0;
	PLIST_ENTRY   pListNode = NULL;
	PCONTROL_OBJ  pProcObj  = NULL;
	PDRVCMD_ExceptFileProc  pCmdExceptProc = NULL;

	pCmdExceptProc = (PDRVCMD_ExceptFileProc)pInBuffer;
	if(!pCmdExceptProc) return STATUS_INVALID_PARAMETER;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return STATUS_UNSUCCESSFUL;
	}
	
	ExAcquireFastMutex( &g_MalwFind.DrvSFolder.LockSFolder );
	pListNode = g_MalwFind.DrvSFolder.ListHead_ExceptProc.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvSFolder.ListHead_ExceptProc)
	{
		pProcObj  = (PCONTROL_OBJ)pListNode;
		pListNode = pListNode->Flink;

		if(pProcObj && pProcObj->Obj.Buffer)	
		{			
			RemoveEntryList( (PLIST_ENTRY)pProcObj  );
			ControlObjDelete( pProcObj );			
		}
	}	
	ExReleaseFastMutex( &g_MalwFind.DrvSFolder.LockSFolder );
	return STATUS_SUCCESS;
}


//PROCESS �������� ����
NTSTATUS 
DrvCmd_P_Set_ExceptDir( IN PVOID pInBuffer )
{
	BOOLEAN              bExist        = FALSE;
	ULONG                ulLength      = 0;
	PCONTROL_OBJ         pNewDirObj    = NULL;
	PDRVCMD_ExceptFileDir  pCmdExceptDir = NULL;

	pCmdExceptDir = (PDRVCMD_ExceptFileDir)pInBuffer;
	if(!pCmdExceptDir) return STATUS_INVALID_PARAMETER;

	// ��å����Ʈ Add
	ulLength = (ULONG)wcslen( pCmdExceptDir->wzExceptDir );
	if(ulLength <= 0) 
	{
		return STATUS_UNSUCCESSFUL;
	}

	// ���� ��尡 �ִ��� �˻��Ѵ�.
	bExist = ISExist_P_ExceptDir_Equal( pCmdExceptDir->wzExceptDir );
	if(bExist)
	{
		return STATUS_SUCCESS;
	}
	
	pNewDirObj = ControlObjAdd( ulLength );
	if(!pNewDirObj || !pNewDirObj->Obj.Buffer) 
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pNewDirObj->Obj.usLength = (USHORT)ulLength;
	RtlStringCchCopyNW( pNewDirObj->Obj.Buffer, 
				        pNewDirObj->Obj.usMaxLength, 
						pCmdExceptDir->wzExceptDir, 
						ulLength   );
	
	// �������� ������ġ ����Ѵ�.
	pNewDirObj->Obj.pObjPos = wcsrchr( pNewDirObj->Obj.Buffer, L'\\' );
	pNewDirObj->Obj.pObjPos++;
		
	ControlHeaderInit( pNewDirObj, &g_MalwFind.DrvProc.ListHead_ExceptDir, MFOBJ_DIR );	
	
	return STATUS_SUCCESS;
}



//PROCESS �������� ����
NTSTATUS 
DrvCmd_P_Clr_ExceptDir( IN PVOID pInBuffer )
{
	PLIST_ENTRY   pListNode = NULL;
	PCONTROL_OBJ  pDirObj   = NULL;
	PDRVCMD_ExceptProcDir pCmdExceptDir = NULL;

	pCmdExceptDir = (PDRVCMD_ExceptProcDir)pInBuffer;
	if(!pCmdExceptDir) return STATUS_INVALID_PARAMETER;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return STATUS_UNSUCCESSFUL;
	}

	ExAcquireFastMutex( &g_MalwFind.DrvProc.LockProc );
	pListNode = g_MalwFind.DrvProc.ListHead_ExceptDir.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvProc.ListHead_ExceptDir)
	{
		pDirObj   = (PCONTROL_OBJ)pListNode;
		pListNode = pListNode->Flink;

		if(pDirObj && pDirObj->Obj.Buffer)	
		{			
			RemoveEntryList( (PLIST_ENTRY)pDirObj  );
			ControlObjDelete( pDirObj );			
			pDirObj = NULL;
		}		
	}	
	ExReleaseFastMutex( &g_MalwFind.DrvProc.LockProc );

	return STATUS_SUCCESS;
}



//PROCESS ���� ����
NTSTATUS 
DrvCmd_P_Set_ProcessCtrl( IN PVOID pInBuffer )
{
	BOOLEAN           bExist = FALSE;
	ULONG             ulLength = 0;
	PCONTROL_OBJ      pNewProcObj = NULL;
	PDRVCMD_ProcCtrl  pCmdProcCtrl = NULL;

	pCmdProcCtrl = (PDRVCMD_ProcCtrl)pInBuffer;
	if (!pCmdProcCtrl)
	{
		return STATUS_INVALID_PARAMETER;
	}

	// ��å����Ʈ Add
	ulLength = (ULONG)wcslen( pCmdProcCtrl->Proc );
	if(ulLength < 2) 
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	g_MalwFind.DrvProc.Policy = pCmdProcCtrl->Policy;

	bExist = ISExist_P_ProcCtrl( pCmdProcCtrl->Proc );
	if(bExist)
	{
		return STATUS_SUCCESS;
	}

	pNewProcObj = ControlObjAdd( ulLength );
	if(!pNewProcObj || !pNewProcObj->Obj.Buffer)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pNewProcObj->Obj.usLength = (USHORT)ulLength;
	RtlStringCchCopyNW( pNewProcObj->Obj.Buffer, pNewProcObj->Obj.usMaxLength, pCmdProcCtrl->Proc, ulLength );
	
    // ���μ��� ���� ��ġ �������� ������ġ ����Ѵ�.
	pNewProcObj->Obj.pObjPos = pNewProcObj->Obj.Buffer;

	// TRUE:  (BlackList) ����Ʈ ������ 
    // FALSE: (WhiteList) ����Ʈ�� ����� ������ ����
	g_MalwFind.DrvProc.bProcDeny = pCmdProcCtrl->bProcDeny;
	
	ControlHeaderInit( pNewProcObj, &g_MalwFind.DrvProc.ListHead_ProcCtrl, MFOBJ_PROCESS );	
	
	return STATUS_SUCCESS;

}



//PROCESS ���� ����
NTSTATUS 
DrvCmd_P_Clr_ProcessCtrl( IN PVOID pInBuffer )
{
	ULONG         ulLength  = 0;
	PLIST_ENTRY   pListNode = NULL;
	PCONTROL_OBJ  pProcObj  = NULL;
	PDRVCMD_ProcCtrl pCmdProcCtrl = NULL;

	pCmdProcCtrl = (PDRVCMD_ProcCtrl)pInBuffer;
	if(!pCmdProcCtrl) return STATUS_INVALID_PARAMETER;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return STATUS_UNSUCCESSFUL;
	}

	ExAcquireFastMutex( &g_MalwFind.DrvProc.LockProc );
	pListNode = g_MalwFind.DrvProc.ListHead_ProcCtrl.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvProc.ListHead_ProcCtrl)
	{
		pProcObj  = (PCONTROL_OBJ)pListNode;
		pListNode = pListNode->Flink;

		if(pProcObj && pProcObj->Obj.Buffer)	
		{			
			RemoveEntryList( (PLIST_ENTRY)pProcObj  );
			ControlObjDelete( pProcObj );			
			pProcObj = NULL;
		}
	}
	ExReleaseFastMutex( &g_MalwFind.DrvProc.LockProc );
	return STATUS_SUCCESS;

}

// ���μ��� �ݹ� -- ���μ��� ��� �״°� ������ ����
NTSTATUS 
DrvCmd_P_Get_Process_Stat( IN  PVOID  pInBuffer, 
						   OUT ULONG  ulInBufferLength, 
						   OUT PVOID  pOutBuffer, 
						   IN  ULONG  ulOutBufferLength,
						   OUT PIO_STATUS_BLOCK  pIoStatus )
{
	char*  pczOutBuffer = NULL;
	NTSTATUS Status = STATUS_SUCCESS;
	
	Status = Drv_ProcLC_FetchData( (char*)pOutBuffer, &ulInBufferLength, ulOutBufferLength );
	if(NT_SUCCESS(Status))
	{
		if(pIoStatus) pIoStatus->Information = ulInBufferLength;
	}

	return STATUS_SUCCESS;
}



// �Ǽ��ڵ�м�
NTSTATUS DrvCmd_MalwFind_SetPolicyReg(IN PVOID pInBuffer)
{
	return STATUS_SUCCESS;
}

NTSTATUS DrvCmd_MalwFind_SetPolicyFile(IN PVOID pInBuffer)
{
	return STATUS_SUCCESS;
}

NTSTATUS DrvCmd_MalwFind_SetPolicyNetwork(IN PVOID pInBuffer)
{
	return STATUS_SUCCESS;
}

NTSTATUS DrvCmd_MalwFind_SetPolicyJobStart(IN PVOID pInBuffer)
{
	return STATUS_SUCCESS;
}


// DeviceIoControl ������ �κ�
NTSTATUS
FltDrv_DeviceControl( IN PFILE_OBJECT    pFileObject, 
				      IN PDEVICE_OBJECT  pDeviceObject,
				      IN ULONG   ulIoCtrlCode, 
					  IN PVOID   pInBuf,
				      IN ULONG   ulInBufLength, 
				      OUT PVOID  pOutBuf, 
				      IN ULONG   ulOutBufLength,
					  OUT PIO_STATUS_BLOCK  pOutIoStatus )
{
	NTSTATUS Status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER( pFileObject || pDeviceObject );
	
	if(FALSE == g_MalwFind.bEntryInit)
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	__try
	{
		switch(ulIoCtrlCode) 
		{		
		//����̹� ����
		case IOCTL_MALWF_GET_VERSION: return DrvCmd_GetVersion();
		//�α� ���� ����/����
		case IOCTL_MALWF_LOG_START: return DrvCmd_LogStart(pInBuf);
		//�α� Fetch �ϱ� 
		case IOCTL_MALWF_GET_LOGFETCH: return DrvCmd_GetLogFetch( pOutBuf, ulOutBufLength, pOutIoStatus );

		// ��ġ ���丮 ����
		case IOCTL_MALWF_SET_SETUPDIRCTRL: return DrvCmd_SetupDirControl(pInBuf);
		// ��ü ��������
		case IOCTL_MALWF_SET_SETUPDIR: return DrvCmd_Set_SetupDir(pInBuf);
					
		//FILE �������� ����
		case IOCTL_MALWF_SET_EXCEPT_FILEDIR:  return DrvCmd_F_Set_ExceptDir(pInBuf);
		//FILE �������� ����
		case IOCTL_MALWF_CLR_EXCEPT_FILEDIR:  return DrvCmd_F_Clr_ExceptDir(pInBuf);
		//FILE �������μ��� ����
		case IOCTL_MALWF_SET_EXCEPT_FILEPROC: return DrvCmd_F_Set_ExceptProc(pInBuf);
		//FILE �������μ��� ����
		case IOCTL_MALWF_CLR_EXCEPT_FILEPROC: return DrvCmd_F_Clr_ExceptProc(pInBuf);

		// �����̸����� ����/����
		case IOCTL_MALWF_SET_FILENAMECTRL: return DrvCmd_F_Set_FileNameCtrl(pInBuf);
		case IOCTL_MALWF_CLR_FILENAMECTRL: return DrvCmd_F_Clr_FileNameCtrl(pInBuf);
			
		//FDD ����� //�̵�������ġ �����
		case IOCTL_MALWF_SET_FDDVOLCTRL: return DrvCmd_F_Set_FddCtrl(pInBuf);
		case IOCTL_MALWF_SET_USBVOLCTRL: return DrvCmd_F_Set_UsbCtrl(pInBuf);
		
		//Ȯ�������� ����/����
		case IOCTL_MALWF_SET_FILEEXTCTRL: return DrvCmd_F_Set_FileExtCtrl(pInBuf);
		case IOCTL_MALWF_CLR_FILEEXTCTRL: return DrvCmd_F_Clr_FileExtCtrl(pInBuf);
		
		//������������ ����/����
		case IOCTL_MALWF_SET_SFCTRL: return DrvCmd_SF_Set_SFolderCtrl(pInBuf);
		case IOCTL_MALWF_CLR_SFCTRL: return DrvCmd_SF_Clr_SFolderCtrl(pInBuf);
		//�������� �������� ����
		case IOCTL_MALWF_SET_EXCEPT_SFDIR:  return DrvCmd_SF_Set_SFolderExceptDir(pInBuf);
		case IOCTL_MALWF_CLR_EXCEPT_SFDIR:  return DrvCmd_SF_Clr_SFolderExceptProc(pInBuf);
		//�������� �������μ��� ����
		case IOCTL_MALWF_SET_EXCEPT_SFPROC: return DrvCmd_SF_Set_SFolderExceptProc(pInBuf);
		case IOCTL_MALWF_CLR_EXCEPT_SFPROC: return DrvCmd_SF_Clr_SFolderExceptProc(pInBuf);
					
		//���μ��� �ݹ� -- ���μ��� ���/�ױ�
		case IOCTL_MALWF_GET_PROC_STAT:
			return DrvCmd_P_Get_Process_Stat( pInBuf, ulInBufLength, pOutBuf, ulOutBufLength, pOutIoStatus   );

		// ���μ��� �������� ����/����
		case IOCTL_MALWF_SET_EXCEPT_PROCDIR:   return DrvCmd_P_Set_ExceptDir(pInBuf);
		case IOCTL_MALWF_CLR_EXCEPT_PROCDIR:   return DrvCmd_P_Clr_ExceptDir(pInBuf);
		// ���μ��� ���� ����/����
		case IOCTL_MALWF_SET_PROC_CTRL: return DrvCmd_P_Set_ProcessCtrl(pInBuf);
		case IOCTL_MALWF_CLR_PROC_CTRL: return DrvCmd_P_Clr_ProcessCtrl(pInBuf);

		// �Ǽ��ڵ�м�
		case IOCTL_MALWF_POLICY_REG:    return DrvCmd_MalwFind_SetPolicyReg(pInBuf);
		case IOCTL_MALWF_POLICY_FILE:   return DrvCmd_MalwFind_SetPolicyFile(pInBuf);
		case IOCTL_MALWF_POLICY_NETWORK:return DrvCmd_MalwFind_SetPolicyNetwork(pInBuf);
		case IOCTL_MALWF_ANALYSIS_START:return DrvCmd_MalwFind_SetPolicyJobStart(pInBuf);
		default:  break;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		Status = STATUS_UNSUCCESSFUL;
		KdPrint(("\nException Occured \n\n"));
	}
	return Status;
}