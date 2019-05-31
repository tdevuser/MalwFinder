
#ifndef _MALWFIND_EXPORT_H_
#define _MALWFIND_EXPORT_H_

#include <afx.h>
#include "MalwFindErrcode.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef  _MALWFIND_EXPORTS
#define MALWFIND_API  __declspec(dllexport)
#else
#define MALWFIND_API  __declspec(dllimport)
#endif


// Startup
MALWFIND_API BOOL __stdcall MalwFind_DriverStartup(void);
// Cleanup
MALWFIND_API BOOL __stdcall MalwFind_DriverCleanup(void);

//����̹� ��ġ
MALWFIND_API BOOL __stdcall MalwFind_DriverInstall(char*  pDrvSymName, char* pDrvSvcName, char* pDrvFullPath, ULONG ulStartType, ULONG ulServiceType);
//����̹� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverUnInstall(char* pDrvSvcName);
//����̹� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverStart(char* pDrvDeivceName, char* pDrvSvcName);
//����̹� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverStop( char* pDrvSvcName );


//����̹� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverGetVersion(void);
//�α� ���ս���/����
MALWFIND_API BOOL __stdcall MalwFind_DriverLogStart( ULONG ulLogStart );
//�α� Fetch
MALWFIND_API BOOL __stdcall MalwFind_DriverGetLogFetch( char* pOutBuffer, ULONG  ulOutBufferSize );
//�����÷��� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverSetGlobalCtrl(BOOL bGlobalCtrl);
//��ġ���������ϱ�
MALWFIND_API BOOL __stdcall MalwFind_DriverSetupDir( char* pSetupDir, char* pSysDir );


//File �������� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_FileDir(char* pczExceptDir);
//File �������� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_FileDir(void);
//File �������μ��� ���� 
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_FileProc(char* pczExceptProc);
//File �������μ��� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_FileProc(void);

//File FDD ����� 
MALWFIND_API BOOL __stdcall MalwFind_DriverSetFDDVolCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
//File �̵�������ġ �����
MALWFIND_API BOOL __stdcall MalwFind_DriverSetUSBVolCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
//File Ȯ���� �����
// TRUE: Ȯ�������� ���ܸ���Ʈ FALSE:Ȯ�������� ���� ���ܸ���Ʈ
MALWFIND_API BOOL __stdcall MalwFind_DriverSetFileExtCtrl(BOOL bFileExtDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczFileExt );
//File Ȯ���� ��������
MALWFIND_API BOOL __stdcall MalwFind_DriverClrFileExtCtrl(void);


//�������� �����
MALWFIND_API BOOL __stdcall MalwFind_DriverSetSFolderCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
//�������� ��������
MALWFIND_API BOOL __stdcall MalwFind_DriverClrSFolderCtrl(void);
//�������� �������� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_SFDir(char* pczExceptDir);
//�������� �������� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_SFDir(void);
//�������� �������μ��� ���� 
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_SFProc( char* pczExceptProc );
//�������� �������μ��� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_SFProc(void);


//���μ��� ��� �״°� ������ ����
MALWFIND_API BOOL __stdcall MalwFind_DriverGetProcStat(char* pczOutBuf, int nMaxOutBuf);
//Process �������� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_ProcDir( char* pczExceptDir );
//Process �������� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_ProcDir(void);
//Process ���� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverSetProcCtrl( BOOL bProcDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczProcCtrl );
//Process ���� ����
MALWFIND_API BOOL __stdcall MalwFind_DriverClrProcCtrl(void);



//CdromCtrl Burning �����
MALWFIND_API BOOL __stdcall MalwFind_DriverSetCdromCtrl( ULONG ulCommand, ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
// MtpCtrl �����
MALWFIND_API BOOL __stdcall MalwFind_DriverSetMtpCtrl( ULONG ulCmd, ULONG ulAcctl, BOOL bLog );
MALWFIND_API BOOL __stdcall MalwFind_GetHashValue( const unsigned char* pbEncKey, unsigned char* pbOutHashValue, int nMaxOutHashValue );






#ifdef __cplusplus
}
#endif

#endif
