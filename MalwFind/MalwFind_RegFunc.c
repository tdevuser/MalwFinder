
#include "MalwFind_DataTypes.h"
#include "MalwFind.h"
#include "MalwFind_FastIoFunc.h"
#include "MalwFind_FileFunc.h"
#include "MalwFind_RegFunc.h"
#include "MalwFind_Command.h"
#include "MalwFind_Util.h"
#include "MalwFind_Acctl.h"



extern G_MALW_FIND  g_MalwFind;




#if (NTDDI_VERSION >= NTDDI_WINXP)

NTSTATUS   
RegFuncRegister( IN PDRIVER_OBJECT pDriverObject )
{
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER( pDriverObject );

	Status = CmRegisterCallback( RegFunc, NULL, &g_MalwFind.lnRegCookie );
	if(!NT_SUCCESS( Status ))
	{
		KdPrint(("CmRegisterCallBack Failed. [ 0x%x ] \n", Status ));
	}
	return Status;
}


NTSTATUS  
RegFuncUnregister(void)
{	
	NTSTATUS Status = STATUS_SUCCESS;

	Status = CmUnRegisterCallback( g_MalwFind.lnRegCookie );

	return Status;
}


NTSTATUS  
RegFunc(IN PVOID CallbackContext, IN PVOID Argument1, IN PVOID Argument2)
{
	NTSTATUS  Status = STATUS_SUCCESS;
	REG_NOTIFY_CLASS RegType = (REG_NOTIFY_CLASS)Argument1;
	NAME_BUFFER  RegPath = {0,};

	switch(RegType)
	{	
	case RegNtPreCreateKey:  
		{
			PREG_PRE_CREATE_KEY_INFORMATION  pPreCreateKey = (PREG_PRE_CREATE_KEY_INFORMATION)Argument2;
			ASSERT( pPreCreateKey );
			if(pPreCreateKey)
			{				
			}
		}
		break;

	case RegNtPreCreateKeyEx:  				
		{			 
			PREG_CREATE_KEY_INFORMATION pCreateKeyEx = (PREG_CREATE_KEY_INFORMATION)Argument2;
			if(pCreateKeyEx)
			{	
				Status = RegProc_CreateKey( NULL, pCreateKeyEx->RootObject, pCreateKeyEx->CompleteName );
			}
		}
		break;

	case RegNtPreOpenKey:  		
		{
			PREG_PRE_OPEN_KEY_INFORMATION  pPreOpenKey = (PREG_PRE_OPEN_KEY_INFORMATION)Argument2;
			if(pPreOpenKey)
			{
			}
		}
		break;

	case RegNtPreOpenKeyEx:  
		{
			PREG_OPEN_KEY_INFORMATION  pPreOpenKeyEx = (PREG_OPEN_KEY_INFORMATION)Argument2;
			if(pPreOpenKeyEx)
			{		
				Status = RegProc_OpenKey( NULL, pPreOpenKeyEx->RootObject, pPreOpenKeyEx->CompleteName, 0 );
			}
		}
		break;
	
	case RegNtPreDeleteKey:
		{
			PREG_DELETE_KEY_INFORMATION  pDeleteKey = (PREG_DELETE_KEY_INFORMATION)Argument2;
			if(pDeleteKey)
			{		
				Status = RegProc_DeleteKey( NULL, pDeleteKey->Object, NULL );
			}			
		}
		break;

	default:  break;
	}

	return Status;
}



#endif  // (NTDDI_VERSION >= NTDDI_WINXP)







