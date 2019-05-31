#include "StdAfx.h"
#include "MalwFManager.h"
#include "MalwFinder.h"
#include "../dll.MalwFindLib/MalwFindExport.h"

#pragma warning( disable:4995 )
#pragma warning( disable:4996 )

#define MAX_VOLNAME  8
#define DRV_SERVICE_NAME        "MalwFind"
#define DRV_MALWFIND_SYS         "MalwFind.sys"
#define DRV_DEVICE_MALWFIND_NAME "\\\\.\\MalwFind"



CMalwFManager g_MalwFMgr;


CMalwFManager::CMalwFManager(void) 
: m_ulDrvStartType(SERVICE_DEMAND_START),  
  m_ulDrvServiceType(SERVICE_KERNEL_DRIVER),
  m_hLogFetchEvent( NULL )
{
	RtlZeroMemory( m_czDrvSymName, MAX_PATH );
	RtlZeroMemory( m_czDrvSvcName, MAX_PATH );
	RtlZeroMemory( m_czDrvFullPath, MAX_PATH );

	StringCchCopyA( m_czDrvSymName, MAX_PATH, DRV_DEVICE_MALWFIND_NAME );
	StringCchCopyA( m_czDrvSvcName, MAX_PATH, DRV_SERVICE_NAME );

	GetHomeDirectory( m_czDrvFullPath, MAX_PATH );
	StringCchCatA( m_czDrvFullPath, MAX_PATH, "\\" );
	StringCchCatA( m_czDrvFullPath, MAX_PATH, DRV_MALWFIND_SYS );
	m_hLogFetchEvent = CreateEvent( NULL, FALSE, FALSE, NULL );


}


CMalwFManager::~CMalwFManager(void)
{

	if(m_hLogFetchEvent)
	{
		CloseHandle( m_hLogFetchEvent );
		m_hLogFetchEvent = NULL;
	}

}


int CMalwFManager::GetShiftIndex( char cVol )
{
	if(cVol < 'A' || cVol > 'Z') return 0xFF;

	return (ULONG)(cVol - 'A');
}

ULONG CMalwFManager::GetFdd_Drives( char* pOutBuffer, int nMaxOutBuffer )
{
	ULONG  ulDrives = 0;

	ASSERT( pOutBuffer && nMaxOutBuffer );
	if(!pOutBuffer || !nMaxOutBuffer) return 0;

	StringCchCopyA( pOutBuffer, nMaxOutBuffer, "A:\\" );
	StringCchCatA ( pOutBuffer, nMaxOutBuffer, "|" );
	StringCchCatA ( pOutBuffer, nMaxOutBuffer, "B:\\" );

	ulDrives |= (1 << GetShiftIndex( 'A' ));
	ulDrives |= (1 << GetShiftIndex( 'B' ));

	return ulDrives;
}



ULONG CMalwFManager::GetVolumeTypeDrives( ULONG ulReqDrvType, char* pOutBuffer, int nMaxOutBuffer )
{
	int    i=0, nLength=0;
	char   czVol[ MAX_VOLNAME ];
	ULONG  ulDrv=0, ulDrives=0, ulDrvType=0, ulReturn=0;

	ASSERT( pOutBuffer && nMaxOutBuffer );
	if(!pOutBuffer || !nMaxOutBuffer) return 0;

	ulDrives = ::GetLogicalDrives();
	for(i=0; i<26; i++)
	{
		ulDrv = ulDrives & (1 << i);
		if(!ulDrv) continue;
		
		RtlZeroMemory( czVol, MAX_VOLNAME );
		czVol[0] = (char)('A' + i);
		czVol[1] = ':';
		czVol[2] = '\\';
		czVol[3] = '\0';

		ulDrvType = ::GetDriveTypeA( czVol );
		if(ulReqDrvType != ulDrvType) continue;

		ulReturn |= ulDrv;
		nLength = (int)(strlen(pOutBuffer));

		if(nLength > 0) StringCchCatA( pOutBuffer, nMaxOutBuffer, "|" );
		StringCchCatA( pOutBuffer, nMaxOutBuffer, czVol );
	}

	return ulReturn;
}



ULONG CMalwFManager::GetFixed_Drives(char* pOutBuffer, int nMaxOutBuffer)
{
	ULONG ulDrv = 0;

	ASSERT( pOutBuffer && nMaxOutBuffer );
	if(!pOutBuffer || !nMaxOutBuffer) return 0;


	ulDrv = GetVolumeTypeDrives( DRIVE_FIXED, pOutBuffer, nMaxOutBuffer );

	return ulDrv;
}


ULONG CMalwFManager::GetUsb_Drives(char* pOutBuffer, int nMaxOutBuffer)
{
	ULONG ulDrv = 0;

	ASSERT( pOutBuffer && nMaxOutBuffer );
	if(!pOutBuffer || !nMaxOutBuffer) return 0;

	ulDrv = GetVolumeTypeDrives( DRIVE_REMOVABLE, pOutBuffer, nMaxOutBuffer );

	return ulDrv;
}



