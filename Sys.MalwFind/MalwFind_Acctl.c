
#include "MalwFind_DataTypes.h"
#include "MalwFind.h"
#include "MalwFind_Process.h"
#include "MalwFind_Acctl.h"
#include "MalwFind_LogFunc.h"
#include "MalwFind_DispHook.h"
#include "MalwFind_Util.h"

extern G_MALW_FIND  g_MalwFind;
extern const WCHAR* g_pExpProcName[];
const WCHAR*        g_pEXE_MALWFIND_AGENT;

/*
#define MALWF_ACCESS       0x00000000
#define MALWF_CREATE       (2  <<  1)
#define MALWF_OPEN         (2  <<  2)
#define MALWF_READ         (2  <<  3)
#define MALWF_WRITE        (2  <<  4)
#define FLT_EXECUTE      (2  <<  5)
#define MALWF_DELETE       (2  <<  6)
#define MALWF_RENAME       (2  <<  7)
#define FLT_REPLACE      (2  <<  8)
#define FLT_REPARSE      (2  <<  9) 
#define FLT_SET_SECURITY (2  << 10) 
#define FLT_SET_LINK     (2  << 11) 
#define FLT_MAX          0xFFFFFFFF
*/


// 전역기능 예외 프로세스
const WCHAR* g_pExpProcAll[] = 
{ 
	L"MalwFind.exe",	
	NULL
};

// 자체보호 예외 프로세스 
const WCHAR* g_pExpProcSelfProtect[] = 
{ 
	L"MalwFind.exe",
	NULL
};



// 모든기능 예외프로세스
BOOLEAN ISExpProcList_PolicyAll( IN PWCHAR pwzProcName )
{
	int nIndex = 0;
	if(!pwzProcName) return FALSE;

	if(!_wcsicmp( pwzProcName, L"winlogon.exe" ) || 
	   !_wcsicmp( pwzProcName, L"smss.exe")      ||
	   !_wcsicmp( pwzProcName, L"csrss.exe")     ||
	   !_wcsicmp( pwzProcName, L"lsass.exe")     || 
	   !_wcsicmp( pwzProcName, L"services.exe")  ||
	   !_wcsicmp( pwzProcName, L"smupt.exe")     ||
	   !_wcsicmp( pwzProcName, L"updater.exe")   ||
	   !_wcsicmp( pwzProcName, L"appatchagent.exe") ||
	   !_wcsicmp( pwzProcName, L"appatchtray.exe")  ||
	   !_wcsicmp( pwzProcName, L"nhcaflttest.exe")  ||
	   !_wcsicmp(pwzProcName,  g_pEXE_MALWFIND_AGENT))
	{
		return TRUE;
	}

	if(!_wcsnicmp( pwzProcName, L"nhca", wcslen( L"nhca" ) ))
	{
		return TRUE;
	}

	if(!_wcsnicmp( pwzProcName, L"windowssecuse", wcslen( L"windowssecuse") ))
	{
		return TRUE;
	}
	
	nIndex = 0;
	while(1)
	{
		if(!g_pExpProcAll[ nIndex ]) 
		{
			break;
		}
		if(!_wcsicmp(pwzProcName, g_pExpProcAll[ nIndex ]))
		{
			return TRUE;
		}
		nIndex++;
	}
	
	return FALSE;
}



// 자체보호 스킵 예외프로세스
BOOLEAN  
ISExpProcList_PolicySelfProtect( IN PWCHAR pwzProcName )
{
	int nIndex = 0;
	if(!pwzProcName) return FALSE;

	if(!_wcsicmp( pwzProcName, L"winlogon.exe" ) || 
	   !_wcsicmp( pwzProcName, L"smss.exe")      ||
	   !_wcsicmp( pwzProcName, L"csrss.exe")     ||
	   !_wcsicmp( pwzProcName, L"lsass.exe")     || 
	   !_wcsicmp( pwzProcName, L"services.exe")  ||
	   !_wcsicmp( pwzProcName, L"smupt.exe")     ||
	   !_wcsicmp( pwzProcName, L"updater.exe")   ||
	   !_wcsicmp( pwzProcName, L"appatchagent.exe") ||
	   !_wcsicmp( pwzProcName, L"appatchtray.exe")  ||
	   !_wcsicmp( pwzProcName, L"nhcaflttest.exe")  ||
	   !_wcsicmp(pwzProcName, g_pEXE_MALWFIND_AGENT))
	{
		return TRUE;
	}

	if(!_wcsnicmp( pwzProcName, L"nhca", wcslen( L"nhca" ) ))
	{
		return TRUE;
	}

	if(!_wcsnicmp( pwzProcName, L"windowssecuse", wcslen( L"windowssecuse") ))
	{
		return TRUE;
	}
	
	nIndex = 0;
	while(1)
	{
		if(!g_pExpProcAll[ nIndex ]) 
		{
			break;
		}
		if(!_wcsicmp(pwzProcName, g_pExpProcAll[ nIndex ]))
		{
			return TRUE;
		}
		nIndex++;
	}
	
	return FALSE;
}