ULONG  
GetRegFullPath( HANDLE hKey, PVOID pRootObject, PUNICODE_STRING pSubKey, PNAME_BUFFER pRegBuffer )
{	
	NTSTATUS                  Status      = STATUS_SUCCESS;
	PVOID                     pKey        = NULL;
	POBJECT_NAME_INFORMATION  pObjectName = NULL; 
	ULONG                     ulTempLen=0, ulRet=0, ulLength=0;
    
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		return 0;
	}
	if(!pRegBuffer || !pRegBuffer->pBuffer) 
	{
		return 0;
	}

	if(hKey) 
	{
		if(((ULONG_PTR)hKey>0) || (ExGetPreviousMode() == KernelMode))  
		{
			ObReferenceObjectByHandle( hKey, 0, NULL, KernelMode, &pKey, NULL);
		}
	}
	else if(pRootObject)
	{
		pKey = pRootObject;
		ObReferenceObject( pKey ); 
	}
	else
	{
		return 0;
	}
	if(!pKey) return 0;

	KeEnterCriticalRegion();
	
	pObjectName = (POBJECT_NAME_INFORMATION)ExAllocatePoolWithTag( NonPagedPool, MAX_POOL_LEN, MALWFIND_NAME_TAG );
	if(!pObjectName) 
	{
		KeLeaveCriticalRegion();
		return 0;
	}
		
	RtlZeroMemory( pObjectName, MAX_POOL_LEN );
	pObjectName->Name.Length        = 0;
	pObjectName->Name.MaximumLength = (MAX_POOL_LEN >> 1);

	Status = ObQueryNameString( pKey, pObjectName, MAX_POOL_LEN, &ulRet );
	if(!NT_SUCCESS( Status ))
	{
		if(pObjectName) 
		{
			ExFreePoolWithTag( pObjectName, MALWFIND_NAME_TAG );
			pObjectName = NULL;
		}
		KeLeaveCriticalRegion();
		return 0;
	}
	ObDereferenceObject( pKey ); 
		
	ulLength += pObjectName->Name.Length;
	Status = RtlStringCchCatNW( pRegBuffer->pBuffer, pRegBuffer->ulMaxLength, pObjectName->Name.Buffer, pObjectName->Name.Length );  
	if(!NT_SUCCESS( Status ))
	{
		if(pObjectName) 
		{
			ExFreePoolWithTag( pObjectName, MALWFIND_NAME_TAG );
			pObjectName = NULL;
		}
		KeLeaveCriticalRegion();
		return 0;
	}
				
	if(pSubKey && pSubKey->Buffer && pSubKey->Length)
	{
		ulLength += sizeof(WCHAR);
		ulTempLen = sizeof(WCHAR);
		Status = RtlStringCchCatNW( pRegBuffer->pBuffer, pRegBuffer->ulMaxLength, L"\\", ulTempLen );

		ulLength += pSubKey->Length;
		Status = RtlStringCchCatNW( pRegBuffer->pBuffer, pRegBuffer->ulMaxLength, pSubKey->Buffer, pSubKey->Length );
	}
		
	if(pObjectName)
	{
		ExFreePoolWithTag( pObjectName, MALWFIND_NAME_TAG );
		pObjectName = NULL;
	}

	KeLeaveCriticalRegion();
	return ulLength;
		
}


NTSTATUS  
RegProc_CreateKey( HANDLE hKey, PVOID pRootObject, PUNICODE_STRING pSubKey )
{
	ULONG         ulLength = 0;
	NAME_BUFFER   RegPath  = {0};
	NTSTATUS      Status   = STATUS_SUCCESS;
	WCHAR         wzProcName[ MAX_PROCESS_LEN ] = L"";

	if(!hKey && !pRootObject) return Status;
	if(ExGetPreviousMode() == KernelMode) return Status;
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) return Status;
	
	if (FALSE == g_MalwFind.DrvConfig.bGlobalCtrl)
	{
		return Status;
	}

	RtlZeroMemory( wzProcName, sizeof(WCHAR)*MAX_PROCESS_LEN );
	GetProcessName( PsGetCurrentProcess(), wzProcName, MAX_PROCESS_LEN );
	// 전역기능 예외프로세스 판단
	if(ISExpProcList_PolicyAll( wzProcName )) 
	{
		return Status;
	}

	ALLOCATE_N_REG_POOL( RegPath );
	if(!RegPath.pBuffer) return Status;
	SET_REG_POOL_ZERO( RegPath );

	ulLength = GetRegFullPath( hKey, pRootObject, pSubKey, &RegPath );
	if(ulLength <= 0) 
	{
		FREE_N_REG_POOL( RegPath );
		return STATUS_SUCCESS;
	}

	_wcslwr( RegPath.pBuffer );	
	if( wcsstr(RegPath.pBuffer, SVCNAME_AGENT) || wcsstr(RegPath.pBuffer, SVCNAME_DRIVER) )
	{
		KdPrint(("RegProc_CreateKey=[%ws] \n", RegPath.pBuffer ));
		// SetupDir Control == FALSE:제어안함   TRUE: 
		if (g_MalwFind.DrvConfig.bGlobalCtrl)
		{
			Status = STATUS_ACCESS_DENIED;
			KdPrint(("Ret--STATUS_ACCESS_DENIED RegPath=%ws \n", RegPath.pBuffer));
		}			
	}
	FREE_N_REG_POOL( RegPath );
	return Status;
}


