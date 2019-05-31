#ifndef _MALWFIND_REG_FUNC_H_
#define _MALWFIND_REG_FUNC_H_

#include <ntdef.h>
#include <ntifs.h>
#include "MalwFind_DataTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif


#if (NTDDI_VERSION >= NTDDI_WINXP)

NTSTATUS  
RegFuncRegister(PDRIVER_OBJECT pDriverObject);

NTSTATUS  
RegFuncUnregister();

NTSTATUS  
RegFunc(IN PVOID CallbackContext, IN PVOID Argument1, IN PVOID Argument2 );


#endif // NTDDI_VERSION >= NTDDI_WINXP



ULONG  
GetRegFullPath( HANDLE hKey, PVOID pRootObject, PUNICODE_STRING pSubKey, PNAME_BUFFER pRegBuffer );



NTSTATUS  
RegProc_CreateKey( HANDLE hKey, PVOID pRootObject, PUNICODE_STRING pSubKey );

NTSTATUS  
RegProc_OpenKey( HANDLE hKey, PVOID pRootObject, PUNICODE_STRING pSubKey, ACCESS_MASK AccessMask );

NTSTATUS  
RegProc_DeleteKey( HANDLE hKey, PVOID pRootObject, PUNICODE_STRING pSubKey );


NTSTATUS  
RegProc_EnumValueKey( HANDLE  hKey,
					  PVOID   pRootObject,
					  KEY_VALUE_INFORMATION_CLASS  KeyValueInformationClass,
					  PVOID   pKeyValueInformation,
					  ULONG   Length,
					  PULONG  pResultLength  );

/*******************************************************************************/
// Reg PagedPool
/*******************************************************************************/
#define ALLOCATE_N_REG_POOL( RegPath ) \
	     RegPath.pBuffer = (PWCHAR)ExAllocateFromNPagedLookasideList( &g_MalwFind.RegNPagedList )


#define FREE_N_REG_POOL( RegPath ) \
		 ExFreeToNPagedLookasideList( &g_MalwFind.RegNPagedList, RegPath.pBuffer ); \
		 RegPath.pBuffer = NULL; \
		 RegPath.pFilePos = NULL; \
		 RegPath.ulLength = 0;  \
	     RegPath.ulMaxLength = 0 


#define SET_REG_POOL_ZERO( RegPath )  \
	RegPath.pFilePos    = NULL; \
	RegPath.ulFile      = MFOBJ_NONE; \
	RegPath.ulMaxLength = MAX_POOL_LEN >> 1 ; \
	RegPath.ulLength    = 0; \
	RtlZeroMemory( RegPath.pBuffer, MAX_POOL_LEN )



#ifdef __cplusplus
};
#endif



#endif