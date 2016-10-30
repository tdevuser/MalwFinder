#include "MalwFind_DataTypes.h"
#include "ntddscsi.h"
#include "ntddcdrm.h"
#include "Ntddcdvd.h"
#include "Ntdddisk.h"
#include "scsi.h"
#include "ntddser.h"
#include "usb.h"
#include "Srb.h"
#include "MalwFind.h"
#include "MalwFind_Command.h"
#include "MalwFind_Acctl.h"
#include "MalwFind_LogFunc.h"
#include "MalwFind_Util.h"
#include "MalwFind_DispHook.h"


extern G_MALW_FIND  g_MalwFind;


#define IS_SCSIOP_WRITE(opCode) \
   ((opCode == SCSIOP_WRITE6) || (opCode == SCSIOP_WRITE) || (opCode == SCSIOP_WRITE12) || \
   (opCode == SCSIOP_WRITE16) || (opCode == 0x10) || (opCode == 0x20) || (opCode == SCSIOP_MEDIUM_REMOVAL))


void FltDrvDispHook_Init( IN PFLT_DRV_CDCTRL pCDCtrl )
{
	ASSERT( pCDCtrl );
	if(!pCDCtrl) return;

	RtlZeroMemory( pCDCtrl, sizeof(FLT_DRV_CDCTRL) );
	ExInitializeFastMutex( &pCDCtrl->LockCdromCtrl );

	// 버닝제어 // CDROM 제어
	pCDCtrl->CdCtrlPolicy.ulAcctl  = MALWF_ACCESS;  /// MALWF_CREATE || MALWF_READ || MALWF_WRITE 
	pCDCtrl->CdCtrlPolicy.bLog     = LOG_OFF;
	pCDCtrl->CdCtrlPolicy.Vol.ulDrives = ZERO_VOLUME;
	
	// DriverHook
	pCDCtrl->DriverHook.bCdom_Hook    = FALSE;
	pCDCtrl->DriverHook.bCdromUF_Hook = FALSE;
	pCDCtrl->DriverHook.bCdromLF_Hook = FALSE;

}


void FltDrvDispHook_UnInit( IN PFLT_DRV_CDCTRL pCDCtrl )
{
	ASSERT( pCDCtrl );
	if(!pCDCtrl) return;

	PreDispatch_Device_UnHook();

	RtlZeroMemory( &pCDCtrl->DrvCdrom,    sizeof(DP_FUNC) );	
	RtlZeroMemory( &pCDCtrl->DrvCdrom_UF, sizeof(DP_FUNC) );	
	RtlZeroMemory( &pCDCtrl->DrvCdrom_LF, sizeof(DP_FUNC) );	

	// 버닝제어 // CDROM 제어
	pCDCtrl->CdCtrlPolicy.ulAcctl  = MALWF_ACCESS;  /// MALWF_CREATE || MALWF_READ || MALWF_WRITE 
	pCDCtrl->CdCtrlPolicy.bLog     = LOG_OFF;
	pCDCtrl->CdCtrlPolicy.Vol.ulDrives = ZERO_VOLUME;

}


// /******************************************************************************************************************
// PFLT_DRV_CDCTRL
// /******************************************************************************************************************

void PreDispatch_Device_Hook(void)
{	

	// Cdrom
	if(!g_MalwFind.DrvCdCrtl.DriverHook.bCdom_Hook)
	{
		Hook_Cdrom();
	}

	// Cdrom UF Imapi
	if(!g_MalwFind.DrvCdCrtl.DriverHook.bCdromUF_Hook)
	{
		Hook_Cdrom_UF();
	}

	// Cdrom LF PxHelp20
	if(!g_MalwFind.DrvCdCrtl.DriverHook.bCdromLF_Hook)
	{
		Hook_Cdrom_LF();
	}

}


// ControlDevice Dispatch UnHook
void PreDispatch_Device_UnHook(void)
{
	
	UnHook_Cdrom_LF();

	UnHook_Cdrom_UF();

	UnHook_Cdrom();

}


// Unload Dispatch sethook
void Hook_Clr_Dispatch_Unload( IN PDRIVER_OBJECT  pDriverObject, IN PDP_FUNC pDpFunc )
{
	ASSERT( pDriverObject && pDpFunc );
	if(!pDriverObject || !pDpFunc) return;

	// IRP_MJ_CREATE
	if(pDpFunc->pCreate)
	{
		pDriverObject->MajorFunction[ IRP_MJ_CREATE ] = pDpFunc->pCreate;
		pDpFunc->pCreate = NULL;
	}
	// IRP_MJ_READ
	if(pDpFunc->pRead)
	{
		pDriverObject->MajorFunction[ IRP_MJ_CREATE ] = pDpFunc->pRead;
		pDpFunc->pRead = NULL;
	}

	// IRP_MJ_WRITE
	if(pDpFunc->pWrite)
	{
		pDriverObject->MajorFunction[ IRP_MJ_CREATE ] = pDpFunc->pWrite;
		pDpFunc->pWrite = NULL;
	}

	// IRP_MJ_DEVICE_CONTROL
	if(pDpFunc->pDevControl)
	{
		pDriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL ] = pDpFunc->pDevControl;
		pDpFunc->pDevControl = NULL;
	}

	// IRP_MJ_INTERNAL_DEVICE_CONTROL
	if(pDpFunc->pIntDevControl)
	{
		pDriverObject->MajorFunction[ IRP_MJ_INTERNAL_DEVICE_CONTROL ] = pDpFunc->pIntDevControl;
		pDpFunc->pIntDevControl = NULL;
	}

	// DriverUnload PDRIVER_UNLOAD
	if(pDpFunc->pDriverUnload)
	{
		pDriverObject->DriverUnload = pDpFunc->pDriverUnload;
		pDpFunc->pDriverUnload = NULL;
	}

}



// Dispatch Unload Function unhook
BOOLEAN  
Hook_Set_Dispatch_Unload( IN PDRIVER_OBJECT    pDriverObject, 
	 					  IN OUT PDP_FUNC      pDpFunc, 
						  IN PDRIVER_DISPATCH  pHookDriverDispatch,
						  IN PDRIVER_UNLOAD    pHookDriverUnload    )
{
	ASSERT( pDriverObject && pDpFunc && pHookDriverDispatch && pHookDriverUnload );
	if(!pDriverObject || !pDpFunc || !pHookDriverDispatch || !pHookDriverUnload )
	{
		KdPrint(("\nHook_Set_Dispatch_Unload Fail."));
		return FALSE; 
	}

	pDpFunc->pCreate        = pDriverObject->MajorFunction[ IRP_MJ_CREATE ];
	pDpFunc->pRead          = pDriverObject->MajorFunction[ IRP_MJ_READ   ];
	pDpFunc->pWrite         = pDriverObject->MajorFunction[ IRP_MJ_WRITE  ];
	pDpFunc->pDevControl    = pDriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL ];
	pDpFunc->pIntDevControl = pDriverObject->MajorFunction[ IRP_MJ_INTERNAL_DEVICE_CONTROL ];

	// DriverUnload
	pDpFunc->pDriverUnload  = pDriverObject->DriverUnload;

	// DirverUNLOAD
	pDriverObject->DriverUnload = pHookDriverUnload;

	pDriverObject->MajorFunction[ IRP_MJ_CREATE ] = pHookDriverDispatch;
	pDriverObject->MajorFunction[ IRP_MJ_READ   ] = pHookDriverDispatch;
	pDriverObject->MajorFunction[ IRP_MJ_WRITE  ] = pHookDriverDispatch;
	pDriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL ] = pHookDriverDispatch;
	pDriverObject->MajorFunction[ IRP_MJ_INTERNAL_DEVICE_CONTROL ] =  pHookDriverDispatch;
  
	KdPrint(("\nHook_Set_Dispatch_Unload Success."));
	return TRUE;

}


