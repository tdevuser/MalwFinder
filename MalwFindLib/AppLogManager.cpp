#include "stdafx.h"

#include "AppLogManager.h"
#include <io.h>
#include <strsafe.h>
#pragma comment(lib, "strsafe.lib")

#pragma warning(disable:4995)
#pragma warning(disable:4996)

#define MAX_BUFFER    1024
#define MAX_FILESIZE  50000000
#define LOG_FILENAME  _T("MalwFindLib_Log.txt")


using namespace APP;
CAppLogManager g_LogManager;


CAppLogManager::CAppLogManager() 
: m_bLog(false), 
  m_pszFileName(NULL) 
{
	InitInstance();
}

CAppLogManager::~CAppLogManager()
{
	ExitInstance();
}


void CAppLogManager::InitInstance()
{	
	CString strFileName;
	
	if(!m_pszFileName)
	{
		m_pszFileName = new TCHAR[ MAX_PATH ];
		ASSERT( m_pszFileName );
		if(!m_pszFileName) return;
	}

	RtlZeroMemory( m_pszFileName, MAX_PATH );	
	strFileName = GetSetupDirectory();	
	StringCchCopy( m_pszFileName, MAX_PATH, strFileName.GetBuffer() );
	strFileName.ReleaseBuffer();
	/*
	nOSVer = GetWindowsVersion();
	if(nOSVer >= 6)
	{
		GetLocalAppDataPath( m_pczFileName, MAX_PATH );
		StringCchCatA( m_pczFileName, MAX_PATH, "\\" );
	}
	else
	{
		strFileNameA = GetHomeDir();	
		StringCchCopyA( m_pczFileName, MAX_PATH, strFileNameA.GetBuffer() );
		strFileNameA.ReleaseBuffer();
	}
	*/

	StringCchCat( m_pszFileName, MAX_PATH, LOG_FILENAME );

}



void CAppLogManager::ExitInstance()
{
	if(m_pszFileName)
	{
		delete [] m_pszFileName;
		m_pszFileName = NULL;
	}
}




void CAppLogManager::Write(const TCHAR* fmt, ...)
{
	SYSTEMTIME SysTime;
	FILE*  pFile = NULL;
	DWORD  dwFileSize = 0;
	TCHAR  szBuffer[ MAX_BUFFER ];
	TCHAR  szPrtBuffer[ MAX_BUFFER ];

	if(!m_bLog) return;

	RtlZeroMemory( szBuffer, MAX_BUFFER );
	RtlZeroMemory( &SysTime, sizeof(SysTime) );

	ASSERT( m_pszFileName && fmt );
	if(!m_pszFileName || !fmt) return;
	
	va_list	 ap;	
	va_start( ap, fmt );
	_vstprintf( szBuffer, fmt, ap );
	va_end(ap);
	
	GetLocalTime( &SysTime );
	_stprintf( szPrtBuffer, _T("[%04d%02d%02d-%02d:%02d:%02d] >> %s \n"), 
			   SysTime.wYear, SysTime.wMonth, SysTime.wDay,
			   SysTime.wHour, SysTime.wMinute, SysTime.wSecond, szBuffer );
		
	pFile = _tfopen( m_pszFileName, _T("a+t") );
	if(!pFile) return;

	dwFileSize = filelength( fileno( pFile ) );
	if(dwFileSize > MAX_FILESIZE)
	{
		fclose( pFile ); 
		_tunlink( m_pszFileName );

		pFile = _tfopen( m_pszFileName, _T("a+t") );
		if(!pFile) return;
	}

	fwrite( szPrtBuffer, 1, _tcslen( szPrtBuffer ), pFile );
	fclose( pFile );


}


int CAppLogManager::GetWindowsVersion()
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi)))
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if(!GetVersionEx((OSVERSIONINFO *)&osvi))  return 0;
	}

	return osvi.dwMajorVersion;

}


void CAppLogManager::GetLocalAppDataPath( char* pczOutPath, int nMaxLength )
{
	typedef UINT (CALLBACK* LPFNDLLFUNC)(GUID& rfid, DWORD dwFlags, HANDLE hToken, PWSTR* ppszPath);
	LPFNDLLFUNC  pGetKnownFldpathFnPtr = NULL;
	HINSTANCE    hInst = NULL;
	PWSTR        pwzPath = NULL;
	HRESULT      hr=0;
	int          nRet =0, nError=0;
	size_t       nLength=0;
	
	ASSERT( pczOutPath && nMaxLength );
	if(!pczOutPath || !nMaxLength) return;

	// Vista에서 "사용자/AppData/LocalLow" 경로에 대한 GUID값  
	GUID FOLDERID_LocalAppData = {0xA520A1A4, 0x1780, 0x4FF6, {0xBD, 0x18, 0x16, 0x73, 0x43, 0xC5, 0xAF, 0x16}};

	hInst = LoadLibrary( _T("shell32.dll") );
	pGetKnownFldpathFnPtr = (LPFNDLLFUNC)GetProcAddress( hInst, "SHGetKnownFolderPath" );
	if ( pGetKnownFldpathFnPtr && SUCCEEDED(pGetKnownFldpathFnPtr( FOLDERID_LocalAppData, 0, NULL, &pwzPath )) )
	{
		hr = StringCchLengthW( pwzPath, STRSAFE_MAX_CCH, (size_t*)&nLength );
		if( SUCCEEDED(hr) )
		{
			nRet = WideCharToMultiByte( CP_ACP, 0, pwzPath, -1, pczOutPath, nMaxLength, NULL, NULL );
			if(!nRet)
			{
				nError = GetLastError();	
				ASSERT( FALSE );		
			}
		}
		CoTaskMemFree( pwzPath );
	}
	FreeLibrary( hInst );

}

