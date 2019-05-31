
#include "MalwFind.h"
#include "MalwFind_Process.h"
#include "MalwFind_LogFunc.h"
#include "MalwFind_Util.h"

const WCHAR* g_pEXE_MALWFIND_AGENT = L"MalwFind.exe";

extern G_MALW_FIND  g_MalwFind;

BOOLEAN  
FsdSetRefProcess()
{
	size_t     nLength  = 0;	
	PEPROCESS  pProcess = NULL;
	NTSTATUS   Status   = STATUS_SUCCESS;
		
	__try
	{
		pProcess = PsGetCurrentProcess();
		if(!pProcess) return FALSE;

		for(g_MalwFind.RefProc.ulImageName=0; g_MalwFind.RefProc.ulImageName<3*PAGE_SIZE; g_MalwFind.RefProc.ulImageName++)
		{
			Status = RtlStringCbLengthA( SYS_PROC_NAME, 16, &nLength );	

			if(!_strnicmp( SYS_PROC_NAME, (PCHAR)pProcess+g_MalwFind.RefProc.ulImageName, nLength )) 
			{
				break;
			}
		}

		switch(g_MalwFind.RefProc.ulImageName) 
		{
		case PNAME_NT4:
			g_MalwFind.RefProc.ulPeb = PEB_NT4;
			break;

		case PNAME_W2K:
			g_MalwFind.RefProc.ulPeb = PEB_W2K;
			g_MalwFind.RefProc.ulPId = PID_W2K;
			g_MalwFind.RefProc.ulActiveProcessLinks = PROC_LINKS_W2K;
			break;

		case PNAME_WXP:
			g_MalwFind.RefProc.ulPeb = PEB_WXP;
			g_MalwFind.RefProc.ulPId = PID_WXP;
			g_MalwFind.RefProc.ulActiveProcessLinks = PROC_LINKS_WXP;
			break;

		case PNAME_WNET:
			g_MalwFind.RefProc.ulPeb = PEB_WNET;
			g_MalwFind.RefProc.ulPId = PID_WNET;
			g_MalwFind.RefProc.ulActiveProcessLinks = PROC_LINKS_WNET;
			break;
		//case PNAME_WNET_SP1:
		case PNAME_WNET_SP2:
			g_MalwFind.RefProc.ulPeb = PEB_WNET_SP2;
			g_MalwFind.RefProc.ulPId = PID_WNET_SP2;
			g_MalwFind.RefProc.ulActiveProcessLinks = PROC_LINKS_WNET_SP2;
			break;

		case PNAME_WNET64:
			g_MalwFind.RefProc.ulPeb = PEB_WNET64;
			g_MalwFind.RefProc.ulPId = PID_WNET64;
			g_MalwFind.RefProc.ulActiveProcessLinks = PROC_LINKS_WNET64;
			break;
		
		//case PNAME_WNET64_SP1:
		case PNAME_WNET64_SP2:
			g_MalwFind.RefProc.ulPeb = PEB_WNET64_SP2;
			g_MalwFind.RefProc.ulPId = PID_WNET64_SP2;
			g_MalwFind.RefProc.ulActiveProcessLinks = PROC_LINKS_WNET64_SP2;
			break;

		case PNAME_WLH:
			g_MalwFind.RefProc.ulPeb = PEB_WLH;
			g_MalwFind.RefProc.ulPId = PID_WLH;
			g_MalwFind.RefProc.ulActiveProcessLinks = PROC_LINKS_WLH;
			break;

		case PNAME_WLH64:
			g_MalwFind.RefProc.ulPeb = PEB_WLH64;
			g_MalwFind.RefProc.ulPId = PID_WLH64;
			g_MalwFind.RefProc.ulActiveProcessLinks = PROC_LINKS_WLH64;
			break;

		case PNAME_WIN7:
			g_MalwFind.RefProc.ulPeb = PEB_WIN7;
			g_MalwFind.RefProc.ulPId = PID_WIN7;
			g_MalwFind.RefProc.ulActiveProcessLinks = PROC_LINKS_WIN7;
			break;

		case PNAME_WIN7_64:
			g_MalwFind.RefProc.ulPeb = PEB_WIN7_64;
			g_MalwFind.RefProc.ulPId = PID_WIN7_64;
			g_MalwFind.RefProc.ulActiveProcessLinks = PROC_LINKS_WIN7_64;
			break;

		case PNAME_WIN8:
			g_MalwFind.RefProc.ulPeb = PEB_WIN8;
			g_MalwFind.RefProc.ulPId = PID_WIN8;
			g_MalwFind.RefProc.ulActiveProcessLinks = PROC_LINKS_WIN8;
			break;

		case PNAME_WIN8_64:
			g_MalwFind.RefProc.ulPeb = PEB_WIN8_64;
			g_MalwFind.RefProc.ulPId = PID_WIN8_64;
			g_MalwFind.RefProc.ulActiveProcessLinks = PROC_LINKS_WIN8_64;
			break;

		default:
			KdPrint(("g_MalwFind.ProcNameOffset is out of case %ld\n", g_MalwFind.RefProc.ulImageName ));
			break;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("FsdSetRefProcess Generated! \n" ));
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************************************************************/

NTSTATUS  
Drv_ProcLC_CreateEvent(void)
{
	NTSTATUS Status = STATUS_SUCCESS;
	DbgPrint(("Drv_ProcLC_CreateEvent\n"));

	// 이벤트가 생성되지 않았다면 생성한다.
	if(!g_MalwFind.DrvProc.hProcLC_EventHandle)
	{
		UNICODE_STRING     LC_Event;
		OBJECT_ATTRIBUTES  ObjAttr;
		RtlInitUnicodeString( &LC_Event, MALWFIND_PROC_LC_EVENT );
		InitializeObjectAttributes( &ObjAttr, &LC_Event, 0, NULL, NULL);
		Status = ZwCreateEvent( &g_MalwFind.DrvProc.hProcLC_EventHandle, EVENT_ALL_ACCESS, &ObjAttr, SynchronizationEvent, FALSE);
		if(!NT_SUCCESS( Status )) return Status;
	}

	if(!g_MalwFind.DrvProc.pProcLC_Event)
	{
		Status = ObReferenceObjectByHandle( g_MalwFind.DrvProc.hProcLC_EventHandle, EVENT_ALL_ACCESS, NULL, KernelMode, &g_MalwFind.DrvProc.pProcLC_Event, NULL);
		if(!NT_SUCCESS( Status )) return Status;
	}

	return STATUS_SUCCESS;	

}



void 
Drv_ProcLC_CloseEvent(void)
{
	KdPrint(("Drv_ProcLC_CloseEvent\n"));

	if(g_MalwFind.DrvProc.pProcLC_Event)
	{
		ObDereferenceObject( g_MalwFind.DrvProc.pProcLC_Event );
		g_MalwFind.DrvProc.pProcLC_Event = NULL;
	}

	if(g_MalwFind.DrvProc.hProcLC_EventHandle) 
	{
		ZwClose( g_MalwFind.DrvProc.hProcLC_EventHandle );
		g_MalwFind.DrvProc.hProcLC_EventHandle = NULL;
	}	

}	



NTSTATUS	
Drv_ProcLC_FetchData( char* pczOutBuffer, ULONG* pInBufferSize, ULONG ulOutBufferSize )
{
	KIRQL  Irql;
	PLIST_ENTRY   pListEntry   = NULL;
	PDRV_P_ENTRY  pProcLCEntry = NULL;
	NTSTATUS  Status = STATUS_SUCCESS;
	ULONG     ulBufferStepLength=0, ulLength=0, ulOutLength=0;
	char      czBuffer[20];

	if(!pczOutBuffer)
	{
		KdPrint(("pczOutBuffer == NULL"));
		return STATUS_INFO_LENGTH_MISMATCH;
	}
	if(pInBufferSize) 
	{
		ulLength = (*pInBufferSize);
		(*pInBufferSize) = 0;
	}

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return STATUS_UNSUCCESSFUL;
	}

	ExAcquireFastMutex( &g_MalwFind.DrvProc.LockProc );	
	while( !IsListEmpty( &g_MalwFind.DrvProc.ListHead_ProcLCEntry ))
	{	
		pListEntry   = RemoveTailList( &g_MalwFind.DrvProc.ListHead_ProcLCEntry );
		pProcLCEntry = (PDRV_P_ENTRY)pListEntry;		
		if(pProcLCEntry)
		{
			RtlZeroMemory( czBuffer, sizeof(czBuffer) );						
			RtlStringCchPrintfA( czBuffer, sizeof(czBuffer), "%d/%d^", pProcLCEntry->bCreate, pProcLCEntry->ulProcID );
			
			ulOutLength = strlen( czBuffer );
			RtlCopyMemory( (pczOutBuffer+ulBufferStepLength), czBuffer, ulOutLength );	

			ulBufferStepLength += ulOutLength;
			if(ulOutBufferSize <= (ulBufferStepLength+8))
			{
				Drv_ProcLC_Delete( pProcLCEntry );
				ExReleaseFastMutex( &g_MalwFind.DrvProc.LockProc );
				return STATUS_SUCCESS;
			}
			Drv_ProcLC_Delete( pProcLCEntry );
		}
	}

	if(pInBufferSize) (*pInBufferSize) = ulBufferStepLength;
	ExReleaseFastMutex( &g_MalwFind.DrvProc.LockProc );
	return STATUS_SUCCESS;
}



BOOLEAN 
Drv_ProcLC_Add( IN ULONG ulProcID, IN BOOLEAN bCreate )
{
	PDRV_P_ENTRY  pNewProcLCEntry = NULL;

	pNewProcLCEntry = (PDRV_P_ENTRY)ExAllocatePoolWithTag( NonPagedPool, sizeof(DRV_P_ENTRY), MALWFIND_NAME_TAG );
	if(!pNewProcLCEntry) return FALSE;
	
	RtlZeroMemory( pNewProcLCEntry, sizeof(DRV_P_ENTRY) );
	pNewProcLCEntry->ulProcID = ulProcID;
	pNewProcLCEntry->bCreate  = bCreate;

	InsertHeadList( &g_MalwFind.DrvProc.ListHead_ProcLCEntry, (PLIST_ENTRY)pNewProcLCEntry );
	return TRUE;

}


void Drv_ProcLC_Delete( IN PDRV_P_ENTRY pProcLCEntry )
{
	ASSERT( pProcLCEntry );
	if(!pProcLCEntry) return;

	ExFreePoolWithTag( pProcLCEntry, MALWFIND_NAME_TAG );
}


void FltDrv_ProcessNotify_Routine( IN HANDLE hParentProcID, IN HANDLE hProcID, IN BOOLEAN bCreate )
{
	NTSTATUS   Status = STATUS_SUCCESS;
	PEPROCESS  pParentProcess = NULL;
	PEPROCESS  pProcess       = NULL;
	WCHAR      wzProcessName[ MAX_PROCESS_LEN ];
	
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) return;

	RtlZeroMemory( wzProcessName, sizeof(wzProcessName) );
	Status = PsLookupProcessByProcessId( hProcID, &pProcess );
	if(NT_SUCCESS( Status ))
	{			
		GetProcessName( pProcess, wzProcessName, MAX_PROCESS_LEN );	
		/*
		if(bCreate) KdPrint(("\n[ %ws ]-[CreateProcess]",    wzProcessName ));
		else        KdPrint(("\n[ %ws ]-[TerminateProcess]", wzProcessName ));
		*/
	}

	// LifeCycle
	Drv_ProcLC_Add( (ULONG)hProcID, bCreate );
	
}



