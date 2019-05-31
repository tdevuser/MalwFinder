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

	// ����̹� ����
	// �α� ���ս���/����
	// �α���ġ
	// ��ü ��������-��ü�������̵� 
#define IOCTL_MALWF_GET_VERSION		  IOCTL_MALWF_CODE(0x800, FILE_ANY_ACCESS)
#define IOCTL_MALWF_LOG_START         IOCTL_MALWF_CODE(0x801, FILE_ANY_ACCESS)
#define IOCTL_MALWF_GET_LOGFETCH      IOCTL_MALWF_CODE(0x802, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_SETUPDIRCTRL  IOCTL_MALWF_CODE(0x805, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_SETUPDIR      IOCTL_MALWF_CODE(0x806, FILE_ANY_ACCESS)
	//FILE �������� ����
	//FILE �������μ��� ����
#define IOCTL_MALWF_SET_EXCEPT_FILEDIR   IOCTL_MALWF_CODE(0x811, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_FILEDIR   IOCTL_MALWF_CODE(0x812, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_EXCEPT_FILEPROC  IOCTL_MALWF_CODE(0x813, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_FILEPROC  IOCTL_MALWF_CODE(0x814, FILE_ANY_ACCESS)
//�����̸� ����
//FDD ����
//�̵�������ġ ����
//����Ȯ���� ����
#define IOCTL_MALWF_SET_FILENAMECTRL  IOCTL_MALWF_CODE(0x817, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_FILENAMECTRL  IOCTL_MALWF_CODE(0x818, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_FDDVOLCTRL    IOCTL_MALWF_CODE(0x821, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_USBVOLCTRL    IOCTL_MALWF_CODE(0x822, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_FILEEXTCTRL   IOCTL_MALWF_CODE(0x823, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_FILEEXTCTRL   IOCTL_MALWF_CODE(0x824, FILE_ANY_ACCESS)
//�������� ����
#define IOCTL_MALWF_SET_SFCTRL        IOCTL_MALWF_CODE(0x825, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_SFCTRL        IOCTL_MALWF_CODE(0x826, FILE_ANY_ACCESS)
//�������� �������� ����
//�������� �������μ��� ����
#define IOCTL_MALWF_SET_EXCEPT_SFDIR    IOCTL_MALWF_CODE(0x827, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_SFDIR    IOCTL_MALWF_CODE(0x828, FILE_ANY_ACCESS)
#define IOCTL_MALWF_SET_EXCEPT_SFPROC   IOCTL_MALWF_CODE(0x829, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_SFPROC   IOCTL_MALWF_CODE(0x82A, FILE_ANY_ACCESS)

//���μ��� �ݹ�//���μ��� ��� �״°� ������ ����
#define IOCTL_MALWF_GET_PROC_STAT       IOCTL_MALWF_CODE(0x841, FILE_ANY_ACCESS)
//PROCESS �������� ����
#define IOCTL_MALWF_SET_EXCEPT_PROCDIR   IOCTL_MALWF_CODE(0x842, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_EXCEPT_PROCDIR   IOCTL_MALWF_CODE(0x843, FILE_ANY_ACCESS)
//���μ��� ����
#define IOCTL_MALWF_SET_PROC_CTRL   IOCTL_MALWF_CODE(0x844, FILE_ANY_ACCESS)
#define IOCTL_MALWF_CLR_PROC_CTRL   IOCTL_MALWF_CODE(0x845, FILE_ANY_ACCESS)


// �Ǽ��ڵ�м�
#define IOCTL_MALWF_POLICY_REG      IOCTL_MALWF_CODE(0x871, FILE_ANY_ACCESS)
#define IOCTL_MALWF_POLICY_FILE     IOCTL_MALWF_CODE(0x872, FILE_ANY_ACCESS)
#define IOCTL_MALWF_POLICY_NETWORK  IOCTL_MALWF_CODE(0x873, FILE_ANY_ACCESS)
#define IOCTL_MALWF_ANALYSIS_START  IOCTL_MALWF_CODE(0x874, FILE_ANY_ACCESS)
*/

