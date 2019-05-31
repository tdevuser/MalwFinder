
#include "stdafx.h"
#include "MalwFindExport.h"
#include "ConfigSvcDriver.h"
#include "MalwFind_Sha1.h"


// Startup
MALWFIND_API BOOL __stdcall MalwFind_DriverStartup(void)
{
	return true;
}
// Cleanup
MALWFIND_API BOOL __stdcall MalwFind_DriverCleanup(void)
{
	return true;
}


//드라이버 설치
MALWFIND_API BOOL __stdcall 
MalwFind_DriverInstall(char* pDrvSymName, char* pDrvSvcName, char* pDrvFullPath, ULONG ulStartType, ULONG ulServiceType )
{
	TCHAR szDrvSymName [MAX_PATH];
	TCHAR szDrvSvcName [MAX_PATH];
	TCHAR szDrvFullPath[MAX_PATH];

	ASSERT( pDrvSvcName && pDrvFullPath );
	if(!pDrvSvcName || !pDrvFullPath) return FALSE;

	RtlZeroMemory( szDrvSymName,  MAX_PATH );
	RtlZeroMemory( szDrvSvcName,  MAX_PATH );
	RtlZeroMemory( szDrvFullPath, MAX_PATH );
	
#ifdef _UNICODE
	MultiByteToWideChar( CP_ACP, 0, pDrvSymName, (int)strlen(pDrvSymName)+1,  szDrvSymName,  MAX_PATH );
	MultiByteToWideChar( CP_ACP, 0, pDrvSvcName, (int)strlen(pDrvSvcName)+1,  szDrvSvcName,  MAX_PATH );
	MultiByteToWideChar( CP_ACP, 0, pDrvFullPath,(int)strlen(pDrvFullPath)+1, szDrvFullPath, MAX_PATH );
#else
	StringCchCopyA( szDrvSymName,  MAX_PATH, pDrvSymName  );
	StringCchCopyA( szDrvSvcName,  MAX_PATH, pDrvSvcName  );
	StringCchCopyA( szDrvFullPath, MAX_PATH, pDrvFullPath );
#endif
	
	// 서비스 이름
	g_SvcDriver.SetServiceName     ( szDrvSvcName,  (ULONG)_tcslen(szDrvSvcName)  );
	g_SvcDriver.SetDriverDeviceName( szDrvSvcName,  (ULONG)_tcslen(szDrvSvcName)  );
	g_SvcDriver.SetServiceImagePath( szDrvFullPath, (ULONG)_tcslen(szDrvFullPath) );

	g_SvcDriver.SetStartType  ( ulStartType   );
	g_SvcDriver.SetServiceType( ulServiceType );
	return g_SvcDriver.InstallService();
}

//드라이버 삭제
MALWFIND_API BOOL __stdcall MalwFind_DriverUnInstall( char* pDrvSvcName )
{
	TCHAR  szDrvSvcName [MAX_PATH];

	ASSERT( pDrvSvcName );
	if(!pDrvSvcName) return FALSE;

	RtlZeroMemory( szDrvSvcName,  MAX_PATH );
#ifdef _UNICODE
	MultiByteToWideChar( CP_ACP, 0, pDrvSvcName, (int)strlen(pDrvSvcName)+1,  szDrvSvcName,  MAX_PATH );
#else
	StringCchCopyA( szDrvSvcName,  MAX_PATH, pDrvSvcName  );
#endif

	g_SvcDriver.SetServiceName( szDrvSvcName, (ULONG)_tcslen(szDrvSvcName)  );
	return g_SvcDriver.DeleteService();	
}


//드라이버 구동
MALWFIND_API BOOL __stdcall MalwFind_DriverStart( char* pDrvDeivceName, char* pDrvSvcName )
{	
	FLT_STATUS Status = FLT_STATUS_SUCCESS;
	TCHAR szDrvSvcName [MAX_PATH], szDrvDeviceName[MAX_PATH];

	ASSERT( pDrvSvcName && pDrvDeivceName );
	if(!pDrvSvcName || !pDrvDeivceName) return FALSE;

	RtlZeroMemory( szDrvDeviceName, MAX_PATH );
	RtlZeroMemory( szDrvSvcName,  MAX_PATH );

#ifdef _UNICODE
	MultiByteToWideChar( CP_ACP, 0, pDrvDeivceName, (int)strlen(pDrvDeivceName)+1,  szDrvDeviceName,  MAX_PATH );
	MultiByteToWideChar( CP_ACP, 0, pDrvSvcName, (int)strlen(pDrvSvcName)+1,  szDrvSvcName,  MAX_PATH );
#else
	StringCchCopyA( szDrvDeviceName, MAX_PATH, pDrvDeivceName );
	StringCchCopyA( szDrvSvcName, MAX_PATH, pDrvSvcName );
#endif 

	g_SvcDriver.SetDriverDeviceName( szDrvDeviceName, (ULONG)_tcslen(szDrvDeviceName) );
	g_SvcDriver.SetServiceName( szDrvSvcName, (ULONG)_tcslen(szDrvSvcName)  );

	Status = g_SvcDriver.RunService();
	if(FLT_SUCCESS( Status )) 
	{
		return TRUE;
	}
	return FALSE;
}

