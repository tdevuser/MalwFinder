
#include "StdAfx.h"
#include "ConfigSvc.h"

using namespace APP;

const TCHAR* SAFE_MINIMAL = _T("System\\CurrentControlSet\\Control\\SafeBoot\\Minimal");
const TCHAR* SAFE_NETWORK = _T("System\\CurrentControlSet\\Control\\SafeBoot\\Network");


CConfigSvc::CConfigSvc() 
: m_ulServiceType(0), 
  m_ulStartType(0), 
  m_pServiceName(NULL), 
  m_pServiceImagePath(NULL)
{
}


CConfigSvc::~CConfigSvc(void)
{
	if(m_pServiceImagePath)
	{
		delete [] m_pServiceImagePath;
		m_pServiceImagePath = NULL;
	}

	if(m_pServiceName)
	{
		delete [] m_pServiceName;
		m_pServiceName = NULL;
	}

}


// ServiceName  Get
TCHAR*  CConfigSvc::GetServiceName(void)
{
	return m_pServiceName;
}

// ServiceName Set
void CConfigSvc::SetServiceName(const TCHAR* pszServiceName, ULONG ulServiceNameLen )
{
	ASSERT( pszServiceName );
	if(!pszServiceName) return;

	if(m_pServiceName)
	{
		delete [] m_pServiceName;
		m_pServiceName = NULL;
	}

	m_pServiceName = new TCHAR[ ulServiceNameLen+1 ];
	if(m_pServiceName)
	{
		RtlZeroMemory( m_pServiceName, ulServiceNameLen+1 );
		_tcsncpy_s( m_pServiceName, (ulServiceNameLen+1), pszServiceName, ulServiceNameLen );
	}

}

// /*************************************************************************************************************/


// ServiceImagepath  
TCHAR*  CConfigSvc::GetServiceImagePath(void)
{
	return m_pServiceImagePath;
}


void CConfigSvc::SetServiceImagePath(const TCHAR* pszServiceImagePath, ULONG  ulServiceImagePathLen )
{
	ASSERT( pszServiceImagePath );
	if(!pszServiceImagePath) return;

	if(!m_pServiceImagePath)
	{
		delete [] m_pServiceImagePath;
		m_pServiceImagePath = NULL;
	}

	m_pServiceImagePath = new TCHAR[ ulServiceImagePathLen+1 ];
	ASSERT( m_pServiceImagePath );
	if(m_pServiceImagePath)
	{
		_tcsncpy_s( m_pServiceImagePath, (ulServiceImagePathLen+1), pszServiceImagePath, ulServiceImagePathLen );
	}

}


// /*************************************************************************************************************/

// ServiceType
ULONG  CConfigSvc::GetServiceType(void)
{
	return m_ulServiceType;
}

// ServiceType
void  CConfigSvc::SetServiceType( ULONG ulServiceType )
{
	m_ulServiceType = ulServiceType;
}

// /*************************************************************************************************************/

// ServiceType SERVICE_FILE_SYSTEM_DRIVER
void  CConfigSvc::SetServiceType_SystemDriver(void)
{	// 0x00000002   SERVICE_FILE_SYSTEM_DRIVER
	m_ulServiceType = SERVICE_FILE_SYSTEM_DRIVER;
}

// ServiceType  SERVICE_KERNEL_DRIVER
void  CConfigSvc::SetServiceType_KernelDriver(void)
{	// SERVICE_KERNEL_DRIVER  0x00000001
	m_ulServiceType = SERVICE_KERNEL_DRIVER;
}


// ServiceType  SERVICE_WIN32_OWN_PROCESS
void  CConfigSvc::SetServiceType_Win32OwnProcess(void)
{	// SERVICE_KERNEL_DRIVER  0x00000010
	m_ulServiceType = SERVICE_WIN32_OWN_PROCESS;
}

// ServiceType  SERVICE_WIN32_SHARE_PROCESS
void  CConfigSvc::SetServiceType_Win32ShareProcess(void)
{	// SERVICE_KERNEL_DRIVER  0x00000020
	m_ulServiceType = SERVICE_WIN32_SHARE_PROCESS;
}


// /*************************************************************************************************************/

// StartType 
ULONG  CConfigSvc::GetStartType(void)
{
	return m_ulStartType;
}

// Start Type
void  CConfigSvc::SetStartType( ULONG ulStartType )
{
	m_ulStartType = ulStartType;
}

// /*************************************************************************************************************/

// SERVICE_BOOT_START 0x00000000
void  CConfigSvc::SetStartType_BootStart(void)
{
	m_ulStartType = SERVICE_BOOT_START;
}

// SERVICE_SYSTEM_START 0x00000001
void  CConfigSvc::SetStartType_SystemStart(void)
{
	m_ulStartType = SERVICE_SYSTEM_START;
}

// SERVICE_AUTO_START  0x00000002
void  CConfigSvc::SetStartType_AutoStart(void)
{
	m_ulStartType = SERVICE_AUTO_START;
}

