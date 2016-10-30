#include "StdAfx.h"
#include "ConfigSvcDriver.h"
#include "..\Common\MalwFind_Comm.h"
#include "MalwFindErrcode.h"
#include "MalwFindExport.h"
#include <winioctl.h>


using namespace APP;
APP::CConfigSvcDriver g_SvcDriver;


CConfigSvcDriver::CConfigSvcDriver(void)
: m_hService(NULL), 
  m_hFsDrv(INVALID_HANDLE_VALUE),
  m_pszDriverDeviceName(NULL)
{	
	SetServiceType_KernelDriver();
	SetStartType_DemandStart();
}


CConfigSvcDriver::~CConfigSvcDriver(void)
{
	if(m_pszDriverDeviceName)
	{
		delete m_pszDriverDeviceName;
		m_pszDriverDeviceName = NULL;
	}

}


CString  
CConfigSvcDriver::GetDriverDeviceName(void)
{
	return (CString)m_pszDriverDeviceName;
}


void CConfigSvcDriver::SetDriverDeviceName(const TCHAR* pszDriverDeviceName, ULONG ulDriverDeviceNameLen )
{
	ASSERT( pszDriverDeviceName );
	if(!pszDriverDeviceName) return;

	if(!m_pszDriverDeviceName)
	{
		delete m_pszDriverDeviceName;
		m_pszDriverDeviceName = NULL;
	}

	m_pszDriverDeviceName = new TCHAR[ ulDriverDeviceNameLen+1 ];
	ASSERT( m_pszDriverDeviceName );
	if(m_pszDriverDeviceName)
	{
		RtlZeroMemory( m_pszDriverDeviceName, sizeof(TCHAR)*(ulDriverDeviceNameLen+1) );
		_tcsncpy_s( m_pszDriverDeviceName, ulDriverDeviceNameLen+1, pszDriverDeviceName, ulDriverDeviceNameLen );
	}

}


void CConfigSvcDriver::SetSvcStatus(DWORD dwState, DWORD dwAccept)
{
	UNREFERENCED_PARAMETER( dwState  );	
	UNREFERENCED_PARAMETER( dwAccept );
	return;
}


BOOL CConfigSvcDriver::Driver_Open(void)
{
	if(!m_hFsDrv || m_hFsDrv == INVALID_HANDLE_VALUE) return FALSE;

	m_hFsDrv = CreateFile( m_pszDriverDeviceName, 
						   GENERIC_READ | GENERIC_WRITE, 
						   0, 
						   NULL, 
						   OPEN_EXISTING, 
						   FILE_ATTRIBUTE_NORMAL, 
						   NULL  );

	if(m_hFsDrv == INVALID_HANDLE_VALUE) return FALSE;

	return TRUE;
}



void CConfigSvcDriver::Dirver_Close(void)
{

	CloseHandle( m_hFsDrv );
	m_hFsDrv = NULL;
	
}


ULONG CConfigSvcDriver::RunService(void)
{
	SC_HANDLE	hSCManager = NULL;
	SC_HANDLE	hSCOpenSvc = NULL;

	SERVICE_STATUS_PROCESS	SvcStatusProc;
	DWORD	dwByteNeeded = 0;

	try
	{
		hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if(!hSCManager) throw FLT_STATUS_INVALID_HANDLE;

		hSCOpenSvc = ::OpenService(hSCManager, m_pServiceName, SC_MANAGER_ALL_ACCESS);
		if(!hSCOpenSvc) throw FLT_STATUS_INVALID_HANDLE;

		RtlZeroMemory(&SvcStatusProc, sizeof(SERVICE_STATUS_PROCESS));
		if(!QueryServiceStatusEx(hSCOpenSvc, SC_STATUS_PROCESS_INFO, (LPBYTE)&SvcStatusProc, sizeof(SERVICE_STATUS_PROCESS), &dwByteNeeded))
		{
			throw FLT_STATUS_UNSUCCESSFUL;
		}

		if(SvcStatusProc.dwCurrentState != SERVICE_RUNNING)
		{
			if(!StartService(hSCOpenSvc, 0, NULL)) throw FLT_STATUS_UNSUCCESSFUL;
		}

		m_hFsDrv = CreateFile( m_pszDriverDeviceName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(m_hFsDrv == INVALID_HANDLE_VALUE) throw FLT_STATUS_INVALID_HANDLE;

	}
	catch(int nException)
	{
		int nError = nException;

		UNREFERENCED_PARAMETER( nError );

		if(hSCOpenSvc)	CloseServiceHandle(hSCOpenSvc);
		if(hSCManager)	CloseServiceHandle(hSCManager);
		if(m_hFsDrv)
		{
			CloseHandle(m_hFsDrv);
			m_hFsDrv = INVALID_HANDLE_VALUE;
		}

		return FLT_STATUS_UNSUCCESSFUL;
	}
	catch(...)
	{
		if(hSCOpenSvc)	CloseServiceHandle(hSCOpenSvc);
		if(hSCManager)	CloseServiceHandle(hSCManager);
		if(m_hFsDrv)
		{
			CloseHandle(m_hFsDrv);
			m_hFsDrv = INVALID_HANDLE_VALUE;
		}

		return FLT_STATUS_UNSUCCESSFUL;
	}


	if(hSCOpenSvc)	CloseServiceHandle(hSCOpenSvc);
	if(hSCManager)	CloseServiceHandle(hSCManager);

	return 0;
}




void CConfigSvcDriver::StopService(void)
{
	if(m_hFsDrv != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFsDrv);
		m_hFsDrv = INVALID_HANDLE_VALUE;
	}
}

 // MainServer Call Func