//드라이버 내림
MALWFIND_API BOOL __stdcall MalwFind_DriverStop( char* pDrvSvcName )
{	
	ASSERT( pDrvSvcName );
	if(!pDrvSvcName) return FALSE;

	return FALSE;
}


//드라이버 버전
MALWFIND_API BOOL __stdcall MalwFind_DriverGetVersion(void)
{
	FLT_STATUS Status = FLT_STATUS_SUCCESS;
	Status = g_SvcDriver.Driver_GetVersion();
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}



// 로그 취합시작/정지
MALWFIND_API BOOL __stdcall MalwFind_DriverLogStart( ULONG ulLogStart )
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_LogStart( ulLogStart );
	if(FLT_SUCCESS( Status )) 
	{
		return TRUE;
	}
	return FALSE;

}


// 로그 Fetch
MALWFIND_API BOOL __stdcall MalwFind_DriverGetLogFetch( char* pOutBuffer, ULONG  ulOutBufferSize )
{
	FLT_STATUS Status = 0;
	
	Status = g_SvcDriver.Driver_GetLogFetch( pOutBuffer, ulOutBufferSize );

	if(FLT_SUCCESS( Status )) 
	{
		return TRUE;
	}
	return FALSE;

}


//전역플래그 설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetGlobalCtrl(BOOL bGlobalCtrl)
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_SetGlobalCtrl(bGlobalCtrl);
	if(FLT_SUCCESS( Status )) return TRUE;
	
	return FALSE;
}

//설치폴더설정하기
MALWFIND_API BOOL __stdcall MalwFind_DriverSetupDir(char* pSetupDir, char* pSysDir )
{
	FLT_STATUS Status = 0;
	char*  pPosition  = NULL;
	char   czHideFolder[ MAX_PATH ];

	RtlZeroMemory( czHideFolder, MAX_PATH );
	pPosition = strrchr( pSetupDir, '\\' );
	if(pPosition)
	{
		pPosition++;
		StringCchCopyA( czHideFolder, MAX_PATH, pPosition );
	}

	Status = g_SvcDriver.Driver_SetSetupDir( pSetupDir, czHideFolder, pSysDir );
	if(FLT_SUCCESS( Status )) return TRUE;
	
	return FALSE;
}


//File 예외폴더 설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_FileDir(char* pczExceptDir)
{
	FLT_STATUS Status = 0;
	
	Status = g_SvcDriver.Driver_F_SetExceptDir( pczExceptDir );
	if(FLT_SUCCESS( Status )) return TRUE;
	
	return FALSE;
}

//File 예외폴더 해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_FileDir(void)
{
	FLT_STATUS Status = 0;
	Status = g_SvcDriver.Driver_F_ClrExceptDir();
	if(FLT_SUCCESS( Status )) return TRUE;
	return FALSE;
}

//File 예외프로세스 설정 
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_FileProc(char* pczExceptProc)
{
	FLT_STATUS Status = 0;
	Status = g_SvcDriver.Driver_F_SetExceptProc( pczExceptProc );
	if(FLT_SUCCESS( Status )) return TRUE;
	return FALSE;
}

//File 예외프로세스 해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_FileProc(void)
{
	FLT_STATUS Status = 0;
	Status = g_SvcDriver.Driver_F_ClrExceptProc();
	if(FLT_SUCCESS( Status )) return TRUE;
	return FALSE;
}


//File FDD 제어설정 
MALWFIND_API BOOL __stdcall MalwFind_DriverSetFDDVolCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog )
{
	FLT_STATUS Status = 0;
	NODE_POLICY  Policy;

	RtlZeroMemory( &Policy, sizeof(Policy) );
	Policy.bLog         = bLog;
	Policy.ulAcctl      = ulAcctl;
	Policy.Vol.ulDrives = ulDrives;
	
	Status = g_SvcDriver.Driver_F_SetFDDCtrl( &Policy );
	if(FLT_SUCCESS( Status )) return TRUE;
	return FALSE;
}