// SERVICE_DEMAND_START 0x00000003
void  CConfigSvc::SetStartType_DemandStart(void)
{
	m_ulStartType = SERVICE_DEMAND_START;
}

// /*************************************************************************************************************/




void CConfigSvc::StopService(void)
{
}

BOOL CConfigSvc::ExistService(TCHAR*  pszSvcName)
{
	SC_HANDLE  hScmManager = NULL;
	SC_HANDLE  hService    = NULL;

	ASSERT( pszSvcName );
	if(!pszSvcName) return FALSE;

	hScmManager = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );  // full access rights 
    if(!hScmManager) return FALSE;

	hService = ::OpenService( hScmManager, pszSvcName, SERVICE_ALL_ACCESS ); 
	if(!hService) 
	{
		::CloseServiceHandle( hScmManager );
		hScmManager = NULL;
		return FALSE;
	}

	::CloseServiceHandle( hScmManager );
	hScmManager = NULL;

	::CloseServiceHandle( hService );
	hService = NULL;

	return TRUE;

}




BOOL CConfigSvc::InstallService(void)
{
    SC_HANDLE hScmManager = NULL;
	SC_HANDLE hService    = NULL;
    TCHAR  szFilePath[MAX_PATH];
	
	DWORD  dwRet = 0;
	DWORD  dwServiceType = 0x00;
	DWORD  dwStartType   = 0x00;

	ASSERT( m_pServiceName );
	if(!m_pServiceName) return FALSE;

	RtlZeroMemory( szFilePath, sizeof(szFilePath) );
	dwRet = GetModuleFileName( NULL, szFilePath, MAX_PATH );
    if(!dwRet) return FALSE;

    hScmManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );  // full access rights 
    if(!hScmManager) return FALSE;

	hService = ::CreateService( hScmManager, 
							    m_pServiceName, 
								m_pServiceName, 
								SERVICE_ALL_ACCESS, 
								m_ulServiceType,
								m_ulStartType,							
								SERVICE_ERROR_NORMAL,
								m_pServiceImagePath, 
								NULL,                      
								NULL,                      
								NULL,                      
								NULL,                      
								NULL   );                 
 
    if(!hService) 
    {
        CloseServiceHandle( hScmManager );
		hScmManager = NULL;
        return FALSE;
    }
  
    CloseServiceHandle( hService );
	hService = NULL;

    CloseServiceHandle( hScmManager );
	hScmManager = NULL;

	return TRUE;

}



BOOL CConfigSvc::DeleteService(void)
{
    SC_HANDLE hScmManager = NULL;
    SC_HANDLE hService    = NULL;

	ASSERT( m_pServiceName );
	if(!m_pServiceName) return FALSE;

	hScmManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );  // full access rights 
	if(!hScmManager) return FALSE;

	hService = ::OpenService( hScmManager, m_pServiceName, DELETE );            // need delete access 
    if(!hService) 
	{
		CloseServiceHandle( hService );
        return FALSE;
    }

	::DeleteService( hService );

    CloseServiceHandle( hService ); 
	hService = NULL;
    
	CloseServiceHandle( hScmManager );
	hScmManager = NULL;

	return TRUE;

}



BOOL CConfigSvc::SafeModeInstallService(const TCHAR* pServiceName, ULONG ulServiceNameLen )
{
	return FALSE;
	/*
	int nRet = 0;

	HKEY   hKey = NULL;
	DWORD  dwType = REG_SZ;
	TCHAR  szSafeMinimal[ MAX_PATH ], szSafeNetwork[ MAX_PATH ];

	ASSERT( pServiceName );
	if(!pServiceName) return;

	RtlZeroMemory( szSafeMinimal, MAX_PATH );
	RtlZeroMemory( szSafeNetwork, MAX_PATH );

	_stprintf_s( szSafeMinimal, MAX_PATH, _T("%s\\%s"), SAFE_MINIMAL, pServiceName );
	_stprintf_s( szSafeNetwork, MAX_PATH, _T("%s\\%s"), SAFE_NETWORK, pServiceName );


	nRet = RegCreateKey( HKEY_LOCAL_MACHINE, szSafeMinimal, &hKey );
	if(nRet == ERROR_SUCCESS)
	{
		nRet = RegSetKeyValue( hKey, NULL, _T(""), dwType, (LPCVOID)_T("Service"), (DWORD)_tcslen(_T("Service")) );
		if(nRet == ERROR_SUCCESS)
		{
		}
		RegCloseKey( hKey );
		hKey = NULL;
	}

	nRet = RegCreateKey( HKEY_LOCAL_MACHINE, szSafeNetwork, &hKey );
	if(nRet == ERROR_SUCCESS)
	{
		nRet = RegSetKeyValue( hKey, NULL, _T(""), dwType, (LPCVOID)_T("Service"), (DWORD)_tcslen(_T("Service")) );
		if(nRet == ERROR_SUCCESS)
		{
		}
		RegCloseKey( hKey );
		hKey = NULL;
	}
*/
	
}


