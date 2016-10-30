#pragma once

#include "MyListCtrl.h"
#include "../Common/MalwFind_Comm.h"


class CMalwFinderDlg : public CDialogEx
{
public:
	typedef struct _VolPolicy
	{
		BOOL   bLog;
		ULONG  ulAcctl;
		ULONG  ulDrives;
		char   czDrives[MAX_PATH];
		_VolPolicy()
		{
			bLog     = FALSE;
			ulAcctl  = 0;
			ulDrives = 0; 
			RtlZeroMemory( czDrives, MAX_PATH );
		}
	} VolPolicy, *PVolPolicy;

public:
	CMalwFinderDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	enum { IDD = IDD_NHCAFLTTEST_DIALOG };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

private:
	void UpdateCheckBoxState(void);

public:
	CButton   m_btnLog;
	CButton   m_btnLogStart;
	CButton   m_btnLogStop;
	CButton   m_btnAccess;
	CButton   m_btnCreate;
	CButton   m_btnOpen;
	CButton   m_btnRead;
	CButton   m_btnWrite;
	CButton   m_btnExecute;
	CButton   m_btnDelete;
	CButton   m_btnRename;

	CStatic   m_lblSvcName;
	CStatic   m_lblDrvFullPath;
	CEdit     m_editSvcName;
	CEdit     m_editDrvFullPath;

	CButton   m_btnDrvInstall;
	CButton   m_btnDrvUnInstall;
	CButton   m_btnDrvStart;
	CButton   m_btnDrvStop;	
	CButton   m_btnDrvSetupDir_Ctrl;
	CButton   m_btnDrvSetupDir;

	// Reg-Policy
	// File-Policy
	// Process-Policy
	// Network-Policy
	TCHAR    m_szProcName[MAX_PATH];
	TCHAR    m_szProcPath[MAX_PATH];

	CButton  m_btn_JobStart;
	CButton  m_btn_PolicyProc;
	CEdit    m_edit_PolicyProc;
	CButton  m_btn_PolicyReg;
	CButton  m_btn_PolicyFile;
	CButton  m_btn_PolicyNetwork;
	CStatic  m_Lbl_MalText;
	CMyListCtrl m_PolicyListCtrl;
	CMyListCtrl m_RetListCtrl;
		
	// FDD 제어설정
	// USB 이동저장장치 제어설정
	// 확장자 제어설정, 확장자 제어해제
	// 공유폴더 제어설정, 공유폴더 제어해제
	CButton  m_btnDrv_F_SetExtCtrl_Black;
	CButton  m_btnDrv_F_ClrExtCtrl;

		
	//프로세스 통계
	CButton  m_btnDrv_P_ProcStat;
	//Process 제어 설정/해제
	CButton  m_btnDrv_P_SetProcCtrl;
	CButton  m_btnDrv_P_ClrProcCtrl;
			
	CButton   m_btnClear;
	CButton   m_btnOK;
	CButton   m_btnCancel;

public:
	char       m_czEncKey[ MAX_ENCKEY_LEN ];
	VOL_DATA   m_EncDrv;
	VOL_DATA   m_EncPolicy;

public:
	BOOL       m_bLog;
	ULONG      m_ulAcctl;

	VolPolicy  m_Policy_Fdd;
	VolPolicy  m_Policy_Usb;	
	VolPolicy  m_Policy_Fixed;
	VolPolicy  m_Policy_Cdrom;

	VolPolicy  m_Policy_SFolder;
	VolPolicy  m_Policy_Proc;

protected:
	void Initial_ListCtrl(CListCtrl* pListCtrl);
	void Initial_PolicyListCtrl(CListCtrl* pListCtrl);
	
protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	
public:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnBtnClk_LogStart(void);
	afx_msg void OnBtnClk_LogStop(void);

	afx_msg void OnBtnClk_CheckLog(void);     
	afx_msg void OnBtnClk_CheckAccess(void);  
	afx_msg void OnBtnClk_CheckCreate(void);  
	afx_msg void OnBtnClk_CheckOpen(void);
	afx_msg void OnBtnClk_CheckRead(void);    
	afx_msg void OnBtnClk_CheckWrite(void);   
	afx_msg void OnBtnClk_CheckExecute(void); 
	afx_msg void OnBtnClk_CheckDelete(void);  
	afx_msg void OnBtnClk_CheckRename(void);  

	afx_msg void OnBtnClk_DriverInstall(void);
	afx_msg void OnBtnClk_DriverUninstall(void);
	afx_msg void OnBtnClk_DriverStart(void);       
	afx_msg void OnBtnClk_DriverStop(void);   
	afx_msg void OnBtnClk_DriverSetGlobalCtrl(void);
	afx_msg void OnBtnClk_DriverSetupDir(void);     
	
	// 프로세스 선택
	afx_msg void OnBtnClk_SelectProcess(void);
	// 레지스트리
	// 파일
	// 네트워크
	// 분석시작
	afx_msg void OnBtnClk_PolicyReg(void);
	afx_msg void OnBtnClk_PolicyFile(void);
	afx_msg void OnBtnClk_PolicyNetwork(void);
	afx_msg void OnBtnClk_JobStart(void);

	afx_msg void OnBtnClk_DriverSetFDDVolCtrl(void);     
	afx_msg void OnBtnClk_DriverSetUSBVolCtrl(void); 	
	afx_msg void OnBtnClk_DriverSetExtCtrl(void); 
	afx_msg void OnBtnClk_DriverClrExtCtrl(void); 
	
	afx_msg void OnBtnClk_DriverSetSFolderCtrl(void);
	afx_msg void OnBtnClk_DriverClrSFolderCtrl(void);	
	afx_msg void OnBtnClk_DriverSetExcept_SFDir(void);  
	afx_msg void OnBtnClk_DriverClrExcept_SFDir(void);  	
	afx_msg void OnBtnClk_DriverSetExcept_SFProc(void); 
	afx_msg void OnBtnClk_DriverClrExcept_SFProc(void); 
		
	afx_msg void OnBtnClk_DriverGetProcStat(void);       	
	afx_msg void OnBtnClk_DriverSetProcCtrl(void);    
	afx_msg void OnBtnClk_DriverClrProcCtrl(void);    
		
	afx_msg void OnBtnClk_Clear();
	afx_msg void OnBtnClk_Ok();
	afx_msg void OnBtnClk_Cancel();
	DECLARE_MESSAGE_MAP()

};



