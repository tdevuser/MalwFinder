#ifndef _MALWFIND_COMMAND_H_
#define _MALWFIND_COMMAND_H_

#include  <ntifs.h>
#include  "MalwFind_DataTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif


/************************************************************************************************************************************/
//  IOCTL Code Definition  >>  FILE_DEVICE_NHCAFLT
/************************************************************************************************************************************/
	/*
#define FILE_DEVICE_MALWFIND  0x00008200
#define IOCTL_MALWF_CODE(_CODE, _Access)  CTL_CODE( FILE_DEVICE_MALWFIND, _CODE, METHOD_BUFFERED, _Access)

	// 드라이버 버전
	// 로그 취합시작/정지
	// 로그패치
	// 자체 폴더제어-자체폴더하이드 
#define IOCTL_MALWF_GET_VERSION		  IOCTL_MALWF_CODE(0x800, FILE_ANY_ACCESS)
#define IOCTL_MALWF_LOG_START         IOCTL_MALWF_CODE(0x801, FILE_ANY_ACCESS)
#define IOCTL_MALWF_GET_LOGFETCH      IOCTL_MALWF_CODE(0x802, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_SETUPDIRCTRL  IOCTL_MALWF_CODE(0x805, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_SETUPDIR      IOCTL_MALWF_CODE(0x806, FILE_ANY_ACCESS)
	//FILE 예외폴더 설정
	//FILE 예외프로세스 설정
#define IOCTL_MALWF_SET_EXCEPT_FILEDIR   IOCTL_MALWF_CODE(0x811, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_FILEDIR   IOCTL_MALWF_CODE(0x812, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_EXCEPT_FILEPROC  IOCTL_MALWF_CODE(0x813, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_FILEPROC  IOCTL_MALWF_CODE(0x814, FILE_ANY_ACCESS)
//파일이름 제어
//FDD 제어
//이동저장장치 제어
//파일확장자 제어
#define IOCTL_MALWF_SET_FILENAMECTRL  IOCTL_MALWF_CODE(0x817, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_FILENAMECTRL  IOCTL_MALWF_CODE(0x818, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_FDDVOLCTRL    IOCTL_MALWF_CODE(0x821, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_USBVOLCTRL    IOCTL_MALWF_CODE(0x822, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_FILEEXTCTRL   IOCTL_MALWF_CODE(0x823, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_FILEEXTCTRL   IOCTL_MALWF_CODE(0x824, FILE_ANY_ACCESS)
//공유폴더 제어
#define IOCTL_MALWF_SET_SFCTRL        IOCTL_MALWF_CODE(0x825, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_SFCTRL        IOCTL_MALWF_CODE(0x826, FILE_ANY_ACCESS)
//공유폴더 예외폴더 설정
//공유폴더 예외프로세스 설정
#define IOCTL_MALWF_SET_EXCEPT_SFDIR    IOCTL_MALWF_CODE(0x827, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_SFDIR    IOCTL_MALWF_CODE(0x828, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_EXCEPT_SFPROC   IOCTL_MALWF_CODE(0x829, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_SFPROC   IOCTL_MALWF_CODE(0x82A, FILE_ANY_ACCESS)

//프로세스 콜백//프로세스 살고 죽는것 가지고 오기
#define IOCTL_MALWF_GET_PROC_STAT       IOCTL_MALWF_CODE(0x841, FILE_ANY_ACCESS)
//PROCESS 예외폴더 설정
#define IOCTL_MALWF_SET_EXCEPT_PROCDIR   IOCTL_MALWF_CODE(0x842, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_PROCDIR   IOCTL_MALWF_CODE(0x843, FILE_ANY_ACCESS)
//프로세스 제어
#define IOCTL_MALWF_SET_PROC_CTRL   IOCTL_MALWF_CODE(0x844, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_PROC_CTRL   IOCTL_MALWF_CODE(0x845, FILE_ANY_ACCESS)


// 악성코드분석
#define IOCTL_MALWF_POLICY_REG      IOCTL_MALWF_CODE(0x871, FILE_ANY_ACCESS)
#define IOCTL_MALWF_POLICY_FILE     IOCTL_MALWF_CODE(0x872, FILE_ANY_ACCESS)
#define IOCTL_MALWF_POLICY_NETWORK  IOCTL_MALWF_CODE(0x873, FILE_ANY_ACCESS)
#define IOCTL_MALWF_ANALYSIS_START  IOCTL_MALWF_CODE(0x874, FILE_ANY_ACCESS)
*/

//드라이버 버전
NTSTATUS DrvCmd_GetVersion(void);


//로그 취합시작/정지
NTSTATUS DrvCmd_LogThreadStart(IN PVOID pInBuffer);