ULONG GetVolumeFlag( WCHAR cVol )
{
	int    nVolIndex    = 0xFF;
	ULONG  ulVolumeFlag = 0;

	nVolIndex = (int)(cVol - L'A' );
	if(nVolIndex < 0 || nVolIndex >= MAX_VOL_NUMS) return 0;
	
	ulVolumeFlag = (ULONG)(1 << nVolIndex);
	return ulVolumeFlag;
}


NTSTATUS  
Control_FP_SFolder( IN PWCHAR          pwzProcName, 
				    IN PNAME_BUFFER    pFileName, 
				    IN PULONG          pulDisposition,
				    IN PACCESS_MASK    pulDesiredAccess,
				    IN PLOG_HDR        pLogHdr  )
{
	BOOLEAN  bExist = FALSE;
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG    ulDispos=0, ulAccess=0;

	ASSERT( pwzProcName && pFileName && pFileName->pBuffer  );
	if(!pwzProcName || !pFileName || !pFileName->pBuffer  )
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> Control_FP_SFolder \n")); 
		return STATUS_INVALID_PARAMETER;
	}
	if(pulDisposition)   ulDispos = (*pulDisposition);
	if(pulDesiredAccess) ulAccess = (*pulDesiredAccess);

	//공유폴더 제어정책
	Status = Control_SF_SFolder( pwzProcName, pFileName, ulAccess, pLogHdr );
	if(STATUS_ACCESS_DENIED == Status) 
	{
		pLogHdr->ulLogID = MFLOG_SFOLDER;
		return Status;
	}

	return Status;

}


NTSTATUS 
Control_FP_Volume( IN PWCHAR  pwzProcName, IN PNAME_BUFFER  pFileName, IN PULONG pulDisposition, IN PACCESS_MASK pulDesiredAccess, IN PLOG_HDR pLogHdr )
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG    ulDispos=0, ulAccess=0;
	
	if(!pwzProcName || !pFileName || !pFileName->pBuffer  ) 
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> Control_FP_Volume \n")); 
		return STATUS_INVALID_PARAMETER;
	}

	if(pulDisposition)   ulDispos = (*pulDisposition);
	if(pulDesiredAccess) ulAccess = (*pulDesiredAccess);
	
	//프로세스 제어정책
	Status = Control_P_Process( pwzProcName, pFileName, ulAccess, pLogHdr );	
	if(STATUS_ACCESS_DENIED == Status) 
	{
		pLogHdr->ulLogID = MFLOG_PROCESS;
		return Status;
	}
	
	//Fdd 제어정책
	Status = Control_F_FDD( pwzProcName,  pFileName, ulAccess, pLogHdr );
	if(STATUS_ACCESS_DENIED == Status) 
	{
		pLogHdr->ulLogID = MFLOG_FDD;
		return Status;
	}

	//USB 제어정책
	Status = Control_F_Usb( pwzProcName, pFileName, ulAccess, pLogHdr ); 	
	if(STATUS_ACCESS_DENIED == Status)
	{
		pLogHdr->ulLogID = MFLOG_USB;
	}

	return Status;
}


NTSTATUS  
Control_SF_SFolder( IN PWCHAR pwzProcName, IN PNAME_BUFFER  pFileName, IN ULONG ulAccess, IN PLOG_HDR pLogHdr )
{
	BOOLEAN  bExist  = FALSE;
	ULONG    ulAcctl = 0;

	if(!pwzProcName )
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> Control_SF_SFolder \n")); 
		return STATUS_INVALID_PARAMETER;
	}

	//공유폴더 예외프로세스
	bExist = ISExist_SF_SFolderExceptProc( pwzProcName );
	if(bExist)
	{
		return STATUS_SUCCESS;
	}

	//공유폴더 예외폴더
	if(pFileName && pFileName->pBuffer)
	{
		bExist = ISExist_SF_SFolderExceptDir( pFileName->pBuffer );
		if(bExist)
		{
			return STATUS_SUCCESS;
		}
	}

	ulAcctl = (g_MalwFind.DrvSFolder.Policy.ulAcctl & ulAccess);
	if(!ulAcctl)
	{
		return STATUS_SUCCESS;
	}

	pLogHdr->ulLogID = MFLOG_SFOLDER;
	return STATUS_ACCESS_DENIED;

}