BOOL CConfigSvcDriver::ExecuteServiceFunc(void) 
{
	// Driver Service 필요로 하지 않다.
	return true;
}


/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/


//드라이버 버전얻기
FLT_STATUS __fastcall CConfigSvcDriver::Driver_GetVersion()
{
	ULONG          ulTotalSize = 0;
	FLT_STATUS     Status = 0;
	DRVCMD_Version DrvVer;

	ulTotalSize = sizeof(DRVCMD_Version);
	RtlZeroMemory( &DrvVer, sizeof(DrvVer) );
	DrvVer.Hdr.ulSignature = MF_SIGNATURE;
	DrvVer.Hdr.ulVersion   = MF_VERSION;
	DrvVer.Hdr.ulTotalSize = ulTotalSize;
	Status = FltPolicySend( IOCTL_MALWF_GET_VERSION, (void*)&DrvVer, ulTotalSize, (void*)&DrvVer, (PULONG_PTR)&ulTotalSize );
	return Status;
}


//로그 취합시작
FLT_STATUS __fastcall CConfigSvcDriver::Driver_LogStart( ULONG ulLogStart )
{
	ULONG            ulTotalSize = 0;
	FLT_STATUS       Status = 0;
	DRVCMD_LogStart  DrvLog;

	ulTotalSize = sizeof(DRVCMD_LogStart);
	RtlZeroMemory( &DrvLog, sizeof(DrvLog) );
	DrvLog.Hdr.ulSignature = MF_SIGNATURE;
	DrvLog.Hdr.ulVersion   = MF_VERSION;
	DrvLog.Hdr.ulTotalSize = ulTotalSize;
	DrvLog.ulLogStart = ulLogStart;
	Status = FltPolicySend( IOCTL_MALWF_LOG_START,(void*)&DrvLog, ulTotalSize, NULL, NULL );
	return Status;
}

// 로그 Fetch 
FLT_STATUS __fastcall CConfigSvcDriver::Driver_GetLogFetch( char* pBuffer, ULONG  ulOutBufferSize )
{
	ULONG            ulTotalSize=0;
	FLT_STATUS       Status=0;
	DRVCMD_LogFetch  LogFetch;

	if(!pBuffer) 
	{
		return FLT_STATUS_INVALID_PARAMETER;
	}

	ulTotalSize = sizeof(DRVCMD_LogFetch);
	RtlZeroMemory( &LogFetch, ulTotalSize );
	LogFetch.Hdr.ulSignature = MF_SIGNATURE;
	LogFetch.Hdr.ulVersion   = MF_VERSION;
	LogFetch.Hdr.ulTotalSize = ulTotalSize;
	Status = FltPolicySend( IOCTL_MALWF_GET_LOGFETCH, (void*)&LogFetch, ulTotalSize, pBuffer, (PULONG_PTR)&ulOutBufferSize );
	return Status;
}