// /**********************************************************************/
//  Hook_Cdrom
// /**********************************************************************/

BOOLEAN 
Hook_Cdrom(void)
{
	BOOLEAN         bSuc = FALSE;
	PDRIVER_OBJECT  pDriverObject = NULL;
	UNICODE_STRING  CdromName;
	
	if(g_MalwFind.DrvCdCrtl.DriverHook.bCdom_Hook) 
	{
		return TRUE;
	}	

	RtlInitUnicodeString( &CdromName, DRIVER_CDROM );
	pDriverObject = FindDriverObject( &CdromName );
	if(!pDriverObject) 
	{
		return FALSE;
	}
	bSuc = Hook_Set_Dispatch_Unload( pDriverObject, &g_MalwFind.DrvCdCrtl.DrvCdrom, PreHook_Cdrom_Dispatch, PreHook_Cdrom_DriverUnload );
	if(bSuc) 
	{
		KdPrint((" \nHook_Cdrom Success"));
	}
	g_MalwFind.DrvCdCrtl.DriverHook.bCdom_Hook = bSuc;
	return bSuc;
}


// /**********************************************************************/
//  UnHook_Cdrom
// /**********************************************************************/

BOOLEAN 
UnHook_Cdrom(void)
{
	PDRIVER_OBJECT  pDriverObject = NULL;
	UNICODE_STRING  CdromName;

	if(!g_MalwFind.DrvCdCrtl.DriverHook.bCdom_Hook) 
	{
		return TRUE;
	}	

	RtlInitUnicodeString( &CdromName, DRIVER_CDROM );
	pDriverObject = FindDriverObject( &CdromName );
	if(!pDriverObject) 
	{
		return FALSE;
	}
	Hook_Clr_Dispatch_Unload( pDriverObject, &g_MalwFind.DrvCdCrtl.DrvCdrom );
	g_MalwFind.DrvCdCrtl.DriverHook.bCdom_Hook = FALSE;
	return TRUE;
}


// /**********************************************************************/
//  Hook_Cdrom_UF
// /**********************************************************************/

BOOLEAN 
Hook_Cdrom_UF(void)
{
	BOOLEAN         bSuc = FALSE;
	PDRIVER_OBJECT  pDriverObject = NULL;
	UNICODE_STRING  CdromUFName;
	
	if(g_MalwFind.DrvCdCrtl.DriverHook.bCdromUF_Hook) 
	{
		return TRUE;
	}	

	RtlInitUnicodeString( &CdromUFName, DRIVER_IMAPI );
	pDriverObject = FindDriverObject( &CdromUFName );
	if(!pDriverObject) 
	{
		RtlInitUnicodeString( &CdromUFName, DRIVER_IMAPI_2 );
		pDriverObject = FindDriverObject( &CdromUFName );
		if(!pDriverObject)
		{
			return FALSE;
		}
	}

	bSuc = Hook_Set_Dispatch_Unload( pDriverObject, &g_MalwFind.DrvCdCrtl.DrvCdrom_UF, PreHook_Cdrom_UF_Dispatch, PreHook_Cdrom_UF_DriverUnload );
	if(bSuc) 
	{
		KdPrint((" \nHook_Cdrom_UF Success."));
	}

	g_MalwFind.DrvCdCrtl.DriverHook.bCdromUF_Hook = bSuc;
	return bSuc;
}


// /**********************************************************************/
//  UnHook_Cdrom_UF
// /**********************************************************************/

BOOLEAN 
UnHook_Cdrom_UF(void)
{
	PDRIVER_OBJECT  pDriverObject = NULL;
	UNICODE_STRING  CdromUFName;

	if(!g_MalwFind.DrvCdCrtl.DriverHook.bCdromUF_Hook) 
	{
		return TRUE;
	}
	
	RtlInitUnicodeString( &CdromUFName, DRIVER_IMAPI );
	pDriverObject = FindDriverObject( &CdromUFName );
	if(!pDriverObject) 
	{
		RtlInitUnicodeString( &CdromUFName, DRIVER_IMAPI_2 );
		pDriverObject = FindDriverObject( &CdromUFName );
		if(!pDriverObject)
		{
			return FALSE;
		}
	}

	Hook_Clr_Dispatch_Unload( pDriverObject, &g_MalwFind.DrvCdCrtl.DrvCdrom_UF );
	g_MalwFind.DrvCdCrtl.DriverHook.bCdromUF_Hook = FALSE;
	return TRUE;
}


// /**********************************************************************/
//  Hook_Cdrom_LF
// /**********************************************************************/
BOOLEAN  
Hook_Cdrom_LF(void)
{
	BOOLEAN         bSuc = FALSE;
	PDRIVER_OBJECT  pDriverObject = NULL;
	UNICODE_STRING  CdromLFName;
	
	if(g_MalwFind.DrvCdCrtl.DriverHook.bCdromLF_Hook) 
	{
		return TRUE;
	}

	// Sonic
	RtlInitUnicodeString( &CdromLFName, DRIVER_PXHELP20 );
	pDriverObject = FindDriverObject( &CdromLFName );
	if(!pDriverObject) 
	{
		return FALSE;
	}
	bSuc = Hook_Set_Dispatch_Unload( pDriverObject, &g_MalwFind.DrvCdCrtl.DrvCdrom_LF, PreHook_Cdrom_LF_Dispatch, PreHook_Cdrom_LF_DriverUnload );
	if(bSuc)  
	{
		KdPrint((" \nHook_Cdrom_LF Success."));
	}
	g_MalwFind.DrvCdCrtl.DriverHook.bCdromLF_Hook = bSuc;
	return bSuc;

}

// /**********************************************************************/
//  UnHook_Cdrom_LF
// /**********************************************************************/

BOOLEAN 
UnHook_Cdrom_LF(void)
{
	PDRIVER_OBJECT  pDriverObject = NULL;
	UNICODE_STRING  CdromLFName;

	if(!g_MalwFind.DrvCdCrtl.DriverHook.bCdromLF_Hook) 
	{
		return TRUE;
	}

	// Sonnic
	RtlInitUnicodeString( &CdromLFName, DRIVER_PXHELP20 );
	pDriverObject = FindDriverObject( &CdromLFName );
	if(!pDriverObject)
	{
		return FALSE;
	}

	Hook_Clr_Dispatch_Unload( pDriverObject, &g_MalwFind.DrvCdCrtl.DrvCdrom_LF );
	g_MalwFind.DrvCdCrtl.DriverHook.bCdromLF_Hook = FALSE;
	return TRUE;
}


// 버닝제어 플래그
BOOLEAN  
ISDevAccess_CdromBurn( IN ULONG ulAccess )
{  
	BOOLEAN  bControl = FALSE;
	// CdromBurn Control
	bControl = (BOOLEAN)(g_MalwFind.DrvCdCrtl.CdCtrlPolicy.ulAcctl & ulAccess);
	return bControl;
}


// /**************************************************************************************************************************
//  Cdrom CD/DVD  Dispatch Hook Function  
// /**************************************************************************************************************************