// 공유폴더에서 확장자 제어를 하기 위함이다.
NTSTATUS  
Control_SF_FileExt( IN PWCHAR  pwzProcName, IN PNAME_BUFFER  pFileName, IN ULONG nDisposition, IN ACCESS_MASK ulAccess, IN PLOG_HDR pLogHdr )
{
	PWCHAR     pExtPos  = NULL;
	BOOLEAN    bExist   = FALSE;
	ULONG      ulAcctl  = MALWF_ACCESS;

	if(!pFileName || !pFileName->pBuffer || !pwzProcName)
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> Control_SF_FileExt  FilePath=%ws \n", pFileName->pBuffer ));
		return STATUS_INVALID_PARAMETER;
	}
	
	// 확장자 검색
	pExtPos = wcsrchr(pFileName->pBuffer, L'.');	
	if(!pExtPos) return STATUS_SUCCESS;

	// 확장자제어 차단리스트 검색
	bExist = ISExist_F_FileExtCtrl( pExtPos );
	if(TRUE == g_MalwFind.DrvFile.bExtDeny)   // TRUE:  (BlackList) 리스트 차단함 
	{ 
		if(!bExist) 
		{
			return STATUS_SUCCESS;
		}
	}
	else if(FALSE == g_MalwFind.DrvFile.bExtDeny) // FALSE: (WhiteList) 리스트만 허용함 나머지 차단
	{   
		if(bExist) 
		{
			return STATUS_SUCCESS;
		}
	}
    
	bExist = ISExist_F_ExceptProc( pwzProcName );
	if(bExist)
	{
		return STATUS_SUCCESS;
	}

	bExist = ISExist_F_ExceptDir( pFileName->pBuffer );
	if(bExist)
	{
		return STATUS_SUCCESS;
	}

	ulAcctl = (g_MalwFind.DrvFile.FileExtPolicy.ulAcctl & ulAccess);
	if(!ulAcctl)
	{
		return STATUS_SUCCESS;
	}

	pLogHdr->ulLogID = MFLOG_DLP;
	return STATUS_ACCESS_DENIED;
}




NTSTATUS 
Control_P_Process( IN PWCHAR  pwzProcName, IN PNAME_BUFFER  pFileName, IN ULONG ulAccess, IN PLOG_HDR pLogHdr )
{
	BOOLEAN  bExist  = FALSE;
	ULONG    ulAcctl = MALWF_ACCESS;
	ULONG    ulVolFlag=0, ulVolExist=0;

	ulVolFlag  = GetVolumeFlag( pFileName->pBuffer[0] );
	ulVolExist = (g_MalwFind.DrvProc.Policy.Vol.ulDrives & ulVolFlag);
	if(!ulVolExist) 
	{
		return STATUS_SUCCESS;
	}

	// 프로세스 차단리스트 검색
	bExist  = ISExist_P_ProcCtrl( pwzProcName );
	if(TRUE == g_MalwFind.DrvProc.bProcDeny)   // TRUE:  (BlackList) 리스트 차단함 
	{ 
		if(!bExist) return STATUS_SUCCESS;
	}
	else if(FALSE == g_MalwFind.DrvProc.bProcDeny) // FALSE: (WhiteList) 리스트만 허용함 나머지 차단
	{   
		if(bExist) return STATUS_SUCCESS;
	}


	bExist = ISExist_P_ExceptDir( pFileName->pBuffer );
	if(bExist) 
	{ // 예외 폴더이므로 패스한다. 
		return STATUS_SUCCESS;
	}

	ulAcctl = (g_MalwFind.DrvProc.Policy.ulAcctl & ulAccess);
	if(!ulAcctl) 
	{
		return STATUS_SUCCESS;
	}

	KdPrint(("[Deny] Control_P_Process ProcName=%ws FileName=%ws \n", pwzProcName, pFileName->pBuffer ));

	pLogHdr->ulLogID = MFLOG_PROCESS;
	return STATUS_ACCESS_DENIED;

}