// GlobalControl 제어플래그 세팅
FLT_STATUS __fastcall CConfigSvcDriver::Driver_SetGlobalCtrl( BOOL bGlobalCtrl )
{
	ULONG        ulTotalSize = 0;
	FLT_STATUS   Status = 0;
	DRVCMD_SetupDir  DrvSetupDir;

	ulTotalSize = sizeof(DrvSetupDir);
	RtlZeroMemory( &DrvSetupDir, sizeof(DrvSetupDir) );
	DrvSetupDir.Hdr.ulSignature = MF_SIGNATURE;
	DrvSetupDir.Hdr.ulVersion   = MF_VERSION;
	DrvSetupDir.Hdr.ulTotalSize = ulTotalSize;	
	DrvSetupDir.bGlobalCtrl = bGlobalCtrl;
	
	Status = FltPolicySend( IOCTL_MALWF_SET_SETUPDIRCTRL,(void*)&DrvSetupDir, ulTotalSize, NULL, NULL );
	return Status;
}


//설치폴더 정보설정
FLT_STATUS __fastcall CConfigSvcDriver::Driver_SetSetupDir( char* pSetupDir, char* pHideFolder, char* pSysDir )
{
	DRVCMD_SetupDir  DrvSetupDir;
	ULONG       ulTotalSize = 0;
	FLT_STATUS  Status      = 0;
	WCHAR wzSetupDir[MAX_PATH], wzHideFolder[MAX_DIR_NAME], wzSysDir[MAX_PATH];

	if(!pSetupDir || !pSysDir) 
	{
		return FLT_STATUS_INVALID_PARAMETER;
	}

	ulTotalSize = sizeof(DrvSetupDir);
	RtlZeroMemory( &DrvSetupDir, sizeof(DrvSetupDir) );
	DrvSetupDir.Hdr.ulSignature = MF_SIGNATURE;
	DrvSetupDir.Hdr.ulVersion   = MF_VERSION;
	DrvSetupDir.Hdr.ulTotalSize = ulTotalSize;

	RtlZeroMemory( wzHideFolder, MAX_DIR_NAME );
	RtlZeroMemory( wzSetupDir,   MAX_PATH );
	RtlZeroMemory( wzSysDir,     MAX_PATH );

	MultiByteToWideChar( CP_ACP, 0, pHideFolder, (int)strlen(pHideFolder)+1, wzHideFolder, MAX_DIR_NAME );
	MultiByteToWideChar( CP_ACP, 0, pSetupDir, (int)strlen(pSetupDir)+1, wzSetupDir, MAX_PATH );
	MultiByteToWideChar( CP_ACP, 0, pSysDir,   (int)strlen(pSysDir)+1,   wzSysDir,   MAX_PATH );

	StringCchCopyW( DrvSetupDir.wzHideFolder, MAX_DIR_NAME, wzHideFolder );
	StringCchCopyW( DrvSetupDir.wzSetupDir,   MAX_260_PATH, wzSetupDir   );
	StringCchCopyW( DrvSetupDir.wzSystemDir,  MAX_260_PATH, wzSysDir     );

	Status = FltPolicySend( IOCTL_MALWF_SET_SETUPDIR,(void*)&DrvSetupDir, ulTotalSize, NULL, NULL );
	return Status;

}



//File 예외폴더 설정
FLT_STATUS __fastcall CConfigSvcDriver::Driver_F_SetExceptDir( char* pczExceptDir )
{
	DRVCMD_ExceptFileDir  DrvDir;
	ULONG      ulTotalSize = 0;
	FLT_STATUS Status = 0;
	WCHAR      wzExceptDir[MAX_PATH];

	if(!pczExceptDir) 
	{
		return FLT_STATUS_INVALID_PARAMETER;
	}

	ulTotalSize = sizeof(DRVCMD_ExceptFileDir);
	RtlZeroMemory( &DrvDir, sizeof(DrvDir) );
	DrvDir.Hdr.ulSignature = MF_SIGNATURE;
	DrvDir.Hdr.ulVersion   = MF_VERSION;
	DrvDir.Hdr.ulTotalSize = ulTotalSize;

	RtlZeroMemory( wzExceptDir, MAX_PATH );
	MultiByteToWideChar( CP_ACP, 0, pczExceptDir, (int)strlen(pczExceptDir)+1, wzExceptDir, MAX_PATH );	

	StringCchCopyW( DrvDir.wzExceptDir, MAX_260_PATH, wzExceptDir );
	Status = FltPolicySend( IOCTL_MALWF_SET_EXCEPT_FILEDIR,(void*)&DrvDir, ulTotalSize, NULL, NULL );
	return Status;
}