NTSTATUS 
Control_Cdrom_SsciPassThrough( IN PVOID pBuffer )
{
	PSCSI_PASS_THROUGH  pPassThrough = NULL;
	
	pPassThrough = (PSCSI_PASS_THROUGH)pBuffer;
	if(!pPassThrough) return STATUS_INVALID_PARAMETER;

	if(!ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ) || pPassThrough->DataIn != SCSI_IOCTL_DATA_OUT) 
	{
		return STATUS_SUCCESS;
	}

	if(IS_SCSIOP_WRITE( pPassThrough->Cdb[0] ))
	{	
		return STATUS_ACCESS_DENIED;
	}
	return STATUS_SUCCESS;

}


NTSTATUS 
Control_Cdrom_SsciPassThroughDirect( IN PVOID pBuffer )
{
	PSCSI_PASS_THROUGH_DIRECT pDirect = NULL;
	
	pDirect = (PSCSI_PASS_THROUGH_DIRECT)pBuffer;
	if(!pDirect) return STATUS_INVALID_PARAMETER;

	if(!ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ) || pDirect->DataIn != SCSI_IOCTL_DATA_OUT) 
	{
		return STATUS_SUCCESS;
	}

	if(IS_SCSIOP_WRITE( pDirect->Cdb[0] ))
	{	
		return STATUS_ACCESS_DENIED;
	}
	return STATUS_SUCCESS;
}


// Cdrom
NTSTATUS 
PreHook_Cdrom_Dispatch( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
	NTSTATUS            Status        = STATUS_SUCCESS;
	PIO_STACK_LOCATION  pIrpStack     = NULL;
	PFILE_OBJECT        pFileObject   = NULL;
	ULONG               ulMajorFunc   = 0x00;
	ULONG               ulControlCode = 0x00;
	HANDLE              hProcessID    = 0;
	PSCSI_PASS_THROUGH_DIRECT  pScsiPassDirect = NULL;
	WCHAR               wzProcName[ MAX_PROCESS_LEN ] = L"Reserved";
	
	pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
	if(!pIrpStack) return CompleteRequest( pIrp, STATUS_INVALID_PARAMETER, 0 );
	
	pFileObject = pIrpStack->FileObject;
	ulMajorFunc = pIrpStack->MajorFunction;	
// *************************************************************************************//
	hProcessID = PsGetCurrentProcessId();
	GetProcessName( PsGetCurrentProcess(), wzProcName, MAX_PROCESS_LEN  );	
// *************************************************************************************//

	switch(ulMajorFunc)
	{
	case IRP_MJ_CREATE:
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom.pCreate) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;	
		}
		// KdPrint((" \nCdrom : IRP_MJ_CREATE"));
		if(ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_OPEN ))
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		Status = g_MalwFind.DrvCdCrtl.DrvCdrom.pCreate( pDeviceObject, pIrp );
		return Status;

	case IRP_MJ_READ:		
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom.pRead) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;			
		}
		// KdPrint((" \nCdrom : IRP_MJ_READ"));
		if(ISDevAccess_CdromBurn( MALWF_READ )) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		Status = g_MalwFind.DrvCdCrtl.DrvCdrom.pRead( pDeviceObject, pIrp );
		return Status;

	case IRP_MJ_WRITE:	
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom.pWrite)
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}		
		// KdPrint((" \nCdrom : IRP_MJ_WRITE"));
		if(ISDevAccess_CdromBurn( MALWF_WRITE )) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		Status = g_MalwFind.DrvCdCrtl.DrvCdrom.pWrite( pDeviceObject, pIrp );
		return Status;

	case IRP_MJ_DEVICE_CONTROL:	
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom.pDevControl) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		ulControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
		if(IOCTL_DISK_UPDATE_DRIVE_SIZE != ulControlCode)
		{
			// KdPrint(("Cdrom : IRP_MJ_DEVICE_CONTROL--ControlCode[0x%08x][ %s ] \n",
			//  	ulControlCode, GetCdromCtrlCode_Paring( ulControlCode,  pIrp->AssociatedIrp.SystemBuffer ) ));
		}

		if(ulControlCode == IOCTL_SCSI_PASS_THROUGH_DIRECT && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ))
		{			
			Status = Control_Cdrom_SsciPassThroughDirect( pIrp->AssociatedIrp.SystemBuffer );
			if(Status == STATUS_ACCESS_DENIED) break;
		}
		else if(ulControlCode == IOCTL_SCSI_PASS_THROUGH && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ))
		{
			Status = Control_Cdrom_SsciPassThrough( pIrp->AssociatedIrp.SystemBuffer );
			if(Status == STATUS_ACCESS_DENIED) break;
		}

		Status = g_MalwFind.DrvCdCrtl.DrvCdrom.pDevControl( pDeviceObject, pIrp );
		return Status;

	case IRP_MJ_INTERNAL_DEVICE_CONTROL:	
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom.pIntDevControl)
		{
			Status = STATUS_ACCESS_DENIED;
			break;				
		}			
		ulControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
		if(IOCTL_DISK_UPDATE_DRIVE_SIZE != ulControlCode)
		{
			KdPrint(("Cdrom : IRP_MJ_INTERNAL_DEVICE_CONTROL--ControlCode[0x%08x][ %s ] \n", 
				ulControlCode, GetCdromCtrlCode_Paring( ulControlCode,  pIrp->AssociatedIrp.SystemBuffer ) )); 
		}

		if( ulControlCode == IOCTL_SCSI_PASS_THROUGH_DIRECT && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ) )
		{
			Status = Control_Cdrom_SsciPassThroughDirect( pIrp->AssociatedIrp.SystemBuffer );
			if(Status == STATUS_ACCESS_DENIED) break;
		}
		else if(ulControlCode == IOCTL_SCSI_PASS_THROUGH && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ) )
		{	
			Status = Control_Cdrom_SsciPassThrough( pIrp->AssociatedIrp.SystemBuffer );
			if(Status == STATUS_ACCESS_DENIED) break;
		}

		Status = g_MalwFind.DrvCdCrtl.DrvCdrom.pIntDevControl( pDeviceObject, pIrp );		
		return Status;

	case IRP_MJ_PNP:
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom.pPnp)  
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		Status = g_MalwFind.DrvCdCrtl.DrvCdrom.pPnp( pDeviceObject, pIrp );
		return Status;
	}

	if(Status == STATUS_ACCESS_DENIED)
	{
		KdPrint(("Cdrom -- STATUS_ACCESS_DENIED  \n\n\n"));
		Status = CompleteRequest( pIrp, STATUS_ACCESS_DENIED, 0 );
		return Status;
	}
	return Status;
}


void PreHook_Cdrom_DriverUnload( IN PDRIVER_OBJECT pDriverObject )
{
	if(!pDriverObject || !g_MalwFind.DrvCdCrtl.DrvCdrom.pDriverUnload) return;

	//KdPrint(("Cdrom DriverUnload -- STATUS_ACCESS_DENIED  \n"));
	g_MalwFind.DrvCdCrtl.DrvCdrom.pDriverUnload( pDriverObject );
	g_MalwFind.DrvCdCrtl.DrvCdrom.pCreate   = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom.pRead     = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom.pWrite    = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom.pDevControl = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom.pIntDevControl = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom.pPnp  =  NULL;

	// DriverUnload
	g_MalwFind.DrvCdCrtl.DrvCdrom.pDriverUnload = NULL;
	g_MalwFind.DrvCdCrtl.DriverHook.bCdom_Hook  = FALSE;

}



