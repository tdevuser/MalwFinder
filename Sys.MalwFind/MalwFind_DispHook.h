#ifndef _MALWFIND_DISP_HOOK_H_
#define _MALWFIND_DISP_HOOK_H_

#include  <ntifs.h>
#include <ntddscsi.h>
#include "MalwFind_DataTypes.h"
#include "MalwFind_Command.h"


#ifdef __cplusplus
extern "C"
{
#endif


void FltDrvDispHook_Init  ( IN PFLT_DRV_CDCTRL pDevControl );
void FltDrvDispHook_UnInit( IN PFLT_DRV_CDCTRL pDevControl );


/******************************************************************************************************/

// Hook
void PreDispatch_Device_Hook(void);
// UnHook
void PreDispatch_Device_UnHook(void);


BOOLEAN Hook_Cdrom(void);
BOOLEAN UnHook_Cdrom(void);

BOOLEAN Hook_Cdrom_UF(void);
BOOLEAN UnHook_Cdrom_UF(void);

BOOLEAN Hook_Cdrom_LF(void);
BOOLEAN UnHook_Cdrom_LF(void);


BOOLEAN Hook_Set_Dispatch_Unload(  IN PDRIVER_OBJECT    pDriverObject, 
								   IN OUT PDP_FUNC      pDpFunc, 
								   IN PDRIVER_DISPATCH  pHookDriverDispatch,
								   IN PDRIVER_UNLOAD    pHookDriverUnload  );

void Hook_Clr_Dispatch_Unload( IN PDRIVER_OBJECT  pDriverObject, IN PDP_FUNC pDpFunc );

// 버닝제어 플래그
BOOLEAN  ISDevAccess_CdromBurn( IN ULONG ulAccess );

// /**************************************************************************************************************************
//  CDROM   CD/DVD  redbook Dispatch Hook Function  
// /**************************************************************************************************************************

// Cdrom
NTSTATUS PreHook_Cdrom_Dispatch( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );
void     PreHook_Cdrom_DriverUnload( IN PDRIVER_OBJECT pDriverObject );

// Cdrom UF
// Imapi
NTSTATUS PreHook_Cdrom_UF_Dispatch( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );
void     PreHook_Cdrom_UF_DriverUnload( IN PDRIVER_OBJECT pDriverObject );

// Cdrom LF
// CDROM Sonic
NTSTATUS PreHook_Cdrom_LF_Dispatch( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp );
void     PreHook_Cdrom_LF_DriverUnload( IN PDRIVER_OBJECT pDriverObject );


NTSTATUS  Control_Cdrom_SsciPassThrough( IN PVOID pBuffer );
NTSTATUS  Control_Cdrom_SsciPassThroughDirect( IN PVOID pBuffer );


/*****************************************************************************************************/

char* GetCdromCtrlCode_Paring( IN ULONG ulControlCode, IN PVOID pSystemBuffer );

void ScsiPassThrough_Print( IN PSCSI_PASS_THROUGH pPassThrough );
void ScsiPassThroughDirect_Print( IN PSCSI_PASS_THROUGH_DIRECT pDirect );



#ifdef __cplusplus
}
#endif


#endif