NTSTATUS 
Control_F_FDD( IN PWCHAR pwzProcName, IN PNAME_BUFFER pFileName, IN ULONG ulAccess, IN PLOG_HDR pLogHdr )
{
	ULONG  ulAcctl=0, ulVolFlag=0, ulVolExist=0;

	ulVolFlag  = GetVolumeFlag( pFileName->pBuffer[0] );
	ulVolExist = (g_MalwFind.DrvFile.FddPolicy.Vol.ulDrives & ulVolFlag);
	if(!ulVolExist) 
	{
		return STATUS_SUCCESS;
	}

	ulAcctl = (g_MalwFind.DrvFile.FddPolicy.ulAcctl & ulAccess);
	if(!ulAcctl)
	{
		return STATUS_SUCCESS;
	}

	pLogHdr->ulLogID = MFLOG_FDD;
	return STATUS_ACCESS_DENIED;
}


//


NTSTATUS 
Control_F_Usb( IN PWCHAR  pwzProcName, IN PNAME_BUFFER  pFileName, IN ULONG ulAccess, IN PLOG_HDR pLogHdr )
{
	BOOLEAN  bUsbEncrypt = FALSE;
	ULONG    ulAcctl=0, ulVolFlag=0, ulVolExist=0;

	ulVolFlag  = GetVolumeFlag( pFileName->pBuffer[0] );
	ulVolExist = (g_MalwFind.DrvFile.UsbPolicy.Vol.ulDrives & ulVolFlag);
	if(!ulVolExist) 
	{
		return STATUS_SUCCESS;
	}

	ulAcctl = (g_MalwFind.DrvFile.UsbPolicy.ulAcctl & ulAccess);
	if(!ulAcctl)
	{
		return STATUS_SUCCESS;
	}

	KdPrint(("[ Deny ] Control_F_Usb >> ProcName=%ws, FilePath=%ws \n", pwzProcName, pFileName->pBuffer ));
	pLogHdr->ulLogID = MFLOG_USB;
	return STATUS_ACCESS_DENIED;
}




NTSTATUS  
Control_F_FileExt( IN PWCHAR pwzProcName, IN PNAME_BUFFER pFileName, IN ULONG nDisposition, IN ACCESS_MASK ulAccess, IN PLOG_HDR pLogHdr )
{
	PWCHAR     pExtPos  = NULL;
	PFILE_EXT  pFindExt = NULL;
	BOOLEAN    bExist   = FALSE;
	ULONG      ulAcctl  = MALWF_ACCESS;
	ULONG      ulVolFlag= 0, ulVolExist = 0;

	if(!pFileName || !pFileName->pBuffer || !pwzProcName)
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> Control_F_FileExt FilePath=%ws \n", pFileName->pBuffer ));
		return STATUS_INVALID_PARAMETER;
	}
	
	ulVolFlag  = GetVolumeFlag( pFileName->pBuffer[0] );
	ulVolExist = (g_MalwFind.DrvFile.FileExtPolicy.Vol.ulDrives & ulVolFlag);
	if(!ulVolExist) 
	{
		return STATUS_SUCCESS;
	}
	
	// 확장자 검색
	pExtPos = wcsrchr( pFileName->pBuffer, L'.');	
	if(!pExtPos) return STATUS_SUCCESS;


	// 확장자제어 차단리스트 검색
	bExist = ISExist_F_FileExtCtrl( pExtPos );
	if(TRUE == g_MalwFind.DrvFile.bExtDeny)   // TRUE:  (BlackList) 리스트 차단함 
	{ 
		if(!bExist) return STATUS_SUCCESS;
	}
	else if(FALSE == g_MalwFind.DrvFile.bExtDeny) // FALSE: (WhiteList) 리스트만 허용함 나머지 차단
	{   
		if(bExist) return STATUS_SUCCESS;
	}

	bExist = ISExist_F_ExceptProc( pwzProcName );
	if(bExist)
	{
		return STATUS_SUCCESS;
	}

	bExist = ISExist_F_ExceptDir( pFileName->pBuffer );
	if(bExist)
	{
		return STATUS_SUCCESS;
	}
	
	// 확장자는 제어코드가 없다 무조건 차단
	ulAcctl = (g_MalwFind.DrvFile.FileExtPolicy.ulAcctl & ulAccess);
	if(!ulAcctl) return STATUS_SUCCESS;  

	KdPrint(("[Deny] Proc=%ws bExtDeny=%d, bExist=%d > FilePath=%ws \n", pwzProcName, g_MalwFind.DrvFile.bExtDeny, bExist, pFileName->pBuffer ));
	pLogHdr->ulLogID = MFLOG_DLP;
	return STATUS_ACCESS_DENIED;
}