// /**************************************************************************************************************************
//  UpperFilter Imapi CD/DVD  Dispatch Hook Function  
// /**************************************************************************************************************************
// CDROM Ext

NTSTATUS  
PreHook_Cdrom_UF_Dispatch( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
	NTSTATUS            Status        = STATUS_SUCCESS;
	PIO_STACK_LOCATION  pIrpStack     = NULL;
	PFILE_OBJECT        pFileObject   = NULL;
	ULONG               ulMajorFunc   = 0x00;
	ULONG               ulControlCode = 0x00;
	HANDLE              hProcessID    = 0;
	PSCSI_PASS_THROUGH_DIRECT  pScsiPassDirect = NULL;
	WCHAR               wzProcName[ MAX_PROCESS_LEN ] = L"Reserved";
	
	pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
	if(!pIrpStack) return CompleteRequest( pIrp, STATUS_INVALID_PARAMETER, 0 );
	
	pFileObject = pIrpStack->FileObject;
	ulMajorFunc = pIrpStack->MajorFunction;	
// *************************************************************************************//
	hProcessID = PsGetCurrentProcessId();
	GetProcessName( PsGetCurrentProcess(), wzProcName, MAX_PROCESS_LEN  );	
// *************************************************************************************//

	switch(ulMajorFunc)
	{
	case IRP_MJ_CREATE:
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pCreate) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;	
		}
		//KdPrint(("Cdrom_UF : IRP_MJ_CREATE \n"));
		if(ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_OPEN ))
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		Status = g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pCreate( pDeviceObject, pIrp );
		return Status;

	case IRP_MJ_READ:		
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pRead) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;			
		}
		//KdPrint(("Cdrom_UF : IRP_MJ_READ \n"));
		if(ISDevAccess_CdromBurn( MALWF_READ )) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		Status = g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pRead( pDeviceObject, pIrp );
		return Status;

	case IRP_MJ_WRITE:	
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pWrite)
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}		
		//KdPrint(("Cdrom_UF : IRP_MJ_WRITE \n"));
		if(ISDevAccess_CdromBurn( MALWF_WRITE )) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		Status = g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pWrite( pDeviceObject, pIrp );
		return Status;

	case IRP_MJ_DEVICE_CONTROL:	
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pDevControl) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}

		ulControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
		if(IOCTL_DISK_UPDATE_DRIVE_SIZE != ulControlCode)
		{			
			// KdPrint(("Cdrom_UF IRP_MJ_DEVICE_CONTROL--ControlCode[0x%08x][ %s ] \n", ulControlCode, GetCdromCtrlCode_Paring( ulControlCode,  pIrp->AssociatedIrp.SystemBuffer ) ));
		}

		if(ulControlCode == IOCTL_SCSI_PASS_THROUGH_DIRECT && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ))
		{
			Status = Control_Cdrom_SsciPassThroughDirect( pIrp->AssociatedIrp.SystemBuffer );
			if(Status == STATUS_ACCESS_DENIED) break;
		}
		else if(ulControlCode == IOCTL_SCSI_PASS_THROUGH && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ))
		{
			Status = Control_Cdrom_SsciPassThrough( pIrp->AssociatedIrp.SystemBuffer );
			if(Status == STATUS_ACCESS_DENIED) break;
		}

		Status = g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pDevControl( pDeviceObject, pIrp );
		return Status;

	case IRP_MJ_INTERNAL_DEVICE_CONTROL:	
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pIntDevControl)
		{
			Status = STATUS_ACCESS_DENIED;
			break;				
		}	

		ulControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
		if(IOCTL_DISK_UPDATE_DRIVE_SIZE != ulControlCode && ulControlCode != 0x001b0011)
		{
			KdPrint(("Cdrom_UF : IRP_MJ_INTERNAL_DEVICE_CONTROL--ControlCode[0x%08x][ %s ] \n", 
				ulControlCode, GetCdromCtrlCode_Paring( ulControlCode,  pIrp->AssociatedIrp.SystemBuffer ) )); 
		}

		if(ulControlCode == IOCTL_SCSI_PASS_THROUGH_DIRECT && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ))
		{
			Status = Control_Cdrom_SsciPassThroughDirect( pIrp->AssociatedIrp.SystemBuffer );
			if(Status == STATUS_ACCESS_DENIED) break;
		}
		else if(ulControlCode == IOCTL_SCSI_PASS_THROUGH && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ))
		{
			Status = Control_Cdrom_SsciPassThrough( pIrp->AssociatedIrp.SystemBuffer );
			if(Status == STATUS_ACCESS_DENIED) break;
		}

		Status = g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pIntDevControl( pDeviceObject, pIrp );		
		return Status;

	case IRP_MJ_PNP:
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pPnp)  
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		Status = g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pPnp( pDeviceObject, pIrp );
		return Status;
	}

	if(Status == STATUS_ACCESS_DENIED)
	{
		KdPrint(("Cdrom_UF -- STATUS_ACCESS_DENIED  \n\n\n"));
		Status = CompleteRequest( pIrp, STATUS_ACCESS_DENIED, 0 );
		return Status;
	}
	return Status;
}


void  
PreHook_Cdrom_UF_DriverUnload( IN PDRIVER_OBJECT pDriverObject )
{
	if(!pDriverObject || !g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pDriverUnload) return;

	//KdPrint(("Cdrom_UF DriverUnload -- STATUS_ACCESS_DENIED  \n\n"));
	g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pDriverUnload( pDriverObject );
	g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pCreate   = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pRead     = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pWrite    = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pDevControl = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pIntDevControl = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pPnp  =  NULL;

	// DriverUnload
	g_MalwFind.DrvCdCrtl.DrvCdrom_UF.pDriverUnload = NULL;
	g_MalwFind.DrvCdCrtl.DriverHook.bCdromUF_Hook = FALSE;

}


// /**************************************************************************************************************************
//  Cdrom LowerFilter PxHelp Ext  CD/DVD  Sonic Dispatch Hook Function  
// /**************************************************************************************************************************

#define  SONIC_BURNING   0x0022203F