NTSTATUS  
RegProc_OpenKey( HANDLE hKey, PVOID pRootObject, PUNICODE_STRING pSubKey, ACCESS_MASK AccessMask )
{
	ULONG        ulLength = 0;
	NAME_BUFFER  RegPath  = {0};
	NTSTATUS     Status   = STATUS_SUCCESS;
	WCHAR        wzProcName[ MAX_PROCESS_LEN ] = L"";

	if(!hKey && !pRootObject) return Status;
	if(ExGetPreviousMode() == KernelMode) return Status;
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) return Status;

	if (FALSE == g_MalwFind.DrvConfig.bGlobalCtrl)
	{
		return Status;
	}

	RtlZeroMemory( wzProcName, sizeof(WCHAR)*MAX_PROCESS_LEN );
	GetProcessName( PsGetCurrentProcess(), wzProcName, MAX_PROCESS_LEN );
	// 전역기능 예외 프로세스 판단
	if(ISExpProcList_PolicyAll( wzProcName )) 
	{
		return Status;
	}

	ALLOCATE_N_REG_POOL( RegPath );
	if(!RegPath.pBuffer) return Status;
	SET_REG_POOL_ZERO( RegPath );

	ulLength = GetRegFullPath( hKey, pRootObject, pSubKey, &RegPath );
	if(ulLength <= 0) 
	{
		FREE_N_REG_POOL( RegPath );	
		return STATUS_SUCCESS;
	}

	_wcslwr( RegPath.pBuffer );
	if( wcsstr(RegPath.pBuffer, SVCNAME_AGENT) || wcsstr(RegPath.pBuffer, SVCNAME_DRIVER) )
	{
		KdPrint(("RegProc_OpenKey=[%ws] \n", RegPath.pBuffer ));
		// SetupDir Control == FALSE:제어안함   TRUE: 제어함 
		if (g_MalwFind.DrvConfig.bGlobalCtrl)
		{
			Status = STATUS_ACCESS_DENIED;
			KdPrint(("Ret--STATUS_ACCESS_DENIED RegPath=%ws \n", RegPath.pBuffer ));
		}			
	}
	FREE_N_REG_POOL( RegPath );	
	return Status;
}