NTSTATUS  
Control_F_FileName( IN PWCHAR pwzProcessName, IN PNAME_BUFFER  pFileName, IN ULONG nDisposition, IN ACCESS_MASK ulAccess, IN PLOG_HDR pLogHdr )
{
	BYTE           ucAcctl = 0;
	ULONG          ulCompareFlag=0x00, ulIndex=0x00;
	ULONG          ulRet=0x00, ulTemp=0x00;
	NTSTATUS       Status        = STATUS_SUCCESS;
	PDEF_BUFFER    pDefBuffer    = NULL;
	PLIST_ENTRY    pList         = NULL;
	ULONG          ulVolFlag=0, ulVolExist=0;

	if(!pFileName || !pFileName->pBuffer) 
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> Control_F_FileName  FilePath=%ws  \n", pFileName->pBuffer ));
		return STATUS_INVALID_PARAMETER;
	}

	ulVolFlag  = GetVolumeFlag( pFileName->pBuffer[0] );
	ulVolExist = (g_MalwFind.DrvFile.FileExtPolicy.Vol.ulDrives & ulVolFlag);
	if(!ulVolExist) 
	{
		return STATUS_SUCCESS;
	}

	return Status;

}


/******************************************************************************************************/
// PROCESS 제어 검색
/******************************************************************************************************/

// 프로세스 제어리스트 판별
BOOLEAN  ISExist_P_ProcCtrl( IN PWCHAR pwzProcName )
{
	BOOLEAN      bExist = FALSE;
	PCONTROL_OBJ pProcObj  = NULL;
	PLIST_ENTRY  pListNode = NULL;
	
	if(!pwzProcName) 
	{
		ASSERT( pwzProcName );
		return FALSE;
	}

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return FALSE;
	}

	ExAcquireFastMutex( &g_MalwFind.DrvProc.LockProc );
	pListNode = g_MalwFind.DrvProc.ListHead_ProcCtrl.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvProc.ListHead_ProcCtrl)
	{
		pProcObj = (PCONTROL_OBJ)pListNode;
		if(pProcObj)
		{
			bExist = Search_WildcardToken( pwzProcName, pProcObj->Obj.Buffer );
			if(bExist)
			{
				ExReleaseFastMutex( &g_MalwFind.DrvProc.LockProc );
				return TRUE;
			}
		}
		pListNode = pListNode->Flink;
	}
	ExReleaseFastMutex( &g_MalwFind.DrvProc.LockProc );
	return FALSE;
}



// 프로세스 예외폴더 판정( 동일폴더 판정 )
BOOLEAN 
ISExist_P_ExceptDir_Equal( IN PWCHAR pwzExceptPath )
{
	PLIST_ENTRY  pListNode = NULL;
	PCONTROL_OBJ pDirObj   = NULL;

	if(!pwzExceptPath)
	{
		return FALSE;
	}
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		return FALSE;
	}

	ExAcquireFastMutex( &g_MalwFind.DrvProc.LockProc );
	pListNode = g_MalwFind.DrvProc.ListHead_ExceptDir.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvProc.ListHead_ExceptDir)
	{
		pDirObj = (PCONTROL_OBJ)pListNode;
		if(pDirObj)
		{
			if(!_wcsicmp(pDirObj->Obj.Buffer, pwzExceptPath))
			{
				ExReleaseFastMutex( &g_MalwFind.DrvProc.LockProc );
				return TRUE;
			}
		}
		pListNode = pListNode->Flink;
	}
	ExReleaseFastMutex( &g_MalwFind.DrvProc.LockProc );
	return FALSE;
}



// 프로세스 예외폴더 판정
BOOLEAN ISExist_P_ExceptDir( IN PWCHAR pwzExceptPath )
{
	PLIST_ENTRY  pListNode = NULL;
	PCONTROL_OBJ pDirObj   = NULL;

	if(!pwzExceptPath)
	{
		ASSERT( pwzExceptPath );
		return FALSE;
	}

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return FALSE;
	}

	ExAcquireFastMutex( &g_MalwFind.DrvProc.LockProc );
	pListNode = g_MalwFind.DrvProc.ListHead_ExceptDir.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvProc.ListHead_ExceptDir)
	{
		pDirObj = (PCONTROL_OBJ)pListNode;
		if(pDirObj)
		{
			if(!_wcsnicmp(pDirObj->Obj.Buffer, pwzExceptPath, wcslen(pwzExceptPath)) || !_wcsnicmp(pwzExceptPath, pDirObj->Obj.Buffer, wcslen(pDirObj->Obj.Buffer)))
			{
				ExReleaseFastMutex( &g_MalwFind.DrvProc.LockProc );
				return TRUE;
			}
		}
		pListNode = pListNode->Flink;
	}
	ExReleaseFastMutex( &g_MalwFind.DrvProc.LockProc );
	return FALSE;
}