//File 예외폴더 해제
FLT_STATUS __fastcall CConfigSvcDriver::Driver_F_ClrExceptDir()
{
	ULONG       ulTotalSize = 0;
	FLT_STATUS  Status      = 0;
	DRVCMD_ExceptFileDir DrvDir;

	ulTotalSize = sizeof(DRVCMD_ExceptFileDir);
	RtlZeroMemory( &DrvDir, sizeof(DrvDir) );
	DrvDir.Hdr.ulSignature = MF_SIGNATURE;
	DrvDir.Hdr.ulVersion   = MF_VERSION;
	DrvDir.Hdr.ulTotalSize = ulTotalSize;

	Status = FltPolicySend( IOCTL_MALWF_CLR_EXCEPT_FILEDIR,(void*)&DrvDir, ulTotalSize, NULL, NULL );
	return Status;
}


//File 예외프로세스폴더 설정
FLT_STATUS __fastcall CConfigSvcDriver::Driver_F_SetExceptProc( char* pczExceptProc )
{
	DRVCMD_ExceptFileProc DrvProc;
	ULONG       ulTotalSize = 0;
	FLT_STATUS  Status = 0;
	WCHAR  wzExceptProc[MAX_PATH];

	ASSERT( pczExceptProc );
	if(!pczExceptProc) return FLT_STATUS_INVALID_PARAMETER;

	ulTotalSize = sizeof(DRVCMD_ExceptFileProc);
	RtlZeroMemory( &DrvProc, sizeof(DrvProc) );
	DrvProc.Hdr.ulSignature = MF_SIGNATURE;
	DrvProc.Hdr.ulVersion   = MF_VERSION;
	DrvProc.Hdr.ulTotalSize = ulTotalSize;

	RtlZeroMemory( wzExceptProc, MAX_PATH );
	MultiByteToWideChar( CP_ACP, 0, pczExceptProc, (int)strlen(pczExceptProc)+1, wzExceptProc, MAX_PATH );	

	StringCchCopyW( DrvProc.wzExceptProc, MAX_260_PATH, wzExceptProc );
	Status = FltPolicySend( IOCTL_MALWF_SET_EXCEPT_FILEPROC,(void*)&DrvProc, ulTotalSize, NULL, NULL );
	return Status;
}


//File 예외프로세스폴더 해제
FLT_STATUS __fastcall CConfigSvcDriver::Driver_F_ClrExceptProc()
{
	DRVCMD_ExceptFileProc DrvProc;
	ULONG       ulTotalSize = 0;
	FLT_STATUS  Status = 0;
	
	ulTotalSize = sizeof(DRVCMD_ExceptFileProc);
	RtlZeroMemory( &DrvProc, sizeof(DrvProc) );
	DrvProc.Hdr.ulSignature = MF_SIGNATURE;
	DrvProc.Hdr.ulVersion   = MF_VERSION;
	DrvProc.Hdr.ulTotalSize = ulTotalSize;

	Status = FltPolicySend( IOCTL_MALWF_CLR_EXCEPT_FILEPROC,(void*)&DrvProc, ulTotalSize, NULL, NULL );
	return Status;
}


//File FDD 제어설정
FLT_STATUS __fastcall CConfigSvcDriver::Driver_F_SetFDDCtrl( NODE_POLICY* pPolicy )
{
	ULONG       ulTotalSize = 0;
	FLT_STATUS  Status = 0;
	DRVCMD_FddVolCtrl  DrvFddCtrl;

	ASSERT( pPolicy );
	if(!pPolicy) return FLT_STATUS_INVALID_PARAMETER;
		
	ulTotalSize = sizeof(DRVCMD_FddVolCtrl);
	RtlZeroMemory( &DrvFddCtrl, sizeof(DrvFddCtrl) );
	DrvFddCtrl.Hdr.ulSignature = MF_SIGNATURE;
	DrvFddCtrl.Hdr.ulVersion   = MF_VERSION;
	DrvFddCtrl.Hdr.ulTotalSize = ulTotalSize;

	DrvFddCtrl.FddPolicy = (*pPolicy);

	Status = FltPolicySend( IOCTL_MALWF_SET_FDDVOLCTRL,(void*)&DrvFddCtrl, ulTotalSize, NULL, NULL );
	return Status;
}