ULONG CMalwFManager::GetCdrom_Drives(char* pOutBuffer, int nMaxOutBuffer)
{
	ULONG ulDrv = 0;

	ASSERT( pOutBuffer && nMaxOutBuffer );
	if(!pOutBuffer || !nMaxOutBuffer) return 0;

	ulDrv = GetVolumeTypeDrives( DRIVE_CDROM, pOutBuffer, nMaxOutBuffer );

	return ulDrv;
}


ULONG CMalwFManager::GetNetwork_Drives( char* pOutBuffer, int nMaxOutBuffer )
{
	ULONG ulDrv = 0;

	ASSERT( pOutBuffer && nMaxOutBuffer );
	if(!pOutBuffer || !nMaxOutBuffer) return 0;

	ulDrv = GetVolumeTypeDrives( DRIVE_REMOTE, pOutBuffer, nMaxOutBuffer );
	return ulDrv;

}
	






void CMalwFManager::GetHomeDirectory( char* pczOutBuffer, int nMaxPath )
{
	int   nLength = 0;
	char  czFileName[ _MAX_PATH  ] = "";
	char  czDrive   [ _MAX_DRIVE ] = "";
	char  czDir     [ _MAX_DIR   ] = "";
	char  czFName   [ _MAX_FNAME ] = "";
	char  czExt     [ _MAX_EXT   ] = "";

	ASSERT( pczOutBuffer );
	if(!pczOutBuffer) return;

	RtlZeroMemory( czFileName, MAX_PATH );
	::GetModuleFileNameA( NULL, czFileName, _MAX_PATH );
	_splitpath_s( czFileName, czDrive, _MAX_DRIVE, czDir, _MAX_DIR, czFName, _MAX_FNAME, czExt, _MAX_EXT );

	StringCchCopyA( pczOutBuffer, nMaxPath, czDrive );
	StringCchCatA(  pczOutBuffer, nMaxPath, czDir   );

	nLength = (int)(strlen(pczOutBuffer)-1);
	pczOutBuffer[ nLength ] = '\0';

}

void CMalwFManager::SetInitParameter( char*  pDrvSymName,
									char*  pDrvSvcName,
									char*  pDrvFullPath,
									ULONG  ulStartType,
									ULONG  ulServiceType )
{
	ASSERT( pDrvSymName && pDrvSvcName && pDrvFullPath );
	if(!pDrvSymName || !pDrvSvcName || !pDrvFullPath) return;

	m_ulDrvStartType   = ulStartType;
	m_ulDrvServiceType = ulServiceType;

	StringCchCopyA( m_czDrvSymName, MAX_PATH, pDrvSymName  );
	StringCchCopyA( m_czDrvSvcName, MAX_PATH, pDrvSvcName  );
	StringCchCopyA( m_czDrvFullPath,MAX_PATH, pDrvFullPath );
}


// ����̹� ��ġ	
BOOL CMalwFManager::Driver_Install(void)
{
	return MalwFind_DriverInstall( m_czDrvSymName, m_czDrvSvcName, m_czDrvFullPath, m_ulDrvStartType, m_ulDrvServiceType );
}

// ����̹� ����
BOOL CMalwFManager::Driver_UnInstall(void)
{
	return MalwFind_DriverUnInstall( m_czDrvSvcName );
}
//����̹� ����
BOOL CMalwFManager::Driver_Start(void)
{
	return MalwFind_DriverStart( m_czDrvSymName, m_czDrvSvcName );
}
//����̹� ����
BOOL CMalwFManager::Driver_Stop(void)
{
	return MalwFind_DriverStop( m_czDrvSvcName );
}






//����̹� ����
BOOL CMalwFManager::Driver_GetVersion(void)
{
	return MalwFind_DriverGetVersion();
}

//�α׽����� ����
BOOL CMalwFManager::Driver_LogStart(ULONG ulLogStart)
{
	return MalwFind_DriverLogStart( ulLogStart );
}


BOOL CMalwFManager::Driver_GetLogFetch( char* pOutBuffer, ULONG ulOutBufferSize )
{
	return MalwFind_DriverGetLogFetch( pOutBuffer, ulOutBufferSize );
}



//�����÷��� ����
BOOL CMalwFManager::Driver_SetGlobalCtrl(BOOL bGlobalCtrl)
{
	return MalwFind_DriverSetGlobalCtrl(bGlobalCtrl);
}
//��ġ���������ϱ�
BOOL CMalwFManager::Driver_SetupDir(char* pSetupDir, char* pSysDir)
{
	return MalwFind_DriverSetupDir( pSetupDir, pSysDir );
}
//File �������� ����
BOOL CMalwFManager::DriverSetExcept_FileDir(char* pczExceptDir)
{
	return MalwFind_DriverSetExcept_FileDir( pczExceptDir );
}
//File �������� ����
BOOL CMalwFManager::DriverClrExcept_FileDir(void)
{
	return MalwFind_DriverClrExcept_FileDir();
}


