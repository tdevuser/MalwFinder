#ifndef _MALWFIND_PROCESS_H_
#define _MALWFIND_PROCESS_H_

#include <ntifs.h>
#include <ntddk.h>
#include "MalwFind_DataTypes.h"


#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************************************************/	
// Kernel 에 익스포트된 함수
NTKERNELAPI  PEPROCESS  PsInitialSystemProcess;

/*****************************************************************************************************************/

// EProcess Offset
BOOLEAN FsdSetRefProcess(void);

/*****************************************************************************************************************/

// Process Callback Func 
void FltDrv_ProcessNotify_Routine( IN HANDLE hParentPId, IN HANDLE hProcessId, IN BOOLEAN bCreate );	



/*****************************************************************************************************************/

NTSTATUS 
Drv_ProcLC_CreateEvent(void);

void     
Drv_ProcLC_CloseEvent(void);

NTSTATUS	
Drv_ProcLC_FetchData(char* pOutputBuf, ULONG *inBufSize, ULONG outBufSize);


BOOLEAN 
Drv_ProcLC_Add( IN ULONG ulProcID, IN BOOLEAN bCreate );

void
Drv_ProcLC_Delete( IN PDRV_P_ENTRY pProcLCEntry );


/*****************************************************************************************************************/



#ifdef __cplusplus
}
#endif


#endif