//File 이동저장장치 제어설정
FLT_STATUS __fastcall CConfigSvcDriver::Driver_F_SetUSBStorCtrl( NODE_POLICY* pPolicy )
{
	ULONG       ulTotalSize = 0;
	FLT_STATUS  Status = 0;
	DRVCMD_USBVolCtrl DrvUSBCtrl;

	ASSERT( pPolicy );
	if(!pPolicy) return FLT_STATUS_INVALID_PARAMETER;

	ulTotalSize = sizeof(DRVCMD_USBVolCtrl);
	RtlZeroMemory( &DrvUSBCtrl, sizeof(DrvUSBCtrl) );
	DrvUSBCtrl.Hdr.ulSignature = MF_SIGNATURE;
	DrvUSBCtrl.Hdr.ulVersion   = MF_VERSION;
	DrvUSBCtrl.Hdr.ulTotalSize = ulTotalSize;

	DrvUSBCtrl.UsbPolicy = (*pPolicy);

	Status = FltPolicySend( IOCTL_MALWF_SET_USBVOLCTRL,(void*)&DrvUSBCtrl, ulTotalSize, NULL, NULL );
	return Status;
}


//File 확장자 제어설정
FLT_STATUS __fastcall 
CConfigSvcDriver::Driver_F_SetFileExtCtrl( BOOL bFileExtDeny, NODE_POLICY* pPolicy, char* pczFileExt )
{
	DRVCMD_FileExtCtrl  FileExtCtrl;
	ULONG       ulTotalSize = 0;
	FLT_STATUS  Status = 0;
	WCHAR       wzFileExt[MAX_PATH];

	ASSERT( pPolicy && pczFileExt );
	if(!pPolicy || !pczFileExt) return FLT_STATUS_INVALID_PARAMETER;

	ulTotalSize = sizeof(FileExtCtrl);
	RtlZeroMemory( &FileExtCtrl, sizeof(FileExtCtrl) );
	FileExtCtrl.Hdr.ulSignature = MF_SIGNATURE;
	FileExtCtrl.Hdr.ulVersion   = MF_VERSION;
	FileExtCtrl.Hdr.ulTotalSize = ulTotalSize;

	//
	// TRUE: 확장자제어 차단리스트 FALSE:확장자제어 차단 예외리스트
	FileExtCtrl.bFileExtDeny = bFileExtDeny;
	//
	//

	RtlZeroMemory( wzFileExt, MAX_PATH );
	MultiByteToWideChar( CP_ACP, 0, pczFileExt, (int)strlen(pczFileExt)+1, wzFileExt, MAX_PATH );

	FileExtCtrl.FileExtPolicy = (*pPolicy);
	StringCchCopyW( FileExtCtrl.FileExt, MAX_260_PATH, wzFileExt );
	Status = FltPolicySend( IOCTL_MALWF_SET_FILEEXTCTRL,(void*)&FileExtCtrl, ulTotalSize, NULL, NULL );
	return Status;
}


//File 확장자 제어해제 
FLT_STATUS __fastcall CConfigSvcDriver::Driver_F_ClrFileExtCtrl()
{
	ULONG       ulTotalSize = 0;
	FLT_STATUS  Status = 0;
	DRVCMD_FileExtCtrl  FileExtCtrl;

	ulTotalSize = sizeof(FileExtCtrl);
	RtlZeroMemory( &FileExtCtrl, sizeof(FileExtCtrl) );
	FileExtCtrl.Hdr.ulSignature = MF_SIGNATURE;
	FileExtCtrl.Hdr.ulVersion   = MF_VERSION;
	FileExtCtrl.Hdr.ulTotalSize = ulTotalSize;

	FileExtCtrl.bFileExtDeny = TRUE;

	Status = FltPolicySend( IOCTL_MALWF_CLR_FILEEXTCTRL,(void*)&FileExtCtrl, ulTotalSize, NULL, NULL );
	return Status;
}