NTSTATUS 
PreHook_Cdrom_LF_Dispatch( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
	NTSTATUS            Status        = STATUS_SUCCESS;
	PIO_STACK_LOCATION  pIrpStack     = NULL;
	PFILE_OBJECT        pFileObject   = NULL;
	ULONG               ulMajorFunc   = 0x00;
	ULONG               ulControlCode = 0x00;
	
	pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
	if(!pIrpStack) return CompleteRequest( pIrp, STATUS_INVALID_PARAMETER, 0 );
	
	pFileObject = pIrpStack->FileObject;
	ulMajorFunc = pIrpStack->MajorFunction;
	switch(ulMajorFunc)
	{
	case IRP_MJ_CREATE:
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pCreate) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;	
		}
		//KdPrint(("Cdrom_LF : IRP_MJ_CREATE \n"));
		if(ISDevAccess_CdromBurn( MALWF_CREATE ))
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		Status = g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pCreate( pDeviceObject, pIrp );
		return Status;

	case IRP_MJ_READ:		
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pRead) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;			
		}

		//KdPrint(("Cdrom_LF : IRP_MJ_READ \n"));
		if(ISDevAccess_CdromBurn( MALWF_READ )) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		Status = g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pRead( pDeviceObject, pIrp );
		return Status;

	case IRP_MJ_WRITE:	
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pWrite)
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		
		//KdPrint(("Cdrom_LF : IRP_MJ_WRITE \n"));
		if(ISDevAccess_CdromBurn( MALWF_WRITE )) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		Status = g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pWrite( pDeviceObject, pIrp );
		return Status;

	case IRP_MJ_DEVICE_CONTROL:	
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pDevControl) 
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}

		ulControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
		// KdPrint(("Cdrom_LF : IRP_MJ_DEVICE_CONTROL--ControlCode[0x%08x][ %s ] \n", ulControlCode, GetCdromCtrlCode_Paring( ulControlCode,  pIrp->AssociatedIrp.SystemBuffer ) )); 

		if((ulControlCode == SONIC_BURNING) && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ))
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		else if(ulControlCode == IOCTL_SCSI_PASS_THROUGH_DIRECT && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ))
		{
			Status = Control_Cdrom_SsciPassThroughDirect( pIrp->AssociatedIrp.SystemBuffer );
			if(Status == STATUS_ACCESS_DENIED) break;

		}
		else if(ulControlCode == IOCTL_SCSI_PASS_THROUGH && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ) )
		{
			Status = Control_Cdrom_SsciPassThrough( pIrp->AssociatedIrp.SystemBuffer );
			if(Status == STATUS_ACCESS_DENIED) break;
		}

		Status = g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pDevControl( pDeviceObject, pIrp );
		return Status;

	case IRP_MJ_INTERNAL_DEVICE_CONTROL:	
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pIntDevControl)
		{
			Status = STATUS_ACCESS_DENIED;
			break;				
		}

		ulControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
		if( ulControlCode != IOCTL_SERIAL_SET_BREAK_ON && ulControlCode != 0x001b0011 && ulControlCode != 0x001b0013 )
		{
			// KdPrint(("Cdrom_LF : IRP_MJ_INTERNAL_DEVICE_CONTROL--ControlCode[0x%08x][ %s ] \n", ulControlCode, GetCdromCtrlCode_Paring( ulControlCode,  pIrp->AssociatedIrp.SystemBuffer )  )); 
		}
		
		if((ulControlCode == SONIC_BURNING) && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ))
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		else if((ulControlCode == IOCTL_SCSI_PASS_THROUGH_DIRECT) && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ))
		{
			Status = Control_Cdrom_SsciPassThroughDirect( pIrp->AssociatedIrp.SystemBuffer );
			if(Status == STATUS_ACCESS_DENIED) break;
		}
		else if(ulControlCode == IOCTL_SCSI_PASS_THROUGH && ISDevAccess_CdromBurn( MALWF_CREATE | MALWF_WRITE ))
		{
			Status = Control_Cdrom_SsciPassThrough( pIrp->AssociatedIrp.SystemBuffer );
			if(Status == STATUS_ACCESS_DENIED) break;
		}

		Status = g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pIntDevControl( pDeviceObject, pIrp );		
		return Status;

	case IRP_MJ_PNP:
		if(!g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pPnp)  
		{
			Status = STATUS_ACCESS_DENIED;
			break;
		}
		Status = g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pPnp( pDeviceObject, pIrp );
		return Status;
	}

	if(Status == STATUS_ACCESS_DENIED)
	{
		KdPrint(("Cdrom_LF : STATUS_ACCESS_DENIED  \n\n\n"));
		Status = CompleteRequest( pIrp, STATUS_ACCESS_DENIED, 0 );
		return Status;
	}
	return Status;
}


void  
PreHook_Cdrom_LF_DriverUnload( IN PDRIVER_OBJECT pDriverObject )
{
	if(!pDriverObject || !g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pDriverUnload) 
	{
		return;
	}

	//KdPrint(("Cdrom_LF DriverUnload  \n\n\n"));
	g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pDriverUnload( pDriverObject );
	g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pCreate   = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pRead     = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pWrite    = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pDevControl = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pIntDevControl = NULL;
	g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pPnp  =  NULL;

	// DriverUnload
	g_MalwFind.DrvCdCrtl.DrvCdrom_LF.pDriverUnload = NULL;
	g_MalwFind.DrvCdCrtl.DriverHook.bCdromLF_Hook = FALSE;

}