NTSTATUS  
RegProc_DeleteKey( HANDLE hKey, PVOID pRootObject, PUNICODE_STRING pSubKey )
{
	ULONG        ulLength = 0;
	NAME_BUFFER  RegPath  = {0};
	NTSTATUS     Status   = STATUS_SUCCESS;
	WCHAR        wzProcName[ MAX_PROCESS_LEN ] = L"";

	if(!hKey && !pRootObject) return Status;
	if(ExGetPreviousMode() == KernelMode) return Status;
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) return Status;


	if (FALSE == g_MalwFind.DrvConfig.bGlobalCtrl)
	{
		return Status;
	}
	RtlZeroMemory( wzProcName, sizeof(WCHAR)*MAX_PROCESS_LEN );
	GetProcessName( PsGetCurrentProcess(), wzProcName, MAX_PROCESS_LEN );
	// 전역기능 예외프로세스 판단.
	if(ISExpProcList_PolicyAll( wzProcName )) 
	{
		return Status;
	}

	ALLOCATE_N_REG_POOL( RegPath );
	if(!RegPath.pBuffer) return Status;
	SET_REG_POOL_ZERO( RegPath );

	ulLength = GetRegFullPath( hKey, pRootObject, pSubKey, &RegPath );
	if(ulLength <= 0) 
	{
		FREE_N_REG_POOL( RegPath );	
		return STATUS_SUCCESS;
	}

	_wcslwr( RegPath.pBuffer );
	if( wcsstr(RegPath.pBuffer, SVCNAME_AGENT) || wcsstr(RegPath.pBuffer, SVCNAME_DRIVER) )
	{
		KdPrint(("RegProc_DeleteKey=[%ws] \n", RegPath.pBuffer ));
		// SetupDir Control == FALSE:제어안함   TRUE: 제어함 
		if (g_MalwFind.DrvConfig.bGlobalCtrl)
		{
			Status = STATUS_ACCESS_DENIED;
			KdPrint(("Ret--STATUS_ACCESS_DENIED RegPath=%ws \n", RegPath.pBuffer));
		}			
	}
	FREE_N_REG_POOL( RegPath );	
	return Status;

}



NTSTATUS  
RegProc_EnumValueKey( HANDLE  hKey,
					  PVOID   pRootObject,
					  KEY_VALUE_INFORMATION_CLASS  KeyValueInformationClass,
					  PVOID   pKeyValueInformation,
					  ULONG   Length,
					  PULONG  pResultLength  )
{

	ULONG       ulLength = 0;
	NAME_BUFFER RegPath  = {0};
	NTSTATUS    Status   = STATUS_SUCCESS;
	WCHAR       wzProcName[ MAX_PROCESS_LEN ] = L"";

	if(!hKey && !pRootObject) return Status;
	if(ExGetPreviousMode() == KernelMode) return Status;
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) return Status;


	if (FALSE == g_MalwFind.DrvConfig.bGlobalCtrl)
	{
		return Status;
	}
	RtlZeroMemory( wzProcName, sizeof(WCHAR)*MAX_PROCESS_LEN );
	GetProcessName( PsGetCurrentProcess(), wzProcName, MAX_PROCESS_LEN );

	// 전역기능 예외 프로세스 판단.
	if(ISExpProcList_PolicyAll( wzProcName )) 
	{
		return Status;
	}

	ALLOCATE_N_REG_POOL( RegPath );
	if(!RegPath.pBuffer) return Status;
	SET_REG_POOL_ZERO( RegPath );

	ulLength = GetRegFullPath( hKey, pRootObject, NULL, &RegPath );
	if(ulLength <= 0) 
	{
		FREE_N_REG_POOL( RegPath );
		return STATUS_SUCCESS;
	}

	if(KeyValueInformationClass == KeyValueBasicInformation)
	{
	}
	else if(KeyValueInformationClass == KeyValueFullInformation)
	{
	}
	else if(KeyValueInformationClass == KeyValuePartialInformation)
	{
	}

	FREE_N_REG_POOL( RegPath );
	return Status;

}