//공유폴더 제어설정
FLT_STATUS __fastcall CConfigSvcDriver::Driver_SF_SetSFolderCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog )
{
	DRVCMD_SFolderCtrl SFCtrl;
	ULONG        ulTotalSize = 0;
	FLT_STATUS   Status      = 0;
	
	ulTotalSize = sizeof(SFCtrl);
	RtlZeroMemory( &SFCtrl, sizeof(SFCtrl) );
	SFCtrl.Hdr.ulSignature = MF_SIGNATURE;
	SFCtrl.Hdr.ulVersion   = MF_VERSION;
	SFCtrl.Hdr.ulTotalSize = ulTotalSize;

	SFCtrl.Policy.Vol.ulDrives = ulDrives;
	SFCtrl.Policy.ulAcctl      = ulAcctl;
	SFCtrl.Policy.bLog         = bLog;
	
	Status = FltPolicySend( IOCTL_MALWF_SET_SFCTRL,(void*)&SFCtrl, ulTotalSize, NULL, NULL );
	return Status;
}


//공유폴더 제어해제
FLT_STATUS __fastcall CConfigSvcDriver::Driver_SF_ClrSFolderCtrl()
{
	ULONG       ulTotalSize = 0;
	FLT_STATUS  Status = 0;
	DRVCMD_SFolderCtrl SFCtrl;

	ulTotalSize = sizeof(SFCtrl);
	RtlZeroMemory( &SFCtrl, sizeof(SFCtrl) );
	SFCtrl.Hdr.ulSignature = MF_SIGNATURE;
	SFCtrl.Hdr.ulVersion   = MF_VERSION;
	SFCtrl.Hdr.ulTotalSize = ulTotalSize;

	Status = FltPolicySend( IOCTL_MALWF_CLR_SFCTRL,(void*)&SFCtrl, ulTotalSize, NULL, NULL );
	return Status;
}


//공유폴더 예외폴더 설정
FLT_STATUS __fastcall CConfigSvcDriver::Driver_SF_SetExceptDir( char* pczExceptDir )
{
	DRVCMD_ExceptFileDir SfDir;
	ULONG      ulTotalSize = 0;
	FLT_STATUS Status = 0;
	WCHAR  wzExceptDir[MAX_PATH];	

	ASSERT( pczExceptDir );
	if(!pczExceptDir) return FLT_STATUS_INVALID_PARAMETER;

	ulTotalSize = sizeof(SfDir);
	RtlZeroMemory( &SfDir, sizeof(SfDir) );
	SfDir.Hdr.ulSignature = MF_SIGNATURE;
	SfDir.Hdr.ulVersion   = MF_VERSION;
	SfDir.Hdr.ulTotalSize = ulTotalSize;

	RtlZeroMemory( wzExceptDir, MAX_PATH );
	MultiByteToWideChar( CP_ACP, 0, pczExceptDir, (int)strlen(pczExceptDir)+1, wzExceptDir, MAX_PATH );	

	StringCchCopyW( SfDir.wzExceptDir, MAX_260_PATH, wzExceptDir );
	Status = FltPolicySend( IOCTL_MALWF_SET_EXCEPT_SFDIR,(void*)&SfDir, ulTotalSize, NULL, NULL );
	return Status;
}


//공유폴더 예외폴더 해제
FLT_STATUS __fastcall CConfigSvcDriver::Driver_SF_ClrExceptDir()
{
	ULONG       ulTotalSize=0;
	FLT_STATUS  Status = 0;
	DRVCMD_ExceptFileDir SfDir;

	ulTotalSize = sizeof(SfDir);
	RtlZeroMemory( &SfDir, sizeof(SfDir) );
	SfDir.Hdr.ulSignature = MF_SIGNATURE;
	SfDir.Hdr.ulVersion   = MF_VERSION;
	SfDir.Hdr.ulTotalSize = ulTotalSize;

	Status = FltPolicySend( IOCTL_MALWF_CLR_EXCEPT_SFDIR,(void*)&SfDir, ulTotalSize, NULL, NULL );
	return Status;
}