char* GetCdromCtrlCode_Paring( IN ULONG ulControlCode, IN PVOID pSystemBuffer )
{
	switch(ulControlCode)
	{
	// FILE_DEVICE_CONTROLLER
	// FILE_READ_ACCESS | FILE_WRITE_ACCESS
	case IOCTL_SCSI_PASS_THROUGH:      
	{
		ScsiPassThrough_Print( (PSCSI_PASS_THROUGH)pSystemBuffer );
		return "IOCTL_SCSI_PASS_THROUGH";                
	}	
	case IOCTL_SCSI_PASS_THROUGH_DIRECT: 
	{
		ScsiPassThroughDirect_Print( (PSCSI_PASS_THROUGH_DIRECT)pSystemBuffer );
		return "IOCTL_SCSI_PASS_THROUGH_DIRECT";
	}

	case IOCTL_IDE_PASS_THROUGH:       
	{
		return "IOCTL_IDE_PASS_THROUGH";   
	}
	case IOCTL_ATA_PASS_THROUGH:       
	{
		return "IOCTL_ATA_PASS_THROUGH";   
	}
	case IOCTL_ATA_PASS_THROUGH_DIRECT:
	{
		return "IOCTL_ATA_PASS_THROUGH_DIRECT";   
	}
	case IOCTL_SCSI_MINIPORT: return "IOCTL_SCSI_MINIPORT";  
	case IOCTL_ATA_MINIPORT:  return "IOCTL_ATA_MINIPORT";   

	case IOCTL_SCSI_GET_INQUIRY_DATA:  return "IOCTL_SCSI_GET_INQUIRY_DATA";    
	case IOCTL_SCSI_GET_CAPABILITIES:  return "IOCTL_SCSI_GET_CAPABILITIES";
	case IOCTL_SCSI_GET_ADDRESS:       return "IOCTL_SCSI_GET_ADDRESS"; 
	case IOCTL_SCSI_RESCAN_BUS:        return "IOCTL_SCSI_RESCAN_BUS"; 
	case IOCTL_SCSI_GET_DUMP_POINTERS: return "IOCTL_SCSI_GET_DUMP_POINTERS";   
	case IOCTL_SCSI_FREE_DUMP_POINTERS:return "IOCTL_SCSI_FREE_DUMP_POINTERS";

    //
	// FILE_DEVICE_CDROM
	//
	case IOCTL_CDROM_READ_TOC:       return "IOCTL_CDROM_READ_TOC";          
	case IOCTL_CDROM_SEEK_AUDIO_MSF: return "IOCTL_CDROM_SEEK_AUDIO_MSF";                
	case IOCTL_CDROM_STOP_AUDIO:     return "IOCTL_CDROM_STOP_AUDIO";                
	case IOCTL_CDROM_PAUSE_AUDIO:    return "IOCTL_CDROM_PAUSE_AUDIO";                
	case IOCTL_CDROM_RESUME_AUDIO:   return "IOCTL_CDROM_RESUME_AUDIO";                
	case IOCTL_CDROM_GET_VOLUME:     return "IOCTL_CDROM_GET_VOLUME";                
	case IOCTL_CDROM_PLAY_AUDIO_MSF: return "IOCTL_CDROM_PLAY_AUDIO_MSF";                
	case IOCTL_CDROM_SET_VOLUME:     return "IOCTL_CDROM_SET_VOLUME";                
	case IOCTL_CDROM_READ_Q_CHANNEL: return "IOCTL_CDROM_READ_Q_CHANNEL";      

#if (NTDDI_VERSION < NTDDI_WS03)
	case IOCTL_CDROM_GET_CONTROL:   return "IOCTL_CDROM_GET_CONTROL";              
#else
	case OBSOLETE_IOCTL_CDROM_GET_CONTROL: return "OBSOLETE_IOCTL_CDROM_GET_CONTROL";       
#endif

	case IOCTL_CDROM_GET_LAST_SESSION:   return "IOCTL_CDROM_GET_LAST_SESSION";          
	case IOCTL_CDROM_RAW_READ:           return "IOCTL_CDROM_RAW_READ";            
	case IOCTL_CDROM_DISK_TYPE:          return "IOCTL_CDROM_DISK_TYPE";            
	case IOCTL_CDROM_GET_DRIVE_GEOMETRY:    return "IOCTL_CDROM_GET_DRIVE_GEOMETRY";       
	case IOCTL_CDROM_GET_DRIVE_GEOMETRY_EX: return "IOCTL_CDROM_GET_DRIVE_GEOMETRY_EX";       
	case IOCTL_CDROM_READ_TOC_EX:       return "IOCTL_CDROM_READ_TOC_EX";       
	case IOCTL_CDROM_GET_CONFIGURATION: return "IOCTL_CDROM_GET_CONFIGURATION";           
	case IOCTL_CDROM_EXCLUSIVE_ACCESS:  return "IOCTL_CDROM_EXCLUSIVE_ACCESS";           
	case IOCTL_CDROM_SET_SPEED:         return "IOCTL_CDROM_SET_SPEED";           
	case IOCTL_CDROM_GET_INQUIRY_DATA:  return "IOCTL_CDROM_GET_INQUIRY_DATA";           

	case IOCTL_CDROM_CHECK_VERIFY: return "IOCTL_CDROM_CHECK_VERIFY";              
	case IOCTL_CDROM_MEDIA_REMOVAL:return "IOCTL_CDROM_MEDIA_REMOVAL";              
	case IOCTL_CDROM_EJECT_MEDIA:  return "IOCTL_CDROM_EJECT_MEDIA";                 
	case IOCTL_CDROM_LOAD_MEDIA:   return "IOCTL_CDROM_LOAD_MEDIA";      
	case IOCTL_CDROM_RESERVE:      return "IOCTL_CDROM_RESERVE";         
	case IOCTL_CDROM_RELEASE:      return "IOCTL_CDROM_RELEASE";         
	case IOCTL_CDROM_FIND_NEW_DEVICES: return "IOCTL_CDROM_FIND_NEW_DEVICES";
		//
		// CDVD Device Control Functions
	case IOCTL_DVD_START_SESSION:  return "IOCTL_DVD_START_SESSION";
	case IOCTL_DVD_READ_KEY:       return "IOCTL_DVD_READ_KEY";
	case IOCTL_DVD_SEND_KEY:       return "IOCTL_DVD_SEND_KEY";
	case IOCTL_DVD_END_SESSION:    return "IOCTL_DVD_END_SESSION";
	case IOCTL_DVD_SET_READ_AHEAD: return "IOCTL_DVD_SET_READ_AHEAD";
	case IOCTL_DVD_GET_REGION:     return "IOCTL_DVD_GET_REGION";
	case IOCTL_DVD_SEND_KEY2:      return "IOCTL_DVD_SEND_KEY2";
		//
		// AACS-related IOCTLs
	case IOCTL_AACS_READ_MEDIA_KEY_BLOCK_SIZE: return "IOCTL_AACS_READ_MEDIA_KEY_BLOCK_SIZE"; 
	case IOCTL_AACS_READ_MEDIA_KEY_BLOCK: return "IOCTL_AACS_READ_MEDIA_KEY_BLOCK";      
	case IOCTL_AACS_START_SESSION: return "IOCTL_AACS_START_SESSION";             
	case IOCTL_AACS_END_SESSION:   return "IOCTL_AACS_END_SESSION";             
	case IOCTL_AACS_SEND_CERTIFICATE: return "IOCTL_AACS_SEND_CERTIFICATE";          
	case IOCTL_AACS_GET_CERTIFICATE:  return "IOCTL_AACS_GET_CERTIFICATE";          
	case IOCTL_AACS_GET_CHALLENGE_KEY:  return "IOCTL_AACS_GET_CHALLENGE_KEY";         
	case IOCTL_AACS_SEND_CHALLENGE_KEY: return "IOCTL_AACS_SEND_CHALLENGE_KEY";        
	case IOCTL_AACS_READ_VOLUME_ID:     return "IOCTL_AACS_READ_VOLUME_ID";         
	case IOCTL_AACS_READ_SERIAL_NUMBER: return "IOCTL_AACS_READ_SERIAL_NUMBER";        
	case IOCTL_AACS_READ_MEDIA_ID:      return "IOCTL_AACS_READ_MEDIA_ID";        
	case IOCTL_AACS_READ_BINDING_NONCE: return "IOCTL_AACS_READ_BINDING_NONCE";        
	case IOCTL_AACS_GENERATE_BINDING_NONCE: return "IOCTL_AACS_GENERATE_BINDING_NONCE";    
		
		// DVD Structure queries
	case IOCTL_DVD_READ_STRUCTURE: return "IOCTL_DVD_READ_STRUCTURE";
		// The following file contains the IOCTL_STORAGE class ioctl definitions
	case IOCTL_STORAGE_SET_READ_AHEAD: return "IOCTL_STORAGE_SET_READ_AHEAD";
		
		// NtDeviceIoControlFile
	case IOCTL_DISK_GET_DRIVE_GEOMETRY: return "IOCTL_DISK_GET_DRIVE_GEOMETRY";  
	case IOCTL_DISK_GET_PARTITION_INFO: return "IOCTL_DISK_GET_PARTITION_INFO";  
	case IOCTL_DISK_SET_PARTITION_INFO: return "IOCTL_DISK_SET_PARTITION_INFO";  
	case IOCTL_DISK_GET_DRIVE_LAYOUT:   return "IOCTL_DISK_GET_DRIVE_LAYOUT";  
	case IOCTL_DISK_SET_DRIVE_LAYOUT:   return "IOCTL_DISK_SET_DRIVE_LAYOUT";  
	case IOCTL_DISK_VERIFY:             return "IOCTL_DISK_VERIFY";    
	case IOCTL_DISK_FORMAT_TRACKS:      return "IOCTL_DISK_FORMAT_TRACKS";    

	// case IOCTL_DISK_REASSIGN_BLOCKS:    return "IOCTL_DISK_REASSIGN_BLOCKS";    

	case IOCTL_DISK_PERFORMANCE:        return "IOCTL_DISK_PERFORMANCE";    
	case IOCTL_DISK_IS_WRITABLE:        return "IOCTL_DISK_IS_WRITABLE";    
	case IOCTL_DISK_LOGGING:            return "IOCTL_DISK_LOGGING";    
	case IOCTL_DISK_FORMAT_TRACKS_EX:   return "IOCTL_DISK_FORMAT_TRACKS_EX";    
	case IOCTL_DISK_HISTOGRAM_STRUCTURE:return "IOCTL_DISK_HISTOGRAM_STRUCTURE"; 
	case IOCTL_DISK_HISTOGRAM_DATA:     return "IOCTL_DISK_HISTOGRAM_DATA";    
	case IOCTL_DISK_HISTOGRAM_RESET:    return "IOCTL_DISK_HISTOGRAM_RESET";    
	case IOCTL_DISK_REQUEST_STRUCTURE:  return "IOCTL_DISK_REQUEST_STRUCTURE";   
	case IOCTL_DISK_REQUEST_DATA:       return "IOCTL_DISK_REQUEST_DATA";   
	case IOCTL_DISK_PERFORMANCE_OFF:    return "IOCTL_DISK_PERFORMANCE_OFF";   

	case IOCTL_DISK_CONTROLLER_NUMBER:  return "IOCTL_DISK_CONTROLLER_NUMBER";   
	//
	// New IOCTLs for GUID Partition tabled disks.
	//
	case IOCTL_DISK_GET_PARTITION_INFO_EX: return "IOCTL_DISK_GET_PARTITION_INFO_EX";   
	case IOCTL_DISK_SET_PARTITION_INFO_EX: return "IOCTL_DISK_SET_PARTITION_INFO_EX";   
	case IOCTL_DISK_GET_DRIVE_LAYOUT_EX:   return "IOCTL_DISK_GET_DRIVE_LAYOUT_EX";     
	case IOCTL_DISK_SET_DRIVE_LAYOUT_EX:   return "IOCTL_DISK_SET_DRIVE_LAYOUT_EX";     
	case IOCTL_DISK_CREATE_DISK:           return "IOCTL_DISK_CREATE_DISK";         
	case IOCTL_DISK_GET_LENGTH_INFO:       return "IOCTL_DISK_GET_LENGTH_INFO";         
	case IOCTL_DISK_GET_DRIVE_GEOMETRY_EX: return "IOCTL_DISK_GET_DRIVE_GEOMETRY_EX";   
	
	// New IOCTL for disk devices that support 8 byte LBA
	//
	// case IOCTL_DISK_REASSIGN_BLOCKS_EX: return "IOCTL_DISK_REASSIGN_BLOCKS_EX";
	case IOCTL_DISK_UPDATE_DRIVE_SIZE:  return "IOCTL_DISK_UPDATE_DRIVE_SIZE";      
	case IOCTL_DISK_GROW_PARTITION:     return "IOCTL_DISK_GROW_PARTITION";      
	case IOCTL_DISK_GET_CACHE_INFORMATION: return "IOCTL_DISK_GET_CACHE_INFORMATION";         
	case IOCTL_DISK_SET_CACHE_INFORMATION: return "IOCTL_DISK_SET_CACHE_INFORMATION";        

#if (NTDDI_VERSION < NTDDI_WIN2003)
	case IOCTL_DISK_GET_WRITE_CACHE_STATE:    return "IOCTL_DISK_GET_WRITE_CACHE_STATE";         
#else
	case OBSOLETE_DISK_GET_WRITE_CACHE_STATE: return "OBSOLETE_DISK_GET_WRITE_CACHE_STATE";      
#endif

	case IOCTL_DISK_DELETE_DRIVE_LAYOUT: return "IOCTL_DISK_DELETE_DRIVE_LAYOUT";           
	case IOCTL_DISK_UPDATE_PROPERTIES:   return "IOCTL_DISK_UPDATE_PROPERTIES";       

	case IOCTL_DISK_FORMAT_DRIVE:        return "IOCTL_DISK_FORMAT_DRIVE";             
	case IOCTL_DISK_SENSE_DEVICE:        return "IOCTL_DISK_SENSE_DEVICE";             
	case IOCTL_DISK_GET_CACHE_SETTING:   return "IOCTL_DISK_GET_CACHE_SETTING";         
	case IOCTL_DISK_SET_CACHE_SETTING:   return "IOCTL_DISK_SET_CACHE_SETTING";         
	case IOCTL_DISK_COPY_DATA:           return "IOCTL_DISK_COPY_DATA";         
	case IOCTL_DISK_INTERNAL_SET_VERIFY:   return "IOCTL_DISK_INTERNAL_SET_VERIFY";       
	case IOCTL_DISK_INTERNAL_CLEAR_VERIFY: return "IOCTL_DISK_INTERNAL_CLEAR_VERIFY";      
	case IOCTL_DISK_INTERNAL_SET_NOTIFY:   return "IOCTL_DISK_INTERNAL_SET_NOTIFY";      
	case IOCTL_DISK_CHECK_VERIFY:     return "IOCTL_DISK_CHECK_VERIFY";      
	case IOCTL_DISK_MEDIA_REMOVAL:    return "IOCTL_DISK_MEDIA_REMOVAL";      
	case IOCTL_DISK_EJECT_MEDIA:      return "IOCTL_DISK_EJECT_MEDIA";      
	case IOCTL_DISK_LOAD_MEDIA:       return "IOCTL_DISK_LOAD_MEDIA";      
	case IOCTL_DISK_RESERVE:          return "IOCTL_DISK_RESERVE";      
	case IOCTL_DISK_RELEASE:          return "IOCTL_DISK_RELEASE";      
	case IOCTL_DISK_FIND_NEW_DEVICES: return "IOCTL_DISK_FIND_NEW_DEVICES";      
	case IOCTL_DISK_GET_MEDIA_TYPES:  return "IOCTL_DISK_GET_MEDIA_TYPES";    

	//
	// The following device control codes are common for all class drivers.  They
	// should be used in place of the older IOCTL_DISK, IOCTL_CDROM and IOCTL_TAPE
	// common codes
	//
	case IOCTL_STORAGE_CHECK_VERIFY:  return "IOCTL_STORAGE_CHECK_VERIFY";             
	case IOCTL_STORAGE_CHECK_VERIFY2: return "IOCTL_STORAGE_CHECK_VERIFY2";              
	case IOCTL_STORAGE_MEDIA_REMOVAL: return "IOCTL_STORAGE_MEDIA_REMOVAL";              
	case IOCTL_STORAGE_EJECT_MEDIA:   return "IOCTL_STORAGE_EJECT_MEDIA";              
	case IOCTL_STORAGE_LOAD_MEDIA:    return "IOCTL_STORAGE_LOAD_MEDIA";              
	case IOCTL_STORAGE_LOAD_MEDIA2:   return "IOCTL_STORAGE_LOAD_MEDIA2";              
	case IOCTL_STORAGE_RESERVE:       return "IOCTL_STORAGE_RESERVE";              
	case IOCTL_STORAGE_RELEASE:       return "IOCTL_STORAGE_RELEASE";              
	case IOCTL_STORAGE_FIND_NEW_DEVICES: return "IOCTL_STORAGE_FIND_NEW_DEVICES";    

	case IOCTL_STORAGE_EJECTION_CONTROL: return "IOCTL_STORAGE_EJECTION_CONTROL";             
	case IOCTL_STORAGE_MCN_CONTROL:      return "IOCTL_STORAGE_MCN_CONTROL";             

	case IOCTL_STORAGE_GET_MEDIA_TYPES:     return "IOCTL_STORAGE_GET_MEDIA_TYPES";             
	case IOCTL_STORAGE_GET_MEDIA_TYPES_EX:  return "IOCTL_STORAGE_GET_MEDIA_TYPES_EX";            
	case IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER: return "IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER";      
	case IOCTL_STORAGE_GET_HOTPLUG_INFO:  return "IOCTL_STORAGE_GET_HOTPLUG_INFO";            
	case IOCTL_STORAGE_SET_HOTPLUG_INFO:  return "IOCTL_STORAGE_SET_HOTPLUG_INFO";            

	case IOCTL_STORAGE_RESET_BUS:         return "IOCTL_STORAGE_RESET_BUS";              
	case IOCTL_STORAGE_RESET_DEVICE:      return "IOCTL_STORAGE_RESET_DEVICE";              
	case IOCTL_STORAGE_BREAK_RESERVATION: return "IOCTL_STORAGE_BREAK_RESERVATION";            
	case IOCTL_STORAGE_PERSISTENT_RESERVE_IN:  return "IOCTL_STORAGE_PERSISTENT_RESERVE_IN";        
	case IOCTL_STORAGE_PERSISTENT_RESERVE_OUT: return "IOCTL_STORAGE_PERSISTENT_RESERVE_OUT";       

	case IOCTL_STORAGE_GET_DEVICE_NUMBER:return "IOCTL_STORAGE_GET_DEVICE_NUMBER";            
	case IOCTL_STORAGE_PREDICT_FAILURE:  return "IOCTL_STORAGE_PREDICT_FAILURE";            
	case IOCTL_STORAGE_READ_CAPACITY:    return "IOCTL_STORAGE_READ_CAPACITY";            
	case IOCTL_STORAGE_QUERY_PROPERTY:   return "IOCTL_STORAGE_QUERY_PROPERTY";       
	//
	// IOCTLs for bandwidth contracts on storage devices
	// (Move this to ntddsfio if we decide to use a new base)
	//
	case IOCTL_STORAGE_GET_BC_PROPERTIES:  return "IOCTL_STORAGE_GET_BC_PROPERTIES";             
	case IOCTL_STORAGE_ALLOCATE_BC_STREAM: return "IOCTL_STORAGE_ALLOCATE_BC_STREAM";             
	case IOCTL_STORAGE_FREE_BC_STREAM:     return "IOCTL_STORAGE_FREE_BC_STREAM";             
	//
	// IOCTL to check for priority support
	//
	case IOCTL_STORAGE_CHECK_PRIORITY_HINT_SUPPORT: return "IOCTL_STORAGE_CHECK_PRIORITY_HINT_SUPPORT";         
	//
	case OBSOLETE_IOCTL_STORAGE_RESET_BUS:    return "OBSOLETE_IOCTL_STORAGE_RESET_BUS";           
	case OBSOLETE_IOCTL_STORAGE_RESET_DEVICE: return "OBSOLETE_IOCTL_STORAGE_RESET_DEVICE";           

	default: 
		return "";
		break;
	}
	
}