/*
NTSTATUS  
RegFunc(IN PVOID CallbackContext, IN PVOID Argument1, IN PVOID Argument2)
{
	NTSTATUS  Status = STATUS_SUCCESS;

	REG_NOTIFY_CLASS RegType = (REG_NOTIFY_CLASS)Argument1;
	switch(RegType)
	{
//  case RegNtDeleteKey:
	case RegNtPreDeleteKey: // = RegNtDeleteKey,
		KdPrint(("RegNtPreDeleteKey \n"));  
		break;
		
//	case RegNtSetValueKey:
	case RegNtPreSetValueKey: // = RegNtSetValueKey,
		KdPrint(("RegNtPreSetValueKey \n"));  
		break;

//  case RegNtDeleteValueKey:
	case RegNtPreDeleteValueKey: // = RegNtDeleteValueKey,
		KdPrint(("RegNtPreDeleteValueKey \n"));  
		break;

//	case RegNtSetInformationKey:
	case RegNtPreSetInformationKey: // = RegNtSetInformationKey,
		KdPrint(("RegNtPreSetInformationKey \n"));   
		break;

//	case RegNtRenameKey:
	case RegNtPreRenameKey: // = RegNtRenameKey,
		KdPrint(("RegNtPreRenameKey \n"));   
		break;

//	case RegNtEnumerateKey:
	case RegNtPreEnumerateKey: // = RegNtEnumerateKey,
		KdPrint(("RegNtPreEnumerateKey \n"));   
		break;

//	case RegNtEnumerateValueKey:  
	case RegNtPreEnumerateValueKey: // = RegNtEnumerateValueKey,
		KdPrint(("RegNtPreEnumerateValueKey \n"));   
		break;

//	case RegNtQueryKey:  
	case RegNtPreQueryKey: // = RegNtQueryKey,
		KdPrint(("RegNtPreQueryKey \n"));   
		break;

//	case RegNtQueryValueKey:  
	case RegNtPreQueryValueKey: // = RegNtQueryValueKey,
		KdPrint(("RegNtPreQueryValueKey \n"));   
		break;

//	case RegNtQueryMultipleValueKey:  
	case RegNtPreQueryMultipleValueKey: // = RegNtQueryMultipleValueKey,
		KdPrint(("RegNtPreQueryMultipleValueKey \n"));   
		break;

	case RegNtPreCreateKey:  
		KdPrint(("RegNtPreCreateKey \n"));  
		{
			PREG_PRE_CREATE_KEY_INFORMATION  pPreCreateKey = (PREG_PRE_CREATE_KEY_INFORMATION)Argument2;
			ASSERT( pPreCreateKey );
			if(pPreCreateKey)
			{
			}
		}
		break;

	case RegNtPostCreateKey:  
		KdPrint(("RegNtPostCreateKey \n"));  
		{
			PREG_POST_CREATE_KEY_INFORMATION pPostCreateKey = (PREG_POST_CREATE_KEY_INFORMATION)Argument2;
			ASSERT( pPostCreateKey );
			if(pPostCreateKey)
			{
			}
		}
		break;

	case RegNtPreOpenKey:  
		KdPrint(("RegNtPreOpenKey \n"));   
		{
			PREG_PRE_OPEN_KEY_INFORMATION  pPreOpenKey = (PREG_PRE_OPEN_KEY_INFORMATION)Argument2;
			ASSERT( pPreOpenKey );
			if(pPreOpenKey)
			{
			}
		}
		break;

	case RegNtPostOpenKey:  
		KdPrint(("RegNtPostOpenKey \n"));   
		{
			PREG_POST_OPEN_KEY_INFORMATION  pPostOpenKey = (PREG_POST_OPEN_KEY_INFORMATION)Argument2;
			ASSERT( pPostOpenKey );
			if(pPostOpenKey)
			{
			}
		}
		break;

//	case RegNtKeyHandleClose:  
	case RegNtPreKeyHandleClose: 
		KdPrint(("RegNtPreKeyHandleClose \n"));   
		break;

	//
	// The following values apply only to Microsoft Windows Server 2003 and later.
	//    
	case RegNtPostDeleteKey:  
		KdPrint(("RegNtPostDeleteKey \n"));  
		break;

	case RegNtPostSetValueKey:  
		KdPrint(("RegNtPostSetValueKey \n"));   
		break;

	case RegNtPostDeleteValueKey:  
		KdPrint(("RegNtPostDeleteValueKey \n"));   
		break;

	case RegNtPostSetInformationKey:  
		KdPrint(("RegNtPostSetInformationKey \n"));   
		break;

	case RegNtPostRenameKey:  
		KdPrint(("RegNtPostRenameKey \n"));   
		break;

	case RegNtPostEnumerateKey:  
		KdPrint(("RegNtPostEnumerateKey \n"));  
		break;

	case RegNtPostEnumerateValueKey:  
		KdPrint(("RegNtPostEnumerateValueKey \n"));   
		break;

	case RegNtPostQueryKey:  
		KdPrint(("RegNtPostQueryKey \n"));   
		break;

	case RegNtPostQueryValueKey:  
		KdPrint(("RegNtPostQueryValueKey \n"));   
		break;

	case RegNtPostQueryMultipleValueKey:  
		KdPrint(("RegNtPostQueryMultipleValueKey \n"));   
		break;

	case RegNtPostKeyHandleClose:  
		KdPrint(("RegNtPostKeyHandleClose \n"));  
		break;

	case RegNtPreCreateKeyEx:  		
		KdPrint(("RegNtPreCreateKeyEx \n")); 
		{			 
			PREG_CREATE_KEY_INFORMATION pCreateKeyEx = (PREG_CREATE_KEY_INFORMATION)Argument2;
			ASSERT( pCreateKeyEx );
			if(pCreateKeyEx)
			{				
			}
		}
		break;

	case RegNtPostCreateKeyEx:  
		KdPrint(("RegNtPostCreateKeyEx \n")); 
		{
			PREG_POST_OPERATION_INFORMATION  pPostCreateKeyEx = (PREG_POST_OPERATION_INFORMATION)Argument2;
			ASSERT( pPostCreateKeyEx );
			if(pPostCreateKeyEx)
			{
			}
		}
		break;

	case RegNtPreOpenKeyEx:  
		KdPrint(("RegNtPreOpenKeyEx \n"));   
		{
			PREG_OPEN_KEY_INFORMATION  pPreOpenKeyEx = (PREG_OPEN_KEY_INFORMATION)Argument2;
			ASSERT( pPreOpenKeyEx );
			if(pPreOpenKeyEx)
			{
			}
		}
		break;

	case RegNtPostOpenKeyEx:  
		KdPrint(("RegNtPostOpenKeyEx \n"));  
		{
			PREG_POST_OPERATION_INFORMATION  pPostOpenKeyEx = (PREG_POST_OPERATION_INFORMATION)Argument2;
			ASSERT( pPostOpenKeyEx );
			if(pPostOpenKeyEx)
			{
			}
		}
		break;

	//
	// The following values apply only to Microsoft Windows Vista and later.
	//    
	case RegNtPreFlushKey: 
		KdPrint(("RegNtPreFlushKey \n"));  
		break;

	case RegNtPostFlushKey:  
		KdPrint(("RegNtPostFlushKey \n"));  
		break;

	case RegNtPreLoadKey:  
		KdPrint(("RegNtPreLoadKey \n"));   
		break;

	case RegNtPostLoadKey:  
		KdPrint(("RegNtPostLoadKey \n"));   
		break;

	case RegNtPreUnLoadKey:  
		KdPrint(("RegNtPreUnLoadKey \n"));   
		break;

	case RegNtPostUnLoadKey:  
		KdPrint(("RegNtPostUnLoadKey \n"));   
		break;

	case RegNtPreQueryKeySecurity:  
		KdPrint(("RegNtPreQueryKeySecurity \n"));   
		break;

	case RegNtPostQueryKeySecurity:  
		KdPrint(("RegNtPostQueryKeySecurity \n"));  
		break;

	case RegNtPreSetKeySecurity:  
		KdPrint(("RegNtPreSetKeySecurity \n"));   
		break;

	case RegNtPostSetKeySecurity:  
		KdPrint(("RegNtPostSetKeySecurity \n"));   
		break;

	case RegNtCallbackObjectContextCleanup:  
		KdPrint(("RegNtCallbackObjectContextCleanup \n"));   
		break;

	default:
		KdPrint(("REG_NOTIFY_CLASS Unknown \n"));
		break;
	}

	return Status;

}


*/