//공유폴더 예외프로세스 설정
FLT_STATUS __fastcall CConfigSvcDriver::Driver_SF_SetExceptProc( char* pczExceptProc )
{
	ULONG       ulTotalSize = 0;
	FLT_STATUS  Status = 0;
	DRVCMD_ExceptFileProc SFProc;
	WCHAR   wzExceptProc[MAX_PATH];

	if(!pczExceptProc) 
	{
		ASSERT( pczExceptProc );
		return FLT_STATUS_INVALID_PARAMETER;
	}

	ulTotalSize = sizeof(SFProc);
	RtlZeroMemory( &SFProc, sizeof(SFProc) );
	SFProc.Hdr.ulSignature = MF_SIGNATURE;
	SFProc.Hdr.ulVersion   = MF_VERSION;
	SFProc.Hdr.ulTotalSize = ulTotalSize;

	RtlZeroMemory( wzExceptProc, MAX_PATH );
	MultiByteToWideChar( CP_ACP, 0, pczExceptProc, (int)strlen(pczExceptProc)+1, wzExceptProc, MAX_PATH );	

	StringCchCopyW( SFProc.wzExceptProc, MAX_260_PATH, wzExceptProc );
	Status = FltPolicySend( IOCTL_MALWF_SET_EXCEPT_SFPROC,(void*)&SFProc, ulTotalSize, NULL, NULL );
	return Status;
}


//공유폴더 예외프로세스 해제
FLT_STATUS __fastcall CConfigSvcDriver::Driver_SF_ClrExceptProc()
{
	ULONG       ulTotalSize = 0;
	FLT_STATUS  Status = 0;
	DRVCMD_ExceptFileProc SFProc;

	ulTotalSize = sizeof(SFProc);
	RtlZeroMemory( &SFProc, sizeof(SFProc) );
	SFProc.Hdr.ulSignature = MF_SIGNATURE;
	SFProc.Hdr.ulVersion   = MF_VERSION;
	SFProc.Hdr.ulTotalSize = ulTotalSize;

	Status = FltPolicySend( IOCTL_MALWF_CLR_EXCEPT_SFPROC,(void*)&SFProc, ulTotalSize, NULL, NULL );
	return Status;
}




//프로세스 콜백
//프로세스 살고 죽는것 가지고 오기
FLT_STATUS __fastcall CConfigSvcDriver::Driver_P_GetProcessStat( char* pczOutBuffer, int nMaxOutBuffer )
{
	ULONG       ulTotalSize=0;
	FLT_STATUS  Status = 0;

	ASSERT( pczOutBuffer );
	if(!pczOutBuffer) return FLT_STATUS_INVALID_PARAMETER;

	Status = FltPolicySend( IOCTL_MALWF_GET_PROC_STAT, pczOutBuffer, nMaxOutBuffer, pczOutBuffer, (PULONG_PTR)&nMaxOutBuffer );
	return Status;
}



//프로세스 예외폴더 설정 
FLT_STATUS __fastcall CConfigSvcDriver::Driver_P_SetExceptDir( char* pczExceptDir )
{
	ULONG               ulTotalSize=0;
	FLT_STATUS          Status = 0;
	DRVCMD_ExceptProcDir  ProcDir;
	WCHAR               wzExceptDir[MAX_PATH];

	if(!pczExceptDir) 
	{
		ASSERT( pczExceptDir );
		return FLT_STATUS_INVALID_PARAMETER;
	}

	ulTotalSize = sizeof(ProcDir);
	RtlZeroMemory( &ProcDir, sizeof(ProcDir) );
	ProcDir.Hdr.ulSignature = MF_SIGNATURE;
	ProcDir.Hdr.ulVersion   = MF_VERSION;
	ProcDir.Hdr.ulTotalSize = ulTotalSize;


	RtlZeroMemory( wzExceptDir, MAX_PATH );
	MultiByteToWideChar( CP_ACP, 0, pczExceptDir, (int)strlen(pczExceptDir)+1, wzExceptDir, MAX_PATH );	

	StringCchCopyW( ProcDir.wzExceptDir, MAX_260_PATH, wzExceptDir );
	Status = FltPolicySend( IOCTL_MALWF_SET_EXCEPT_PROCDIR,(void*)&ProcDir, ulTotalSize, NULL, NULL );
	return Status;
}