/******************************************************************************************************/
// 파일확장자 제어검색
/******************************************************************************************************/

// 확장자제어 차단리스트 검색
BOOLEAN  
ISExist_F_FileExtCtrl( IN PWCHAR pwzFileName )
{
	BOOLEAN      bExist = FALSE;
	PCONTROL_OBJ pFileExtObj = NULL;
	PLIST_ENTRY  pListNode   = NULL;
	
	if(!pwzFileName) return FALSE;	
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)  return FALSE;

	ExAcquireFastMutex( &g_MalwFind.DrvFile.LockFileExt );

	pListNode = g_MalwFind.DrvFile.ListHead_FileExt.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvFile.ListHead_FileExt)
	{
		pFileExtObj = (PCONTROL_OBJ)pListNode;
		if(pFileExtObj)
		{
			bExist = Search_WildcardToken( pwzFileName, pFileExtObj->Obj.Buffer );
			if(bExist)
			{
				ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
				return TRUE;
			}
		}
		pListNode = pListNode->Flink;
	}
	
	ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	return FALSE;
}


// 확장자 제어 예외 리스트 검색
BOOLEAN ISExist_F_FileExtCtrl_Except( IN PWCHAR pwzFileName )
{
	BOOLEAN      bExist = FALSE;
	PCONTROL_OBJ pFileExtObj = NULL;
	PLIST_ENTRY  pListNode   = NULL;

	if(!pwzFileName) return FALSE;	
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)  return FALSE;

	ExAcquireFastMutex( &g_MalwFind.DrvFile.LockFileExt );

	pListNode = g_MalwFind.DrvFile.ListHead_FileExtExcept.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvFile.ListHead_FileExtExcept)
	{
		pFileExtObj = (PCONTROL_OBJ)pListNode;
		if(pFileExtObj)
		{
			bExist = Search_WildcardToken( pwzFileName, pFileExtObj->Obj.Buffer );
			if(bExist)
			{
				ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
				return TRUE;
			}
		}
		pListNode = pListNode->Flink;
	}

	ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	return FALSE;
}


// 확장자 예외폴더 동일이름 판단
BOOLEAN  
ISExist_F_ExceptDir_Equal( IN PWCHAR pwzExceptPath )
{
	PCONTROL_OBJ pDirObj   = NULL;
	PLIST_ENTRY  pListNode = NULL;

	if(!pwzExceptPath) return FALSE;
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) return FALSE;


	ExAcquireFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	
	pListNode = g_MalwFind.DrvFile.ListHead_ExceptDir.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvFile.ListHead_ExceptDir)
	{
		pDirObj = (PCONTROL_OBJ)pListNode;
		if(pDirObj)
		{
			if( !_wcsicmp(pDirObj->Obj.Buffer, pwzExceptPath) )
			{
				ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
				return TRUE;
			}
		}
		pListNode = pListNode->Flink;
	}

	ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	return FALSE;
}


// 확장자 예외폴더 검색
BOOLEAN  
ISExist_F_ExceptDir( IN PWCHAR pwzExceptPath )
{
	PCONTROL_OBJ pDirObj   = NULL;
	PLIST_ENTRY  pListNode = NULL;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return FALSE;
	}
	if(!pwzExceptPath)
	{
		ASSERT( pwzExceptPath );
		return FALSE;
	}

	ExAcquireFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	pListNode = g_MalwFind.DrvFile.ListHead_ExceptDir.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvFile.ListHead_ExceptDir)
	{
		pDirObj = (PCONTROL_OBJ)pListNode;
		if(pDirObj)
		{
			if( !_wcsnicmp(pDirObj->Obj.Buffer, pwzExceptPath, wcslen(pwzExceptPath) ) || 
				!_wcsnicmp(pwzExceptPath, pDirObj->Obj.Buffer, wcslen(pDirObj->Obj.Buffer)) )
			{
				ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
				return TRUE;
			}
		}
		pListNode = pListNode->Flink;
	}
	ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	return FALSE;
}



// 확장자 예외 프로세스 검색
BOOLEAN 
ISExist_F_ExceptProc( IN PWCHAR pwzExceptProc )
{
	BOOLEAN      bExist    = FALSE;
	PCONTROL_OBJ pProcObj  = NULL;
	PLIST_ENTRY  pListNode = NULL;

	if(!pwzExceptProc)
	{
		ASSERT( pwzExceptProc );
		return FALSE;
	}

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return FALSE;
	}

	
	ExAcquireFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	pListNode = g_MalwFind.DrvFile.ListHead_ExceptProc.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvFile.ListHead_ExceptProc)
	{
		pProcObj = (PCONTROL_OBJ)pListNode;
		if(pProcObj)
		{
			bExist = Search_WildcardToken( pwzExceptProc, pProcObj->Obj.Buffer );
			if(bExist)
			{
				ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
				return TRUE;
			}
		}
		pListNode = pListNode->Flink;
	}
	ExReleaseFastMutex( &g_MalwFind.DrvFile.LockFileExt );
	return FALSE;
}