//File 이동저장장치 제어설정
MALWFIND_API BOOL __stdcall 
MalwFind_DriverSetUSBVolCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog )
{
	FLT_STATUS   Status = 0;
	NODE_POLICY  Policy;

	RtlZeroMemory( &Policy, sizeof(Policy) );
	Policy.bLog         = bLog;
	Policy.ulAcctl      = ulAcctl;
	Policy.Vol.ulDrives = ulDrives;

	Status = g_SvcDriver.Driver_F_SetUSBStorCtrl( &Policy );
	if(FLT_SUCCESS( Status )) return TRUE;
	return FALSE;

}


//File 확장자 제어설정
// TRUE: 확장자제어 차단리스트 FALSE:확장자제어 차단 예외리스트
MALWFIND_API BOOL __stdcall 
MalwFind_DriverSetFileExtCtrl( BOOL bFileExtDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczFileExt )
{
	FLT_STATUS   Status = 0;
	NODE_POLICY  Policy;

	RtlZeroMemory( &Policy, sizeof(Policy) );
	Policy.bLog         = bLog;
	Policy.ulAcctl      = ulAcctl;
	Policy.Vol.ulDrives = ulDrives;

	Status = g_SvcDriver.Driver_F_SetFileExtCtrl( bFileExtDeny, &Policy, pczFileExt );
	if(FLT_SUCCESS( Status )) 
	{
		return TRUE;
	}
	return FALSE;
}


//File 확장자 제어해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrFileExtCtrl(void)
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_F_ClrFileExtCtrl();
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}


//공유폴더 제어설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetSFolderCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog )
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_SF_SetSFolderCtrl( ulDrives, ulAcctl, bLog );
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}


//공유폴더 제어해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrSFolderCtrl(void)
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_SF_ClrSFolderCtrl();
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}


//공유폴더 예외폴더 설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_SFDir(char* pczExceptDir)
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_SF_SetExceptDir( pczExceptDir );
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}

//공유폴더 예외폴더 해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_SFDir(void)
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_SF_ClrExceptDir();
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}


//공유폴더 예외프로세스 설정 
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_SFProc( char* pczExceptProc )
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_SF_SetExceptProc( pczExceptProc );
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}

//공유폴더 예외프로세스 해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_SFProc(void)
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_SF_ClrExceptProc();
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}



//프로세스 살고 죽는것 가지고 오기
MALWFIND_API BOOL __stdcall MalwFind_DriverGetProcStat(char* pczOutBuffer, int nMaxOutBuffer)
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_P_GetProcessStat( pczOutBuffer, nMaxOutBuffer );
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}


//Process 예외폴더 설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_ProcDir( char* pczExceptDir )
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_P_SetExceptDir( pczExceptDir );
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}


//Process 예외폴더 해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_ProcDir(void)
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_P_ClrExceptDir();
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}


//Process 제어 설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetProcCtrl( BOOL bProcDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczProcCtrl )
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_P_SetProcCtrl( bProcDeny, ulDrives, ulAcctl, bLog, pczProcCtrl );
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}


//Process 제어 해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrProcCtrl(void)
{
	FLT_STATUS Status = 0;

	Status = g_SvcDriver.Driver_P_ClrProcCtrl(); 
	if(FLT_SUCCESS( Status )) return TRUE;

	return FALSE;
}




MALWFIND_API 
BOOL __stdcall MalwFind_GetHashValue( const unsigned char* pbEncKey, unsigned char* pbOutHashValue, int nMaxOutHashValue )
{
	bool           bSuc = false;
	char           czHashValue[MAX_PATH];
	TCHAR          szOutHash[ MAX_PATH ];
	CStringA       strHashValueA;
	APP::CFltSha1  Hash;
	
	ASSERT( pbEncKey && pbOutHashValue && nMaxOutHashValue );
	if(!pbEncKey || !pbOutHashValue || !nMaxOutHashValue) return FALSE;
	
	RtlZeroMemory( czHashValue, MAX_PATH );
	RtlZeroMemory( szOutHash,   MAX_PATH  );

	Hash.Reset();
	Hash.Update( (const unsigned char*)pbEncKey, (unsigned int)strlen((const char*)pbEncKey) );
	Hash.Final();

	bSuc = Hash.ReportHash( szOutHash, APP::CFltSha1::REPORT_HEX_SHORT );
	if(!bSuc) 
	{
		ASSERT( FALSE );
		return FALSE;
	}

	strHashValueA = szOutHash;
	StringCchCopyA( czHashValue, MAX_PATH, strHashValueA.GetBuffer() );
	strHashValueA.ReleaseBuffer();

	RtlCopyMemory( (char*)pbOutHashValue, czHashValue, nMaxOutHashValue );
	return TRUE;

}