//프로세스 예외폴더 해제
FLT_STATUS __fastcall CConfigSvcDriver::Driver_P_ClrExceptDir()
{
	ULONG       ulTotalSize=0;
	FLT_STATUS  Status = 0;
	DRVCMD_ExceptProcDir ProcDir;

	ulTotalSize = sizeof(ProcDir);
	RtlZeroMemory( &ProcDir, sizeof(ProcDir) );
	ProcDir.Hdr.ulSignature = MF_SIGNATURE;
	ProcDir.Hdr.ulVersion   = MF_VERSION;
	ProcDir.Hdr.ulTotalSize = ulTotalSize;

	Status = FltPolicySend( IOCTL_MALWF_CLR_EXCEPT_PROCDIR,(void*)&ProcDir, ulTotalSize, NULL, NULL );
	return Status;
}


//프로세스 제어설정 
FLT_STATUS __fastcall CConfigSvcDriver::Driver_P_SetProcCtrl( BOOL bProcDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pzProcCtrl )
{
	ULONG        ulTotalSize = 0;
	FLT_STATUS   Status = 0;
	WCHAR        wzProcCtrl[MAX_PATH];
	DRVCMD_ProcCtrl ProcCtrl;
	

	ASSERT( pzProcCtrl );
	if(!pzProcCtrl) return FLT_STATUS_INVALID_PARAMETER;
	
	ulTotalSize = sizeof(ProcCtrl);
	RtlZeroMemory( &ProcCtrl, sizeof(ProcCtrl) );
	ProcCtrl.Hdr.ulSignature = MF_SIGNATURE;
	ProcCtrl.Hdr.ulVersion   = MF_VERSION;
	ProcCtrl.Hdr.ulTotalSize = ulTotalSize;

	RtlZeroMemory( wzProcCtrl, MAX_PATH );
	MultiByteToWideChar( CP_ACP, 0, pzProcCtrl, (int)strlen(pzProcCtrl)+1, wzProcCtrl, MAX_PATH );	

	ProcCtrl.Policy.Vol.ulDrives = ulDrives;
	ProcCtrl.Policy.ulAcctl  = ulAcctl;
	ProcCtrl.Policy.bLog     = bLog;

	//
	// TRUE: 확장자제어 차단리스트 FALSE:확장자제어 차단 예외리스트
	ProcCtrl.bProcDeny = bProcDeny;
	//
	//

	StringCchCopyW( ProcCtrl.Proc, MAX_260_PATH, wzProcCtrl );

	Status = FltPolicySend( IOCTL_MALWF_SET_PROC_CTRL,(void*)&ProcCtrl, ulTotalSize, NULL, NULL );
	return Status;
}


//프로세스 제어해제
FLT_STATUS __fastcall CConfigSvcDriver::Driver_P_ClrProcCtrl()
{
	ULONG       ulTotalSize=0;
	FLT_STATUS  Status = 0;
	DRVCMD_ProcCtrl ProcCtrl;

	ulTotalSize = sizeof(ProcCtrl);
	RtlZeroMemory( &ProcCtrl, sizeof(ProcCtrl) );
	ProcCtrl.Hdr.ulSignature = MF_SIGNATURE;
	ProcCtrl.Hdr.ulVersion   = MF_VERSION;
	ProcCtrl.Hdr.ulTotalSize = ulTotalSize;

	ProcCtrl.bProcDeny = TRUE;

	Status = FltPolicySend( IOCTL_MALWF_CLR_PROC_CTRL,(void*)&ProcCtrl, ulTotalSize, NULL, NULL );
	return Status;
}



FLT_STATUS __fastcall 
CConfigSvcDriver::FltPolicySend( ULONG  ulControlCode, 
							     void*  pSendBuf, 
								 ULONG  ulSendLength, 
								 void*  pReceiveBuf, 
								 PULONG_PTR  pReceiveLength )
{
	DWORD dwRet = 0;
	BOOL  bSuc  = FALSE;

	if(m_hFsDrv == INVALID_HANDLE_VALUE) 
	{ 
		return FLT_STATUS_INVALID_HANDLE;
	}

	if(pReceiveBuf && pReceiveLength) 
	{
		bSuc = DeviceIoControl(  m_hFsDrv, ulControlCode, 
								 pSendBuf, ulSendLength, 
								 pReceiveBuf, (DWORD)*pReceiveLength, 
								 &dwRet, NULL );
	}
	else 
	{
		bSuc = DeviceIoControl( m_hFsDrv, ulControlCode, pSendBuf, ulSendLength, NULL, NULL, &dwRet, NULL);
	}

	return FLT_STATUS_SUCCESS;

}