//File �������μ��� ���� 
BOOL CMalwFManager::DriverSetExcept_FileProc(char* pczExceptProc)
{
	return MalwFind_DriverSetExcept_FileProc( pczExceptProc );
}
//File �������μ��� ����
BOOL CMalwFManager::DriverClrExcept_FileProc(void)
{
	return MalwFind_DriverClrExcept_FileProc();
}


//File FDD ����� 
BOOL CMalwFManager::DriverSetFDDVolCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog )
{
	return MalwFind_DriverSetFDDVolCtrl( ulDrives, ulAcctl, bLog );
}
//File �̵�������ġ �����
BOOL CMalwFManager::DriverSetUSBVolCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog )
{
	return MalwFind_DriverSetUSBVolCtrl( ulDrives, ulAcctl, bLog );
}


//File Ȯ���� �����

// TRUE: Ȯ�������� ���ܸ���Ʈ FALSE:Ȯ�������� ���� ���ܸ���Ʈ
BOOL CMalwFManager::DriverSetFileExtCtrl( BOOL bFileExtDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczFileExt )
{
	return MalwFind_DriverSetFileExtCtrl( bFileExtDeny, ulDrives, ulAcctl, bLog, pczFileExt );
}
//File Ȯ���� ��������
BOOL CMalwFManager::DriverClrFileExtCtrl(void)
{
	return MalwFind_DriverClrFileExtCtrl();
}


//�������� �����
BOOL CMalwFManager::DriverSetSFolderCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog )
{
	return MalwFind_DriverSetSFolderCtrl( ulDrives, ulAcctl, bLog );
}
//�������� ��������
BOOL CMalwFManager::DriverClrSFolderCtrl(void)
{
	return MalwFind_DriverClrSFolderCtrl();
}


//�������� �������� ����
BOOL CMalwFManager::DriverSetExcept_SFDir(char* pczExceptDir)
{
	return MalwFind_DriverSetExcept_SFDir( pczExceptDir );
}
//�������� �������� ����
BOOL CMalwFManager::DriverClrExcept_SFDir(void)
{
	return MalwFind_DriverClrExcept_SFDir();
}


//�������� �������μ��� ���� 
BOOL CMalwFManager::DriverSetExcept_SFProc( char* pczExceptProc )
{
	return MalwFind_DriverSetExcept_SFProc( pczExceptProc );
}
//�������� �������μ��� ����
BOOL CMalwFManager::DriverClrExcept_SFProc(void)
{
	return MalwFind_DriverClrExcept_SFProc();
}




//���μ��� ��� �״°� ������ ����
BOOL CMalwFManager::DriverGetProcStat(char* pczOutBuffer, int nMaxOutBuffer)
{
	return MalwFind_DriverGetProcStat( pczOutBuffer, nMaxOutBuffer );
}


//Process �������� ����
BOOL CMalwFManager::DriverSetExcept_ProcDir( char* pczExceptDir )
{
	return MalwFind_DriverSetExcept_ProcDir( pczExceptDir );
}
//Process �������� ����
BOOL CMalwFManager::DriverClrExcept_ProcDir(void)
{
	return MalwFind_DriverClrExcept_ProcDir();
}


//Process ���� ����
BOOL CMalwFManager::DriverSetProcCtrl( BOOL bProcDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczProcCtrl )
{
	return MalwFind_DriverSetProcCtrl( bProcDeny, ulDrives, ulAcctl, bLog, pczProcCtrl );
}
//Process ���� ����
BOOL CMalwFManager::DriverClrProcCtrl(void)
{
	return MalwFind_DriverClrProcCtrl();
}




// �α� ��ġ ������
DWORD WINAPI 
CMalwFManager::ThreadProc_LogFetch( PVOID pParam )
{
	int       nItem=0;
	DWORD     dwRet = 0x00;
	ULONG     ulBufferSize = 0, ulPos = 0, ulLength = 0;
	CStringA  strText, strCount;
	char      czBuffer[ 1024 ];
	char*  pczToken    = NULL;
	char*  pczSubToken = NULL; 
	
	while(true)
	{
		dwRet = WaitForSingleObject( g_MalwFMgr.m_hLogFetchEvent, 2000 );
		if(dwRet == WAIT_OBJECT_0) break;

		RtlZeroMemory( czBuffer, sizeof(czBuffer) );
		g_MalwFMgr.Driver_GetLogFetch( czBuffer, sizeof(czBuffer) );
		ulLength = (ULONG)strlen( czBuffer );
		if(ulLength > 0)
		{
			pczToken = strtok( czBuffer, "||" );
			while( pczToken )
			{
				nItem = theApp.m_pDlg->m_RetListCtrl.GetItemCount();

				strText.Format ( "%s",    pczToken );
				strCount.Format( "Log%d", nItem    );
				theApp.m_pDlg->m_RetListCtrl.AddItem(nItem, 0, strCount);
				theApp.m_pDlg->m_RetListCtrl.AddItem(nItem, 1, strText);

				pczToken = strtok( NULL, "||" );
			}
		}
	}
	return 0;
}