/******************************************************************************************************************************/



// 자체보호 프로세스 숨기기

BOOLEAN  
Protect_ProcessHide(void)
{	
	BOOLEAN      bRet = FALSE;
	PLIST_ENTRY  pCurrentLink=NULL, pPrevLink=NULL, pNextLink=NULL;
	PEPROCESS    pSysProcess = NULL, pProcess=NULL;
	WCHAR        wzProcessName[ MAX_PROCESS_LEN ];
	pSysProcess = (PEPROCESS)PsInitialSystemProcess;
	pProcess    = pSysProcess;
	ASSERT( pProcess );
	if(!pProcess) return FALSE;

	__try
	{
		pCurrentLink = (PLIST_ENTRY)((ULONG_PTR)pProcess + (ULONG_PTR)g_MalwFind.RefProc.ulActiveProcessLinks);
		ASSERT( pCurrentLink );
		if(!pCurrentLink)  return FALSE;
		pCurrentLink = pCurrentLink->Flink;

		do
		{
			if(!pCurrentLink) break;			
			pProcess = (PEPROCESS)((ULONG_PTR)pCurrentLink - (ULONG_PTR)g_MalwFind.RefProc.ulActiveProcessLinks);
			ASSERT( pProcess );
			if(!pProcess) break;
			
			RtlZeroMemory( wzProcessName, sizeof(wzProcessName) );
			GetProcessName( pProcess, wzProcessName, MAX_PROCESS_LEN );

			if (!_wcsnicmp(wzProcessName, g_pEXE_MALWFIND_AGENT, wcslen(g_pEXE_MALWFIND_AGENT)))
			{
				pNextLink = pCurrentLink->Flink;
				pPrevLink = pCurrentLink->Blink;
				if(pPrevLink)  pPrevLink->Flink = pNextLink;
				if(pNextLink)  pNextLink->Blink = pPrevLink;
				KdPrint(("ProcessName=%ws  ProcessHide SuccessFully. \n", wzProcessName ));				
				bRet = TRUE;
			}			
			pCurrentLink = pCurrentLink->Flink;

		} while(pSysProcess != pProcess);

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("Protect_ProcessHide Occured. \n"));
	}

	return bRet;

}




/*****************************************************************************************************************/

