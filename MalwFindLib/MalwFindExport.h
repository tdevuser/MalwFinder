
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

//드라이버 설치
MALWFIND_API BOOL __stdcall MalwFind_DriverInstall(char*  pDrvSymName, char* pDrvSvcName, char* pDrvFullPath, ULONG ulStartType, ULONG ulServiceType);
//드라이버 삭제
MALWFIND_API BOOL __stdcall MalwFind_DriverUnInstall(char* pDrvSvcName);
//드라이버 구동
MALWFIND_API BOOL __stdcall MalwFind_DriverStart(char* pDrvDeivceName, char* pDrvSvcName);
//드라이버 내림
MALWFIND_API BOOL __stdcall MalwFind_DriverStop( char* pDrvSvcName );


//드라이버 버전
MALWFIND_API BOOL __stdcall MalwFind_DriverGetVersion(void);
//로그 취합시작/정지
MALWFIND_API BOOL __stdcall MalwFind_DriverLogStart( ULONG ulLogStart );
//로그 Fetch
MALWFIND_API BOOL __stdcall MalwFind_DriverGetLogFetch( char* pOutBuffer, ULONG  ulOutBufferSize );
//전역플래그 설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetGlobalCtrl(BOOL bGlobalCtrl);
//설치폴더설정하기
MALWFIND_API BOOL __stdcall MalwFind_DriverSetupDir( char* pSetupDir, char* pSysDir );


//File 예외폴더 설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_FileDir(char* pczExceptDir);
//File 예외폴더 해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_FileDir(void);
//File 예외프로세스 설정 
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_FileProc(char* pczExceptProc);
//File 예외프로세스 해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_FileProc(void);

//File FDD 제어설정 
MALWFIND_API BOOL __stdcall MalwFind_DriverSetFDDVolCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
//File 이동저장장치 제어설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetUSBVolCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
//File 확장자 제어설정
// TRUE: 확장자제어 차단리스트 FALSE:확장자제어 차단 예외리스트
MALWFIND_API BOOL __stdcall MalwFind_DriverSetFileExtCtrl(BOOL bFileExtDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczFileExt );
//File 확장자 제어해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrFileExtCtrl(void);


//공유폴더 제어설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetSFolderCtrl( ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
//공유폴더 제어해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrSFolderCtrl(void);
//공유폴더 예외폴더 설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_SFDir(char* pczExceptDir);
//공유폴더 예외폴더 해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_SFDir(void);
//공유폴더 예외프로세스 설정 
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_SFProc( char* pczExceptProc );
//공유폴더 예외프로세스 해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_SFProc(void);


//프로세스 살고 죽는것 가지고 오기
MALWFIND_API BOOL __stdcall MalwFind_DriverGetProcStat(char* pczOutBuf, int nMaxOutBuf);
//Process 예외폴더 설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetExcept_ProcDir( char* pczExceptDir );
//Process 예외폴더 해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrExcept_ProcDir(void);
//Process 제어 설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetProcCtrl( BOOL bProcDeny, ULONG ulDrives, ULONG ulAcctl, BOOL bLog, char* pczProcCtrl );
//Process 제어 해제
MALWFIND_API BOOL __stdcall MalwFind_DriverClrProcCtrl(void);



//CdromCtrl Burning 제어설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetCdromCtrl( ULONG ulCommand, ULONG ulDrives, ULONG ulAcctl, BOOL bLog );
// MtpCtrl 제어설정
MALWFIND_API BOOL __stdcall MalwFind_DriverSetMtpCtrl( ULONG ulCmd, ULONG ulAcctl, BOOL bLog );
MALWFIND_API BOOL __stdcall MalwFind_GetHashValue( const unsigned char* pbEncKey, unsigned char* pbOutHashValue, int nMaxOutHashValue );






#ifdef __cplusplus
}
#endif

#endif