//로그  Fetch 
NTSTATUS 
DrvCmd_GetLogFetch( OUT PVOID pOutBuffer, IN ULONG ulOutBufferLength, OUT PIO_STATUS_BLOCK pIoStatus );


//설치 디렉토리 제어
NTSTATUS DrvCmd_SetupDirControl(IN PVOID pInBuffer);
//자체 폴더제어-자체폴더하이드 
NTSTATUS DrvCmd_Set_SetupDir(IN PVOID pInBuffer);

// FDD 제어
NTSTATUS DrvCmd_F_Set_FddCtrl(IN PVOID pInBuffer);
//이동저장장치 제어
NTSTATUS DrvCmd_F_Set_UsbCtrl( IN PVOID pInBuffer );

// 파일이름제어 설정/해제
NTSTATUS DrvCmd_F_Set_FileNameCtrl(IN PVOID pInBuffer);
NTSTATUS DrvCmd_F_Clr_FileNameCtrl(IN PVOID pInBuffer);

// 확장자 제어 /해제
// 0:허용리스트 1:차단리스트 
NTSTATUS DrvCmd_F_Set_FileExtCtrl( IN PVOID pInBuffer );
NTSTATUS DrvCmd_F_Clr_FileExtCtrl( IN PVOID pInBuffer );


// FILE 예외폴더 설정
NTSTATUS DrvCmd_F_Set_ExceptDir(IN PVOID pInBuffer);
// FILE 예외폴더 해제
NTSTATUS DrvCmd_F_Clr_ExceptDir(IN PVOID pInBuffer);
// FILE 예외프로세스 설정
NTSTATUS DrvCmd_F_Set_ExceptProc(IN PVOID pInBuffer);
// FILE 예외프로세스 해제
NTSTATUS DrvCmd_F_Clr_ExceptProc(IN PVOID pInBuffer);


//공유폴더 제어 목록
NTSTATUS DrvCmd_SF_Set_SFolderCtrl( IN PVOID pInBuffer );
//공유폴더 해제 모
NTSTATUS DrvCmd_SF_Clr_SFolderCtrl( IN PVOID pInBuffer );


//공유폴더 예외폴더 설정
NTSTATUS DrvCmd_SF_Set_SFolderExceptDir( IN PVOID pInBuffer );
//공유폴더 예외폴더 해제
NTSTATUS DrvCmd_SF_Clr_SFolderExceptProc( IN PVOID pInBuffer );
//공유폴더 예외프로세스 설정
NTSTATUS DrvCmd_SF_Set_SFolderExceptProc( IN PVOID pInBuffer );
//공유폴더 예외프로세스 해제		
NTSTATUS DrvCmd_SF_Clr_SFolderExceptProc( IN PVOID pInBuffer );


// 프로세스 콜백 -- 프로세스 살고 죽는것 가지고 오기
NTSTATUS 
DrvCmd_P_Get_Process_Stat( IN PVOID  pInBuffer, 
						   OUT ULONG ulInBufferLength, 
						   OUT PVOID pOutBuffer, 
						   IN ULONG  ulOutBufferLength,
						   OUT PIO_STATUS_BLOCK  pIoStatus );

//PROCESS 예외폴더 설정/해제
//PROCESS 제어 설정/해제
NTSTATUS DrvCmd_P_Set_ExceptDir( IN PVOID pInBuffer );
NTSTATUS DrvCmd_P_Clr_ExceptDir( IN PVOID pInBuffer );
NTSTATUS DrvCmd_P_Set_ProcCtrl(  IN PVOID pInBuffer );
NTSTATUS DrvCmd_P_Clr_ProcCtrl(  IN PVOID pInBuffer );


// 악성코드분석
NTSTATUS DrvCmd_MalwFind_SetPolicyReg (IN PVOID pInBuffer);
NTSTATUS DrvCmd_MalwFind_SetPolicyFile(IN PVOID pInBuffer);
NTSTATUS DrvCmd_MalwFind_SetPolicyNetwork(IN PVOID pInBuffer);
NTSTATUS DrvCmd_MalwFind_SetPolicyJobStart(IN PVOID pInBuffer);



NTSTATUS
FltDrv_DeviceControl( IN PFILE_OBJECT   pFileObject, 
				      IN PDEVICE_OBJECT pDeviceObject,
				      IN ULONG   ulIoControlCode, 
				      IN PVOID   pInBuffer, 
				      IN ULONG   ulInBufferLen, 
				      OUT PVOID  pOutBuffer, 
				      IN ULONG   ulOutBufferLen,
				      OUT PIO_STATUS_BLOCK pOutIoStatus );








#ifdef __cplusplus
}
#endif

#endif