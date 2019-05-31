#pragma once

class CMalwFManager
{
public:
	CMalwFManager(void);
	virtual ~CMalwFManager(void);
public:	
	int   GetShiftIndex( char cVol );
	ULONG GetVolumeTypeDrives( ULONG ulDrvType, char* pOutBuffer, int nMaxOutBuffer );
	ULONG GetFdd_Drives  ( char* pOutBuffer, int nMaxOutBuffer );
	ULONG GetUsb_Drives  ( char* pOutBuffer, int nMaxOutBuffer );
	ULONG GetCdrom_Drives( char* pOutBuffer, int nMaxOutBuffer );	
	ULONG GetFixed_Drives( char* pOutBuffer, int nMaxOutBuffer );
	ULONG GetNetwork_Drives( char* pOutBuffer, int nMaxOutBuffer );	
	void GetHomeDirectory( char* pczOutBuffer, int nMaxPath );	
	void SetInitParameter( char*  pDrvSymName,
						   char*  pDrvSvcName,
                           char*  pDrvFullPath,
					       ULONG  ulStartType,
					       ULONG  ulServiceType );
public:
	HANDLE m_hLogFetchEvent;
	static DWORD WINAPI ThreadProc_LogFetch( PVOID pParam );
public:	
	BOOL Driver_Install(void);   // ����̹� ��ġ
	BOOL Driver_UnInstall(void); // ����̹� ����	
	BOOL Driver_Start(void);     // ����̹� ����
	BOOL Driver_Stop(void);      // ����̹� ����

	//����̹� ����
	BOOL Driver_GetVersion(void);
	// �α� ���ս���/���� 
	BOOL Driver_LogStart( ULONG ulLogStart );
	// �α� Fetch
	BOOL Driver_GetLogFetch( char* pOutBuffer, ULONG  ulOutBufferSize );
	//�����÷��� ����
	BOOL Driver_SetGlobalCtrl(BOOL bGlobalCtrl);
	//��ġ���������ϱ�
	BOOL Driver_SetupDir(char* pSetupDir, char* pSysDir);


	//File �������� ����
	BOOL DriverSetExcept_FileDir(char* pczExceptDir);
	//File �������� ����
	BOOL DriverClrExcept_FileDir(void);
	//File �������μ��� ���� 
	BOOL DriverSetExcept_FileProc(char* pczExceptProc);
	//File �������μ��� ����
	BOOL DriverClrExcept_FileProc(void);


	//File FDD ����� 
	BOOL DriverSetFDDVolCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
	//File �̵�������ġ �����
	BOOL DriverSetUSBVolCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
	// TRUE: Ȯ�������� ���ܸ���Ʈ FALSE:Ȯ�������� ���� ���ܸ���Ʈ
	//File Ȯ���� �����
	BOOL DriverSetFileExtCtrl( BOOL bFileExtDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczFileExt );
	//File Ȯ���� ��������
	BOOL DriverClrFileExtCtrl(void);
	
	
	//�������� �����
	BOOL  DriverSetSFolderCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
	//�������� ��������
	BOOL  DriverClrSFolderCtrl(void);
	//�������� �������� ����
	BOOL  DriverSetExcept_SFDir(char* pczExceptDir);
	//�������� �������� ����
	BOOL  DriverClrExcept_SFDir(void);
	//�������� �������μ��� ���� 
	BOOL  DriverSetExcept_SFProc( char* pczExceptProc );
	//�������� �������μ��� ����
	BOOL  DriverClrExcept_SFProc(void);

	

	//���μ��� ��� �״°� ������ ����
	BOOL  DriverGetProcStat(char* pczOutBuffer, int nMaxOutBuffer);
	//Process �������� ����
	BOOL  DriverSetExcept_ProcDir( char* pczExceptDir );
	//Process �������� ����
	BOOL  DriverClrExcept_ProcDir(void);
	//Process ���� ����
	BOOL  DriverSetProcCtrl( BOOL bProcDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczProcCtrl );
	//Process ���� ����
	BOOL  DriverClrProcCtrl(void);

private:
	ULONG  m_ulDrvStartType;
	ULONG  m_ulDrvServiceType;
	char   m_czDrvSymName [MAX_PATH];
	char   m_czDrvSvcName [MAX_PATH];
	char   m_czDrvFullPath[MAX_PATH];
public:
	char* GetDrvSymName(void) { return m_czDrvSymName;  }
	char* GetDrvSvcName(void) { return m_czDrvSvcName;  }
	char* GetDrvFullPath(void){ return m_czDrvFullPath; }

};


extern CMalwFManager g_MalwFMgr;