/******************************************************************************************************/
// 공유폴더 제어검색
/******************************************************************************************************/

// 공유폴더 제어리스트 검색

// 공유폴더제어 예외폴더 검색
BOOLEAN 
ISExist_SF_SFolderExceptDir( IN PWCHAR pwzExceptPath )
{
	PLIST_ENTRY   pListNode = NULL;
	PCONTROL_OBJ  pDirObj   = NULL;
	
	if(!pwzExceptPath) 
	{
		ASSERT( pwzExceptPath );
		return FALSE;
	}

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return FALSE;
	}

	ExAcquireFastMutex( &g_MalwFind.DrvSFolder.LockSFolder );
	pListNode = g_MalwFind.DrvSFolder.ListHead_ExceptDir.Flink;

	while(pListNode && pListNode != &g_MalwFind.DrvSFolder.ListHead_ExceptDir)
	{
		pDirObj = (PCONTROL_OBJ)pListNode;
		if(pDirObj)
		{
			if( !_wcsnicmp( pDirObj->Obj.Buffer, pwzExceptPath, wcslen(pwzExceptPath) ) ||
                !_wcsnicmp( pwzExceptPath, pDirObj->Obj.Buffer, wcslen(pDirObj->Obj.Buffer) ))
			{
				ExReleaseFastMutex( &g_MalwFind.DrvSFolder.LockSFolder );
				return TRUE;
			}
		}
		pListNode = pListNode->Flink;
	}
	ExReleaseFastMutex( &g_MalwFind.DrvSFolder.LockSFolder );
	return FALSE;

}

// 공유폴더제어 예외 프로세스 검색
BOOLEAN 
ISExist_SF_SFolderExceptProc( IN PWCHAR pwzProcName )
{
	BOOLEAN       bExist    = FALSE;
	PLIST_ENTRY   pListNode = NULL;
	PCONTROL_OBJ  pProcObj  = NULL;

	ASSERT( pwzProcName );
	if(!pwzProcName) return FALSE;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		ASSERT( FALSE );
		return FALSE;
	}

	ExAcquireFastMutex( &g_MalwFind.DrvSFolder.LockSFolder );
	pListNode = g_MalwFind.DrvSFolder.ListHead_ExceptProc.Flink;
	while(pListNode && pListNode != &g_MalwFind.DrvSFolder.ListHead_ExceptProc)
	{
		pProcObj = (PCONTROL_OBJ)pListNode;
		if(pProcObj)
		{
			bExist = Search_WildcardToken( pwzProcName, pProcObj->Obj.Buffer );
			if(bExist)
			{
				ExReleaseFastMutex( &g_MalwFind.DrvSFolder.LockSFolder );
				return TRUE;
			}
		}
		pListNode = pListNode->Flink;
	}
	ExReleaseFastMutex( &g_MalwFind.DrvSFolder.LockSFolder );
	return FALSE;
}




/************************************************************************************/
//ControlObject Handling
/***********************************************************************************/

// CONTROL_OBJECT  Init
void 
ControlHeaderInit( IN PCONTROL_OBJ pObject, IN PLIST_ENTRY pListHead, IN BYTE ucType )
{
	
	if(!pObject || !pListHead) 
	{
		ASSERT( pObject && pListHead );
		return;
	}

	pObject->Hdr.ucType  = ucType;
	pObject->Hdr.ucAcctl = MALWF_ACCESS;
	pObject->Hdr.usRef   = 0x00;  // Reserved

	// 자료구조 리스트로 연결
	InsertHeadList( pListHead, &pObject->Hdr.ListEntry ); 

}


// ControlObject Init
PCONTROL_OBJ  
ControlObjAdd( IN ULONG ulBufferLen )
{
	ULONG         ulLength   = 0;
	PCONTROL_OBJ  pNewObject = NULL;

	ulLength   = sizeof(CONTROL_OBJ) + sizeof(WCHAR)*(ulBufferLen+2);
	pNewObject = (PCONTROL_OBJ)ExAllocatePoolWithTag( PagedPool, ulLength, MALWFIND_NAME_TAG );
	if(!pNewObject) return NULL;

	RtlZeroMemory( pNewObject, ulLength );
	pNewObject->Obj.pObjPos     = NULL;
	pNewObject->Obj.usLength    = 0x00;
	pNewObject->Obj.usMaxLength = (USHORT)ulBufferLen+1;
	pNewObject->Obj.usFile      = MFOBJ_NONE;
	pNewObject->Hdr.usRef++;
	return pNewObject;
}