//����̹� ����
NTSTATUS DrvCmd_GetVersion(void);


//�α� ���ս���/����
NTSTATUS DrvCmd_LogThreadStart(IN PVOID pInBuffer);

//�α�  Fetch 
NTSTATUS 
DrvCmd_GetLogFetch( OUT PVOID pOutBuffer, IN ULONG ulOutBufferLength, OUT PIO_STATUS_BLOCK pIoStatus );


//��ġ ���丮 ����
NTSTATUS DrvCmd_SetupDirControl(IN PVOID pInBuffer);
//��ü ��������-��ü�������̵� 
NTSTATUS DrvCmd_Set_SetupDir(IN PVOID pInBuffer);

// FDD ����
NTSTATUS DrvCmd_F_Set_FddCtrl(IN PVOID pInBuffer);
//�̵�������ġ ����
NTSTATUS DrvCmd_F_Set_UsbCtrl( IN PVOID pInBuffer );

// �����̸����� ����/����
NTSTATUS DrvCmd_F_Set_FileNameCtrl(IN PVOID pInBuffer);
NTSTATUS DrvCmd_F_Clr_FileNameCtrl(IN PVOID pInBuffer);

// Ȯ���� ���� /����
// 0:��븮��Ʈ 1:���ܸ���Ʈ 
NTSTATUS DrvCmd_F_Set_FileExtCtrl( IN PVOID pInBuffer );
NTSTATUS DrvCmd_F_Clr_FileExtCtrl( IN PVOID pInBuffer );


// FILE �������� ����
NTSTATUS DrvCmd_F_Set_ExceptDir(IN PVOID pInBuffer);
// FILE �������� ����
NTSTATUS DrvCmd_F_Clr_ExceptDir(IN PVOID pInBuffer);
// FILE �������μ��� ����
NTSTATUS DrvCmd_F_Set_ExceptProc(IN PVOID pInBuffer);
// FILE �������μ��� ����
NTSTATUS DrvCmd_F_Clr_ExceptProc(IN PVOID pInBuffer);


//�������� ���� ���
NTSTATUS DrvCmd_SF_Set_SFolderCtrl( IN PVOID pInBuffer );
//�������� ���� ��
NTSTATUS DrvCmd_SF_Clr_SFolderCtrl( IN PVOID pInBuffer );


//�������� �������� ����
NTSTATUS DrvCmd_SF_Set_SFolderExceptDir( IN PVOID pInBuffer );
//�������� �������� ����
NTSTATUS DrvCmd_SF_Clr_SFolderExceptProc( IN PVOID pInBuffer );
//�������� �������μ��� ����
NTSTATUS DrvCmd_SF_Set_SFolderExceptProc( IN PVOID pInBuffer );
//�������� �������μ��� ����		
NTSTATUS DrvCmd_SF_Clr_SFolderExceptProc( IN PVOID pInBuffer );


// ���μ��� �ݹ� -- ���μ��� ��� �״°� ������ ����
NTSTATUS 
DrvCmd_P_Get_Process_Stat( IN PVOID  pInBuffer, 
						   OUT ULONG ulInBufferLength, 
						   OUT PVOID pOutBuffer, 
						   IN ULONG  ulOutBufferLength,
						   OUT PIO_STATUS_BLOCK  pIoStatus );

//PROCESS �������� ����/����
//PROCESS ���� ����/����
NTSTATUS DrvCmd_P_Set_ExceptDir( IN PVOID pInBuffer );
NTSTATUS DrvCmd_P_Clr_ExceptDir( IN PVOID pInBuffer );
NTSTATUS DrvCmd_P_Set_ProcCtrl(  IN PVOID pInBuffer );
NTSTATUS DrvCmd_P_Clr_ProcCtrl(  IN PVOID pInBuffer );


// �Ǽ��ڵ�м�
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