// /**************************************************************************************************************************
//  CDROM   CD/DVD  redbook Dispatch Hook Function  
// /**************************************************************************************************************************


void ScsiPassThrough_Print( IN PSCSI_PASS_THROUGH pPassThrough )
{
	ASSERT( pPassThrough );
	if(!pPassThrough) return;

	/*
	KdPrint(("pPassThrough->Length     : [%d] \n", pPassThrough->Length ));
	KdPrint(("pPassThrough->ScsiStatus : [%d] \n", pPassThrough->ScsiStatus ));
	KdPrint(("pPassThrough->PathId     : [%d] \n", pPassThrough->PathId ));
	KdPrint(("pPassThrough->TargetId   : [%d] \n", pPassThrough->TargetId ));
	KdPrint(("pPassThrough->Lun        : [%d] \n", pPassThrough->Lun ));	
	KdPrint(("pPassThrough->CdbLength  : [%d] \n", pPassThrough->CdbLength ));
	KdPrint(("pPassThrough->SenseInfoLength : [%d] \n", pPassThrough->SenseInfoLength ));
	
	KdPrint(("pPassThrough->DataTransferLength : [%d] \n", pPassThrough->DataTransferLength ));
	KdPrint(("pPassThrough->TimeOutValue       : [%d] \n", pPassThrough->TimeOutValue ));
	KdPrint(("pPassThrough->SenseInfoOffset    : [%d] \n", pPassThrough->SenseInfoOffset ));
	KdPrint(("pPassThrough->DataIn             : [%d] \n", pPassThrough->DataIn  ));
	*/

	if(pPassThrough->DataIn == SCSI_IOCTL_DATA_OUT)
	{
		KdPrint((" \npPassThrough->DataIn : [ SCSI_IOCTL_DATA_OUT ] CdbOpCode=[ %02x ]", pPassThrough->Cdb[0] ));
	}
	else if(pPassThrough->DataIn == SCSI_IOCTL_DATA_IN)
	{
		// KdPrint(("\npPassThrough->DataIn : [ SCSI_IOCTL_DATA_IN ] CdbOpCode=[ %02x ]", pPassThrough->Cdb[0] ));
	}
	else if(pPassThrough->DataIn == SCSI_IOCTL_DATA_UNSPECIFIED)
	{
		// KdPrint(("\npPassThrough->DataIn : [ SCSI_IOCTL_DATA_UNSPECIFIED ]"));
	}

	return;
}