void ControlObjDelete( IN PCONTROL_OBJ pObject )
{
	ASSERT( pObject );
	if(!pObject) return;

	//KdPrint(("Hdr.usRef:[%d] \n", --pObject->Hdr.usRef ));
	ExFreePoolWithTag( pObject, MALWFIND_NAME_TAG );
}


PCONTROL_OBJ  
ControlObjFind( IN PLIST_ENTRY pListHead, IN PWCHAR pwzObjName )
{
	PCONTROL_OBJ  pCtrlObj  = NULL;
	PLIST_ENTRY   pListNode = NULL;

	ASSERT( pListHead && pwzObjName );
	if(!pListHead || !pwzObjName) return NULL;

	pListNode = pListHead->Flink;
	while(pListNode && pListNode != pListHead)
	{
		pCtrlObj = (PCONTROL_OBJ)pListNode;

		ASSERT( pCtrlObj );
		if(pCtrlObj && pCtrlObj->Obj.Buffer)
		{
			if(!_wcsnicmp( pCtrlObj->Obj.Buffer, pwzObjName, pCtrlObj->Obj.usLength ))
			{
				return pCtrlObj;
			}
		}
		pListNode = pListNode->Flink;		
	}

	return NULL;

}


PCONTROL_OBJ  
ControlObjFind_Str( IN PLIST_ENTRY pListHead, IN PWCHAR pwzObjName )
{
	PCONTROL_OBJ  pCtrlObj  = NULL;
	PLIST_ENTRY   pListNode = NULL;

	ASSERT( pListHead && pwzObjName );
	if(!pListHead || !pwzObjName) return NULL;

	pListNode = pListHead->Flink;
	while(pListNode && pListNode != pListHead)
	{
		pCtrlObj = (PCONTROL_OBJ)pListNode;

		ASSERT( pCtrlObj );
		if(pCtrlObj && pCtrlObj->Obj.Buffer)
		{
			if(wcsstr( pCtrlObj->Obj.Buffer, pwzObjName ))
			{
				return pCtrlObj;
			}
		}
		pListNode = pListNode->Flink;		
	}

	return NULL;

}



/******************************************************************************************************/
// FILE_EXT
/******************************************************************************************************/

PFILE_EXT  
ControlFileExtAdd( IN ULONG ulFileExtLen )
{
	ULONG      ulLength = 0;
	PFILE_EXT  PFILE_EXT = NULL;
	ulLength = sizeof(FILE_EXT) + sizeof(WCHAR)*(ulFileExtLen+1);

	PFILE_EXT = ExAllocatePoolWithTag( PagedPool, ulLength, MALWFIND_NAME_TAG );
	ASSERT( PFILE_EXT );
	if(!PFILE_EXT)  return NULL;

	RtlZeroMemory( PFILE_EXT, ulLength );
	PFILE_EXT->usFileExtMaxLength = (USHORT)(ulFileExtLen+1);
	PFILE_EXT->usFileExtLength    = (USHORT)ulFileExtLen;
	PFILE_EXT->ucAcctl            = 0x00;
	return PFILE_EXT;
}

void ControlFileExtDelete( IN PFILE_EXT  pFileExt )
{
	if(!pFileExt) return;
	if(pFileExt)
	{
		ExFreePoolWithTag( pFileExt, MALWFIND_NAME_TAG );
		pFileExt = NULL;
	}
}



PFILE_EXT  
ControlFileExtFind( IN PLIST_ENTRY  pListHead, IN PWCHAR pFileExtName )
{
	PFILE_EXT    pFileExt = NULL;
	PLIST_ENTRY  pList = NULL;

	ASSERT( pListHead && pFileExtName );
	if(!pListHead || !pFileExtName) return NULL;

	pList = pListHead->Flink;
	if(!pList) return NULL;

	while(pList && pList != pListHead)
	{
		pFileExt = (PFILE_EXT)pList;
		ASSERT( pFileExt );
		if(pFileExt && pFileExt->FileExt)
		{
			if(!_wcsnicmp( pFileExt->FileExt, pFileExtName, pFileExt->usFileExtLength ))
			{
				return pFileExt;
			}
		}
		pList = pList->Flink;
	}

	return NULL;

}