void ScsiPassThroughDirect_Print( IN PSCSI_PASS_THROUGH_DIRECT pDirect )
{
	ASSERT( pDirect );
	if(!pDirect) return;

	/*
	KdPrint(("pDirect->Length     : [%d] \n", pDirect->Length ));
	KdPrint(("pDirect->ScsiStatus : [%d] \n", pDirect->ScsiStatus ));
	KdPrint(("pDirect->PathId     : [%d] \n", pDirect->PathId ));
	KdPrint(("pDirect->TargetId   : [%d] \n", pDirect->TargetId ));
	KdPrint(("pDirect->Lun        : [%d] \n", pDirect->Lun ));	
	KdPrint(("pDirect->CdbLength  : [%d] \n", pDirect->CdbLength ));
	KdPrint(("pDirect->SenseInfoLength : [%d] \n", pDirect->SenseInfoLength ));
	
	KdPrint(("pDirect->DataTransferLength : [%d] \n", pDirect->DataTransferLength ));
	KdPrint(("pDirect->TimeOutValue       : [%d] \n", pDirect->TimeOutValue ));
	KdPrint(("pDirect->SenseInfoOffset    : [%d] \n", pDirect->SenseInfoOffset ));
	KdPrint(("pDirect->DataIn             : [%d] \n", pDirect->DataIn  ));
	*/

	// KdPrint(("pDirect->DataBuffer         : [%s] \n", pDirect->DataBuffer ));
	if(pDirect->DataIn == SCSI_IOCTL_DATA_OUT)
	{
		KdPrint(("pDirect->DataIn : [ SCSI_IOCTL_DATA_OUT ] CdbOpCode=[ %02x ] \n", pDirect->Cdb[0] ));
	}
	else if(pDirect->DataIn == SCSI_IOCTL_DATA_IN)
	{
		// KdPrint(("pDirect->DataIn : [ SCSI_IOCTL_DATA_IN ] CdbOpCode=[ %02x ] \n", pDirect->Cdb[0] ));
	}
	else if(pDirect->DataIn == SCSI_IOCTL_DATA_UNSPECIFIED)
	{
		// KdPrint(("pDirect->DataIn : [ SCSI_IOCTL_DATA_UNSPECIFIED ] CdbOpCode=[ %02x ] \n", pDirect->Cdb[0] ));
	}

	return;
}



