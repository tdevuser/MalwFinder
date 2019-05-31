
// NHCAFltTestDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "MalwFinder.h"
#include "MalwFinderDlg.h"
#include "MalwFManager.h"
#include "../Common/MalwFind_Comm.h"


BOOL g_GlobalCtrl = FALSE;


#define MSG_OK      _T("확인")
#define MSG_CANCEL  _T("취소")

#define MSG_DRV_INSTALL    _T("드라이버설치")
#define MSG_DRV_UNINSTALL  _T("드라이버삭제")
#define MSG_DRV_START      _T("드라이버시작")
#define MSG_DRV_STOP       _T("드라이버중지")

#define MSG_DRV_SETUPDIR_CTRL _T("설치폴더제어")
#define MSG_DRV_SETUPDIR   _T("설치폴더설정")


#define IDC_EDIT_SVCNAME            49081
#define IDC_EDIT_IMAGEPATH          49082

#define IDC_DRV_INSTALL_BTN         49091
#define IDC_DRV_UNINSTALL_BTN       49092
#define IDC_DRV_START_BTN           49093
#define IDC_DRV_STOP_BTN            49094
#define IDC_DRV_SETUPDIR_BTN        49095
#define IDC_DRV_SETUPDIR_CTRL_BTN   49096

#define IDC_DRV_LOG_START           49901
#define IDC_DRV_LOG_STOP            49902

#define IDC_DRV_CHECK_LOG           50001
#define IDC_DRV_CHECK_ACCESS        50002
#define IDC_DRV_CHECK_CREATE        50003
#define IDC_DRV_CHECK_OPEN          50004
#define IDC_DRV_CHECK_READ          50005
#define IDC_DRV_CHECK_WRITE         50006
#define IDC_DRV_CHECK_EXECUTE       50007
#define IDC_DRV_CHECK_DELETE        50008
#define IDC_DRV_CHECK_RENAME        50009


#define IDC_DRV_F_SET_FDDVOL_CTRL  50011
#define IDC_DRV_F_SET_USBVOL_CTRL  50012

#define IDC_EDIT_F_FILE_EXT         50013
#define IDC_EDIT_F_EXCEPT_DIR       50014
#define IDC_EDIT_F_EXCEPT_PROC      50015 

#define IDC_DRV_F_SET_EXT_CTRL      50016
#define IDC_DRV_F_CLR_EXT_CTRL      50017

#define IDC_DRV_F_SET_EXCEPT_DIR    50018
#define IDC_DRV_F_CLR_EXCEPT_DIR    50019
#define IDC_DRV_F_SET_EXCEPT_PROC   50020
#define IDC_DRV_F_CLR_EXCEPT_PROC   50021

#define IDC_DRV_SF_SET_SFOLDER_CTRL 50022
#define IDC_DRV_SF_CLR_SFOLDER_CTRL 50023
#define IDC_DRV_SF_SET_EXCEPT_DIR   50024
#define IDC_DRV_SF_CLR_EXCEPT_DIR   50025
#define IDC_DRV_SF_SET_EXCEPT_PROC  50026
#define IDC_DRV_SF_CLR_EXCEPT_PROC  50027

#define IDC_EDIT_P_PROCESS          50031
#define IDC_EDIT_P_EXCEPT_DIR       50032

#define IDC_DRV_P_PROC_STAT         50033
#define IDC_DRV_P_SET_PROC_CTRL     50034
#define IDC_DRV_P_CLR_PROC_CTRL     50035
#define IDC_DRV_P_SET_EXCEPT_DIR    50036
#define IDC_DRV_P_CLR_EXCEPT_DIR    50037

#define IDC_DRV_SET_USBVOLUME_ENCRYPT     51051
#define IDC_DRV_SET_USBFOLDER_ENCRYPT     51052 
#define IDC_DRV_SET_USBFOLDER_EXCEPTCTRL  51053

#define IDC_DRV_QUERY_ENCFILE     51055
#define IDC_DRV_GET_ENCFILE_STAT  51056
#define IDC_DRV_SET_CDROM_CTRL    51057
#define IDC_DRV_SET_MTP_CTRL      51058

#define IDC_CLEAR_BTN     52000
#define IDC_OK_BTN        52001
#define IDC_CANCEL_BTN    52002
#define IDC_DRV_LISTCTRL  55005
#define MAX_1024_BUFFER   1024


#define IDC_POLICY_JOB_START       89999
#define IDC_POLICY_SELECT_PROCESS  90001
#define IDC_EDIT_PROCPATH          90002
#define IDC_POLICY_REG             90003
#define IDC_POLICY_FILE            90004
#define IDC_POLICY_NETWORK         90005
#define IDC_POLICY_LISTCTRL        90006


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


CMalwFinderDlg::CMalwFinderDlg(CWnd* pParent /*=NULL*/) 
: CDialogEx(CMalwFinderDlg::IDD, pParent)
{
	
	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );

	RtlZeroMemory( m_czEncKey,   MAX_ENCKEY_LEN );
	RtlZeroMemory( &m_EncDrv,    sizeof(m_EncDrv) );
	RtlZeroMemory( &m_EncPolicy, sizeof(m_EncPolicy) );

	m_bLog  = TRUE;
	m_ulAcctl = MALWF_CREATE | MALWF_WRITE | MALWF_DELETE | MALWF_RENAME;
	// FLT_OPEN | FLT_READ |
	RtlZeroMemory( &m_Policy_Fdd,   sizeof(m_Policy_Fdd)   );
	RtlZeroMemory( &m_Policy_Usb,   sizeof(m_Policy_Usb)   );
	RtlZeroMemory( &m_Policy_Fixed, sizeof(m_Policy_Fixed) );
	RtlZeroMemory( &m_Policy_Cdrom, sizeof(m_Policy_Cdrom) );
	RtlZeroMemory( &m_Policy_SFolder, sizeof(m_Policy_SFolder) );
	RtlZeroMemory( &m_Policy_Proc,    sizeof(m_Policy_Proc)    );
	
	RtlZeroMemory(m_szProcName, MAX_PATH); 
	RtlZeroMemory(m_szProcPath, MAX_PATH);
}


void CMalwFinderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

#define IDC_DRV_CHECK_LOG           50001
#define IDC_DRV_CHECK_ACCESS        50002
#define IDC_DRV_CHECK_CREATE        50003
#define IDC_DRV_CHECK_OPEN          50004
#define IDC_DRV_CHECK_READ          50005
#define IDC_DRV_CHECK_WRITE         50006
#define IDC_DRV_CHECK_EXECUTE       50007
#define IDC_DRV_CHECK_DELETE        50008
#define IDC_DRV_CHECK_RENAME        50009

BEGIN_MESSAGE_MAP(CMalwFinderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CREATE()
	ON_WM_SIZE()

	ON_BN_CLICKED( IDC_DRV_LOG_START,       OnBtnClk_LogStart     )
	ON_BN_CLICKED( IDC_DRV_LOG_STOP,        OnBtnClk_LogStop      )
	ON_BN_CLICKED( IDC_DRV_CHECK_LOG,       OnBtnClk_CheckLog     )

	ON_BN_CLICKED( IDC_DRV_CHECK_ACCESS,    OnBtnClk_CheckAccess   )	
	ON_BN_CLICKED( IDC_DRV_CHECK_CREATE,    OnBtnClk_CheckCreate   )
	ON_BN_CLICKED( IDC_DRV_CHECK_OPEN,      OnBtnClk_CheckOpen     )
	ON_BN_CLICKED( IDC_DRV_CHECK_READ,      OnBtnClk_CheckRead     )
	ON_BN_CLICKED( IDC_DRV_CHECK_WRITE,     OnBtnClk_CheckWrite    )
	ON_BN_CLICKED( IDC_DRV_CHECK_EXECUTE,   OnBtnClk_CheckExecute  )
	ON_BN_CLICKED( IDC_DRV_CHECK_DELETE,    OnBtnClk_CheckDelete   )
	ON_BN_CLICKED( IDC_DRV_CHECK_RENAME,    OnBtnClk_CheckRename   )
	
	ON_BN_CLICKED( IDC_DRV_INSTALL_BTN,      OnBtnClk_DriverInstall       )
	ON_BN_CLICKED( IDC_DRV_UNINSTALL_BTN,    OnBtnClk_DriverUninstall     )
	ON_BN_CLICKED( IDC_DRV_START_BTN,        OnBtnClk_DriverStart         )
	ON_BN_CLICKED(IDC_DRV_STOP_BTN,          OnBtnClk_DriverStop          )
	ON_BN_CLICKED(IDC_DRV_SETUPDIR_CTRL_BTN, OnBtnClk_DriverSetGlobalCtrl )
	ON_BN_CLICKED(IDC_DRV_SETUPDIR_BTN,      OnBtnClk_DriverSetupDir)
	 	
	
	ON_BN_CLICKED(IDC_POLICY_SELECT_PROCESS, OnBtnClk_SelectProcess)
	ON_BN_CLICKED(IDC_POLICY_REG,       OnBtnClk_PolicyReg)
	ON_BN_CLICKED(IDC_POLICY_FILE,      OnBtnClk_PolicyFile)
	ON_BN_CLICKED(IDC_POLICY_NETWORK,   OnBtnClk_PolicyNetwork)
	ON_BN_CLICKED(IDC_POLICY_JOB_START, OnBtnClk_JobStart)

	

	ON_BN_CLICKED(IDC_DRV_F_SET_FDDVOL_CTRL, OnBtnClk_DriverSetFDDVolCtrl )
	ON_BN_CLICKED(IDC_DRV_F_SET_USBVOL_CTRL, OnBtnClk_DriverSetUSBVolCtrl )
	ON_BN_CLICKED(IDC_DRV_F_SET_EXT_CTRL,     OnBtnClk_DriverSetExtCtrl)
	ON_BN_CLICKED(IDC_DRV_F_CLR_EXT_CTRL,    OnBtnClk_DriverClrExtCtrl)

	ON_BN_CLICKED(IDC_DRV_SF_SET_SFOLDER_CTRL, OnBtnClk_DriverSetSFolderCtrl)
	ON_BN_CLICKED(IDC_DRV_SF_CLR_SFOLDER_CTRL, OnBtnClk_DriverClrSFolderCtrl)
	ON_BN_CLICKED(IDC_DRV_SF_SET_EXCEPT_DIR,   OnBtnClk_DriverSetExcept_SFDir)
	ON_BN_CLICKED(IDC_DRV_SF_CLR_EXCEPT_DIR,   OnBtnClk_DriverClrExcept_SFDir)
	ON_BN_CLICKED(IDC_DRV_SF_SET_EXCEPT_PROC,  OnBtnClk_DriverSetExcept_SFProc)
	ON_BN_CLICKED(IDC_DRV_SF_CLR_EXCEPT_PROC,  OnBtnClk_DriverClrExcept_SFProc)

	ON_BN_CLICKED(  IDC_DRV_P_PROC_STAT,        OnBtnClk_DriverGetProcStat        )         
	
	ON_BN_CLICKED(  IDC_DRV_P_SET_PROC_CTRL,    OnBtnClk_DriverSetProcCtrl)
	ON_BN_CLICKED(  IDC_DRV_P_CLR_PROC_CTRL,    OnBtnClk_DriverClrProcCtrl)

	ON_BN_CLICKED(  IDC_CLEAR_BTN,                CMalwFinderDlg::OnBtnClk_Clear   )
	ON_BN_CLICKED(  IDC_OK_BTN,                  &CMalwFinderDlg::OnBtnClk_Ok     )
	ON_BN_CLICKED(  IDC_CANCEL_BTN,              &CMalwFinderDlg::OnBtnClk_Cancel )

END_MESSAGE_MAP()




BOOL CMalwFinderDlg::OnInitDialog()
{
	CString  strSvcName, strDrvFullPath;

	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	strSvcName     = g_MalwFMgr.GetDrvSvcName();
	strDrvFullPath = g_MalwFMgr.GetDrvFullPath();	

	if(m_editSvcName.GetSafeHwnd()) m_editSvcName.SetWindowText( strSvcName );
	if(m_editDrvFullPath.GetSafeHwnd()) m_editDrvFullPath.SetWindowTextW( strDrvFullPath );

	UpdateCheckBoxState();

	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		
	return TRUE;  

}



void CMalwFinderDlg::UpdateCheckBoxState(void)
{
	m_btnLog.SetCheck( m_bLog );

	if(!m_ulAcctl) 
	{
		m_btnAccess.SetCheck( true  );

		m_btnCreate.SetCheck ( false );
		m_btnOpen.SetCheck   ( false );
		m_btnRead.SetCheck   ( false );
		m_btnWrite.SetCheck  ( false );
		m_btnExecute.SetCheck( false );
		m_btnDelete.SetCheck ( false );
		m_btnRename.SetCheck ( false );
	}
	else
	{
		m_btnAccess.SetCheck( false  );

		if(m_ulAcctl & MALWF_CREATE) m_btnCreate.SetCheck( true );
		else                       m_btnCreate.SetCheck( false );

		if (m_ulAcctl & MALWF_OPEN) m_btnOpen.SetCheck(true);
		else                     m_btnOpen.SetCheck( false );

		if (m_ulAcctl & MALWF_READ) m_btnRead.SetCheck(true);
		else                     m_btnRead.SetCheck( false );

		if (m_ulAcctl & MALWF_WRITE) m_btnWrite.SetCheck(true);
		else                      m_btnWrite.SetCheck( false );

		if (m_ulAcctl & MALWF_EXECUTE) m_btnExecute.SetCheck(true);
		else                        m_btnExecute.SetCheck( false );

		if (m_ulAcctl & MALWF_DELETE) m_btnDelete.SetCheck(true);
		else                       m_btnDelete.SetCheck( false );

		if (m_ulAcctl & MALWF_RENAME) m_btnRename.SetCheck(true);
		else                       m_btnRename.SetCheck( false );

	}

}



void CMalwFinderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}



void CMalwFinderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); 
		
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


HCURSOR CMalwFinderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



int CMalwFinderDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	DWORD dwStyle       = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
	DWORD dwListStyle   = dwStyle  | WS_BORDER | LVS_REPORT | LVS_SINGLESEL; // | LVS_SHOWSELALWAYS
	DWORD dwListExStyle = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES; // | LVS_EX_CHECKBOXES ;
	DWORD dwTreeStyle   = dwStyle | WS_BORDER | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_DISABLEDRAGDROP | TVS_NOTOOLTIPS;

	// 서비스이름
	if(!m_lblSvcName.Create( _T("서비스"), WS_CHILD | WS_VISIBLE | SS_LEFT, CRect(0,0,0,0), this, 0 ))  return -1;
	m_lblSvcName.SetFont( &g_FontDefault );
	if(!m_editSvcName.Create( dwStyle | ES_MULTILINE | ES_READONLY | WS_TABSTOP | WS_BORDER, CRect(0,0,0,0), this, IDC_EDIT_SVCNAME )) return -1;
	m_editSvcName.SetFont( &g_FontDefault );

	// 드라이버 이미지패스
	if(!m_lblDrvFullPath.Create( _T("ImagePath"), WS_CHILD | WS_VISIBLE | SS_LEFT, CRect(0,0,0,0), this, 0 ))  return -1;
	m_lblDrvFullPath.SetFont( &g_FontDefault );
	if(!m_editDrvFullPath.Create( dwStyle | ES_MULTILINE | ES_READONLY | WS_TABSTOP | WS_BORDER, CRect(0,0,0,0), this, IDC_EDIT_IMAGEPATH )) return -1;
	m_editDrvFullPath.SetFont( &g_FontDefault );

	//Install Button
	if(!m_btnDrvInstall.Create( MSG_DRV_INSTALL, dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_INSTALL_BTN ))  return -1;
	m_btnDrvInstall.SetFont( &g_FontDefault );
	//Uninstall Button
	if(!m_btnDrvUnInstall.Create( MSG_DRV_UNINSTALL, dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_UNINSTALL_BTN ))  return -1;
	m_btnDrvUnInstall.SetFont( &g_FontDefault );
	//Start Button
	if(!m_btnDrvStart.Create( MSG_DRV_START, dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_START_BTN ))  return -1;
	m_btnDrvStart.SetFont( &g_FontDefault );
	//Stop Button
	if(!m_btnDrvStop.Create( MSG_DRV_STOP, dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_STOP_BTN ))  return -1;
	m_btnDrvStop.SetFont( &g_FontDefault );

	//SetupDir
	if(!m_btnDrvSetupDir.Create( MSG_DRV_SETUPDIR, dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_SETUPDIR_BTN ))  return -1;
	m_btnDrvSetupDir.SetFont( &g_FontDefault );

	//SetupDir_Ctrl
	if(!m_btnDrvSetupDir_Ctrl.Create( MSG_DRV_SETUPDIR_CTRL, dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_SETUPDIR_CTRL_BTN ))  return -1;
	m_btnDrvSetupDir_Ctrl.SetFont( &g_FontDefault );
		
	if(!m_RetListCtrl.Create( dwListStyle, CRect(0,0,0,0), this, IDC_DRV_LISTCTRL )) return -1;
	m_RetListCtrl.SetExtendedStyle( m_RetListCtrl.GetExtendedStyle() | dwListExStyle );
	m_RetListCtrl.SetFont( &g_FontDefault );	
	Initial_ListCtrl( &m_RetListCtrl );


	if(!m_btnLogStart.Create(_T("로그취합 시작"), dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_LOG_START ))  return -1;
	m_btnLogStart.SetFont( &g_FontDefault );	

	if(!m_btnLogStop.Create(_T("로그취합 중지"), dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_LOG_STOP ))  return -1;
	m_btnLogStop.SetFont( &g_FontDefault );	

	if(!m_btnLog.Create(_T("로그"), dwStyle | BS_CHECKBOX | BS_AUTOCHECKBOX, CRect(0,0,0,0), this, IDC_DRV_CHECK_LOG ))  return -1;
	m_btnLog.SetFont( &g_FontDefault );		

	if(!m_btnAccess.Create(_T("허용"), dwStyle | BS_CHECKBOX | BS_AUTOCHECKBOX, CRect(0,0,0,0), this, IDC_DRV_CHECK_ACCESS ))  return -1;
	m_btnAccess.SetFont( &g_FontDefault );	

	if(!m_btnCreate.Create(_T("생성"), dwStyle | BS_CHECKBOX | BS_AUTOCHECKBOX, CRect(0,0,0,0), this, IDC_DRV_CHECK_CREATE ))  return -1;
	m_btnCreate.SetFont( &g_FontDefault );	

	if(!m_btnOpen.Create(_T("열기"), dwStyle | BS_CHECKBOX | BS_AUTOCHECKBOX, CRect(0,0,0,0), this, IDC_DRV_CHECK_OPEN ))  return -1;
	m_btnOpen.SetFont( &g_FontDefault );		

	if(!m_btnRead.Create(_T("읽기"), dwStyle | BS_CHECKBOX | BS_AUTOCHECKBOX, CRect(0,0,0,0), this, IDC_DRV_CHECK_READ ))  return -1;
	m_btnRead.SetFont( &g_FontDefault );	

	if(!m_btnWrite.Create(_T("쓰기"), dwStyle | BS_CHECKBOX | BS_AUTOCHECKBOX, CRect(0,0,0,0), this, IDC_DRV_CHECK_WRITE ))  return -1;
	m_btnWrite.SetFont( &g_FontDefault );	

	if(!m_btnExecute.Create(_T("실행"), dwStyle | BS_CHECKBOX | BS_AUTOCHECKBOX, CRect(0,0,0,0), this, IDC_DRV_CHECK_EXECUTE ))  return -1;
	m_btnExecute.SetFont( &g_FontDefault );	

	if(!m_btnDelete.Create(_T("삭제"), dwStyle | BS_CHECKBOX | BS_AUTOCHECKBOX, CRect(0,0,0,0), this, IDC_DRV_CHECK_DELETE ))  return -1;
	m_btnDelete.SetFont( &g_FontDefault );	

	if(!m_btnRename.Create(_T("리네임"), dwStyle | BS_CHECKBOX | BS_AUTOCHECKBOX, CRect(0,0,0,0), this, IDC_DRV_CHECK_RENAME ))  return -1;
	m_btnRename.SetFont( &g_FontDefault );	


	// Reg-Policy
	// File-Policy
	// Process-Policy
	// Network-Policy

	if (!m_btn_JobStart.Create(_T("분석시작"), dwStyle | BS_PUSHBUTTON, CRect(0, 0, 0, 0), this, IDC_POLICY_JOB_START ))  return -1;
	m_btn_JobStart.SetFont(&g_FontDefault);

	// 악성코드 프로세스
	if (!m_btn_PolicyProc.Create(_T("프로세스선택"), dwStyle | BS_PUSHBUTTON, CRect(0, 0, 0, 0), this, IDC_POLICY_SELECT_PROCESS ))  return -1;
	m_btn_PolicyProc.SetFont(&g_FontDefault);

	
	if (!m_edit_PolicyProc.Create(dwStyle | ES_MULTILINE | ES_READONLY | WS_TABSTOP | WS_BORDER, CRect(0, 0, 0, 0), this, IDC_EDIT_PROCPATH)) return -1;
	m_edit_PolicyProc.SetFont(&g_FontDefault);

	// PolicyReg
	if (!m_btn_PolicyReg.Create(_T("PolicyReg"), dwStyle | BS_PUSHBUTTON, CRect(0, 0, 0, 0), this, IDC_POLICY_SELECT_PROCESS))  return -1;
	m_btn_PolicyReg.SetFont(&g_FontDefault);
	// PolicyFile
	if (!m_btn_PolicyFile.Create(_T("PolicyFile"), dwStyle | BS_PUSHBUTTON, CRect(0, 0, 0, 0), this, IDC_DRV_F_SET_FDDVOL_CTRL))  return -1;
	m_btn_PolicyFile.SetFont(&g_FontDefault);
	// PolicyNetwork
	if (!m_btn_PolicyNetwork.Create(_T("PolicyNetwork"), dwStyle | BS_PUSHBUTTON, CRect(0, 0, 0, 0), this, IDC_DRV_F_SET_FDDVOL_CTRL))  return -1;
	m_btn_PolicyNetwork.SetFont(&g_FontDefault);


	if (!m_Lbl_MalText.Create(_T("악성코드 분석정책"), WS_CHILD | WS_VISIBLE | SS_LEFT, CRect(0, 0, 0, 0), this, 0))  return -1;
	m_Lbl_MalText.SetFont(&g_FontDefault);

	// 악성코드 분석정책 리스트
	if (!m_PolicyListCtrl.Create(dwListStyle, CRect(0, 0, 0, 0), this, IDC_POLICY_LISTCTRL)) return -1;
	m_PolicyListCtrl.SetExtendedStyle(m_RetListCtrl.GetExtendedStyle() | dwListExStyle);
	m_PolicyListCtrl.SetFont(&g_FontDefault);
	Initial_PolicyListCtrl(&m_PolicyListCtrl);

	
	// 확장자 제어설정 
	if(!m_btnDrv_F_SetExtCtrl_Black.Create( _T("확장자제어 설정"), dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_F_SET_EXT_CTRL ))  return -1;
	m_btnDrv_F_SetExtCtrl_Black.SetFont( &g_FontDefault );
	
	// 확장자 제어해제
	if(!m_btnDrv_F_ClrExtCtrl.Create(_T("확장자제어 해제"), dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_F_CLR_EXT_CTRL ))  return -1;
	m_btnDrv_F_ClrExtCtrl.SetFont( &g_FontDefault );	
	

		
	//프로세스 살고 죽는것 가지고 오기
	if(!m_btnDrv_P_ProcStat.Create(_T("프로세스통계"), dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_P_PROC_STAT ))  return -1;
	m_btnDrv_P_ProcStat.SetFont( &g_FontDefault );

	//Process 제어 설정
	if(!m_btnDrv_P_SetProcCtrl.Create(_T("프로세스제어설정"), dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_P_SET_PROC_CTRL ))  return -1;
	m_btnDrv_P_SetProcCtrl.SetFont( &g_FontDefault );
	//Process 제어 해제
	if(!m_btnDrv_P_ClrProcCtrl.Create(_T("프로세스제어해제"), dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_DRV_P_CLR_PROC_CTRL ))  return -1;
	m_btnDrv_P_ClrProcCtrl.SetFont( &g_FontDefault );
	


	if(!m_btnClear.Create( _T("클리어"), dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_CLEAR_BTN ))  return -1;
	m_btnClear.SetFont( &g_FontDefault );

	//SetupDir_Ctrl
	if(!m_btnOK.Create( MSG_OK, dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_OK_BTN ))  return -1;
	m_btnOK.SetFont( &g_FontDefault );

	//SetupDir
	if(!m_btnCancel.Create( MSG_CANCEL, dwStyle | BS_PUSHBUTTON, CRect(0,0,0,0), this, IDC_CANCEL_BTN ))  return -1;
	m_btnCancel.SetFont( &g_FontDefault );

	return 0;

}


#define COL_Number     _T("No")
#define COL_IoCtrlCode _T("IoCode") 
#define COL_Command    _T("DrvCmd")
#define COL_Message    _T("Message")

void CMalwFinderDlg::Initial_ListCtrl(CListCtrl* pListCtrl)
{	
	ASSERT( pListCtrl );
	if(!pListCtrl) return;

	static TCHAR* ppHeaders[] = 
	{ 
		COL_Command,
		COL_Message, 
		NULL 
	};
	
	LV_COLUMN  lvColumn;
	RtlZeroMemory( &lvColumn, sizeof(lvColumn) );
	for(int i=0;;i++)
	{
		if(ppHeaders[i] == NULL) break;
		lvColumn.mask   = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH ;
		lvColumn.fmt    = LVCFMT_LEFT;
		lvColumn.pszText = ppHeaders[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = 80;		

		if(i==0) lvColumn.cx = 120;
		else if(i==1) lvColumn.cx = 750;

		pListCtrl->InsertColumn( i, &lvColumn );			
	}

}


#define COL_POLICY    _T("Policy")
#define COL_CONTENTS  _T("Contents")

void CMalwFinderDlg::Initial_PolicyListCtrl(CListCtrl* pListCtrl)
{
	ASSERT(pListCtrl);
	if (!pListCtrl) return;

	static TCHAR* ppHeaders[] =
	{
		COL_POLICY,
		COL_CONTENTS,
		NULL
	};

	LV_COLUMN  lvColumn;
	RtlZeroMemory(&lvColumn, sizeof(lvColumn));
	for (int i = 0;; i++)
	{
		if (ppHeaders[i] == NULL) break;
		lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.pszText = ppHeaders[i];
		lvColumn.iSubItem = i;
		lvColumn.cx = 80;

		if (i == 0) lvColumn.cx = 120;
		else if (i == 1) lvColumn.cx = 750;

		pListCtrl->InsertColumn(i, &lvColumn);
	}

}



#define HEIGHT_GAP 10
#define WIDTH_GAP  15
#define EXPAND_GAP 300

void CMalwFinderDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	int nLblWidth = 70,  nLblHeight  = 20, nEditWidth = 300, nEditHeigth = 20, nCheckWidth = 50;
	int nLeft = 15, nTop = 15, nBtnWidth = 115, nBtnHeight  = 20;
	int nListLeft = 0,  nListTop = 15+nBtnHeight+HEIGHT_GAP, nListWidth = 0,   nListHeight = 0;

	if(GetSafeHwnd()) MoveWindow( 0, 0, 840, 950 );

	if(m_lblSvcName.GetSafeHwnd())  m_lblSvcName.MoveWindow(  nLeft, nTop+4, nLblWidth, nLblHeight );	
	nLeft += nLblWidth;
	if(m_editSvcName.GetSafeHwnd()) m_editSvcName.MoveWindow( nLeft, nTop, nEditWidth, nEditHeigth );	

	nLeft = 15;
	nTop += nBtnHeight + HEIGHT_GAP;
	if(m_lblDrvFullPath.GetSafeHwnd()) m_lblDrvFullPath.MoveWindow(   nLeft, nTop+4, nLblWidth, nLblHeight );	
	nLeft += nLblWidth;
	if(m_editDrvFullPath.GetSafeHwnd()) m_editDrvFullPath.MoveWindow( nLeft, nTop, nEditWidth+EXPAND_GAP, nEditHeigth );	

	nLeft = 15;
	nTop += nBtnHeight + HEIGHT_GAP;
	if(m_btnDrvInstall.GetSafeHwnd()) m_btnDrvInstall.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );	
	nLeft += nBtnWidth + WIDTH_GAP;
	if(m_btnDrvUnInstall.GetSafeHwnd()) m_btnDrvUnInstall.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );
	nLeft += nBtnWidth + WIDTH_GAP;
	if(m_btnDrvStart.GetSafeHwnd()) m_btnDrvStart.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );	
	nLeft += nBtnWidth + WIDTH_GAP;
	if(m_btnDrvStop.GetSafeHwnd()) m_btnDrvStop.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );
	
	// SetupDir
	nLeft  = 15;
	nTop  += nBtnHeight + HEIGHT_GAP;
	if(m_btnDrvSetupDir.GetSafeHwnd()) m_btnDrvSetupDir.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );
	nLeft += nBtnWidth + WIDTH_GAP;
	if(m_btnDrvSetupDir_Ctrl.GetSafeHwnd()) m_btnDrvSetupDir_Ctrl.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );	

	nLeft += nBtnWidth + WIDTH_GAP;
	if(m_btnLogStart.GetSafeHwnd()) m_btnLogStart.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );	
	nLeft += nBtnWidth + WIDTH_GAP;
	if(m_btnLogStop.GetSafeHwnd()) m_btnLogStop.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );	

	nLeft  = 20;
	nTop  += nBtnHeight + HEIGHT_GAP;
	// Log
	if(m_btnLog.GetSafeHwnd()) m_btnLog.MoveWindow( nLeft, nTop, nCheckWidth, nBtnHeight );	
	// FLT_ACCESS
	nLeft += nCheckWidth+WIDTH_GAP;
	if(m_btnAccess.GetSafeHwnd()) m_btnAccess.MoveWindow( nLeft, nTop, nCheckWidth, nBtnHeight );	
	// FLT_CREATE
	nLeft += nCheckWidth+WIDTH_GAP;
	if(m_btnCreate.GetSafeHwnd()) m_btnCreate.MoveWindow( nLeft, nTop, nCheckWidth, nBtnHeight );	
	// FLT_OPEN
	nLeft += nCheckWidth+WIDTH_GAP;
	if(m_btnOpen.GetSafeHwnd())   m_btnOpen.MoveWindow( nLeft, nTop, nCheckWidth, nBtnHeight );	
	// FLT_READ
	nLeft += nCheckWidth+WIDTH_GAP;
	if(m_btnRead.GetSafeHwnd())    m_btnRead.MoveWindow( nLeft, nTop, nCheckWidth, nBtnHeight );	
	// FLT_WRITE
	nLeft += nCheckWidth+WIDTH_GAP;
	if(m_btnWrite.GetSafeHwnd())   m_btnWrite.MoveWindow( nLeft, nTop, nCheckWidth, nBtnHeight );	
	// FLT_EXECUTE
	nLeft += nCheckWidth+WIDTH_GAP;
	if(m_btnExecute.GetSafeHwnd()) m_btnExecute.MoveWindow( nLeft, nTop, nCheckWidth, nBtnHeight );	
	// FLT_DELETE
	nLeft += nCheckWidth+WIDTH_GAP;
	if(m_btnDelete.GetSafeHwnd())  m_btnDelete.MoveWindow( nLeft, nTop, nCheckWidth, nBtnHeight );	
	// FLT_RENAME
	nLeft += nCheckWidth+WIDTH_GAP;
	if(m_btnRename.GetSafeHwnd())  m_btnRename.MoveWindow( nLeft, nTop, nCheckWidth+10, nBtnHeight );	


	nLeft = 15;
	nTop += nBtnHeight + HEIGHT_GAP + 30;
	// 악성코드 프로세스 선택
	if (m_btn_PolicyProc.GetSafeHwnd()) m_btn_PolicyProc.MoveWindow(nLeft, nTop, nBtnWidth, nBtnHeight);

	nLeft += nBtnWidth + WIDTH_GAP;
	if (m_edit_PolicyProc.GetSafeHwnd()) m_edit_PolicyProc.MoveWindow(nLeft, nTop, nEditWidth + EXPAND_GAP, nEditHeigth);
	

	nLeft = 15;
	nTop += nBtnHeight + HEIGHT_GAP;
	// 레지스트리 정책
	if (m_btn_PolicyReg.GetSafeHwnd()) m_btn_PolicyReg.MoveWindow(nLeft, nTop, nBtnWidth, nBtnHeight);
	// 파일정책
	nLeft += nBtnWidth + WIDTH_GAP;
	if (m_btn_PolicyFile.GetSafeHwnd()) m_btn_PolicyFile.MoveWindow(nLeft, nTop, nBtnWidth, nBtnHeight);
	// 네트워크 정책
	nLeft += nBtnWidth + WIDTH_GAP;
	if (m_btn_PolicyNetwork.GetSafeHwnd()) m_btn_PolicyNetwork.MoveWindow(nLeft, nTop, nBtnWidth, nBtnHeight);
	
	// 분석시작
	nLeft += nBtnWidth + WIDTH_GAP;
	if (m_btn_JobStart.GetSafeHwnd()) m_btn_JobStart.MoveWindow(nLeft, nTop, nBtnWidth, nBtnHeight);

	// 악성코드 분석정책
	nLeft = 15;
	nTop += nBtnHeight + HEIGHT_GAP;
	if (m_Lbl_MalText.GetSafeHwnd()) m_Lbl_MalText.MoveWindow(nLeft, nTop, nBtnWidth, nBtnHeight);

	nLeft = 15;
	nTop += nBtnHeight;
	if (m_PolicyListCtrl.GetSafeHwnd())
	{
		nListWidth = 800;	nListHeight = 250;
		m_PolicyListCtrl.MoveWindow(nLeft, nTop, nListWidth, nListHeight);
	}

	
	// 확장자 제어설정, 확장자 제어해제
	nLeft = 15;
	nTop += nBtnHeight + HEIGHT_GAP + nListHeight;
	
	//if(m_btnDrv_F_SetExtCtrl_Black.GetSafeHwnd()) m_btnDrv_F_SetExtCtrl_Black.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );		
	// nLeft += nBtnWidth + WIDTH_GAP;
	// if(m_btnDrv_F_ClrExtCtrl.GetSafeHwnd()) m_btnDrv_F_ClrExtCtrl.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );		
			
	//프로세스 통계
	nLeft = 15;
    // nTop += nBtnHeight + HEIGHT_GAP;
	// nLeft += nBtnWidth + WIDTH_GAP;
	if(m_btnDrv_P_ProcStat.GetSafeHwnd()) m_btnDrv_P_ProcStat.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );		
	/*
	// Process 제어설정/해제
	nLeft += nBtnWidth + WIDTH_GAP;
	if(m_btnDrv_P_SetProcCtrl.GetSafeHwnd()) m_btnDrv_P_SetProcCtrl.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );
	nLeft += nBtnWidth + WIDTH_GAP;
	if(m_btnDrv_P_ClrProcCtrl.GetSafeHwnd()) m_btnDrv_P_ClrProcCtrl.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );
	*/
	
	nLeft = 15;
	nTop += nBtnHeight + HEIGHT_GAP;
	if(m_RetListCtrl.GetSafeHwnd())
	{
		nListWidth  = 800;	nListHeight = 250;
		m_RetListCtrl.MoveWindow( nLeft, nTop, nListWidth, nListHeight );
	}
	
	nLeft = (nListWidth/2)-(nBtnWidth);
	nTop += nListHeight + HEIGHT_GAP;
	if(m_btnClear.GetSafeHwnd()) m_btnClear.MoveWindow( nLeft-200, nTop, nBtnWidth, nBtnHeight );
	if(m_btnOK.GetSafeHwnd()) m_btnOK.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );
	nLeft += nBtnWidth + WIDTH_GAP + 10;
	if(m_btnCancel.GetSafeHwnd()) m_btnCancel.MoveWindow( nLeft, nTop, nBtnWidth, nBtnHeight );	

}


void CMalwFinderDlg::OnBtnClk_LogStart(void)
{
	int       nItem=0;
	CStringA  strText;

	// 로그 취합시작
	g_MalwFMgr.Driver_LogStart( LOG_ON );

	strText.Format( "LogStart=LOG_ON" );
	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "LogStart" );
	m_RetListCtrl.AddItem( nItem, 1, strText );

	CreateThread( NULL, 0, CMalwFManager::ThreadProc_LogFetch, this, 0, NULL );

	strText.Format( "ThreadProc_LogFetch Thread Start" );
	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "ThreadProc_LogFetch" );
	m_RetListCtrl.AddItem( nItem, 1, strText );
}



void CMalwFinderDlg::OnBtnClk_LogStop(void)
{
	int       nItem=0;
	CStringA  strText;

	g_MalwFMgr.Driver_LogStart( LOG_OFF );

	strText.Format( "LogStop=LOG_OFF" );
	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "LogStop" );
	m_RetListCtrl.AddItem( nItem, 1, strText );

}


void CMalwFinderDlg::OnBtnClk_CheckLog(void)
{
	m_bLog = m_btnLog.GetCheck();
}


void CMalwFinderDlg::OnBtnClk_CheckAccess(void)
{
	ULONG ulFlag = 0x00;
	BOOL  bCheck = m_btnAccess.GetCheck();

	if(bCheck)
	{
		m_ulAcctl &= MALWF_ACCESS;
	}
	else
	{
		ulFlag = ~(MALWF_CREATE);
		m_ulAcctl &= ulFlag;
	}

	UpdateCheckBoxState();
}

void CMalwFinderDlg::OnBtnClk_CheckCreate(void)
{
	ULONG ulFlag = 0x00;
	BOOL  bCheck = m_btnCreate.GetCheck();

	if(bCheck)
	{
		m_ulAcctl |= MALWF_CREATE;
	}
	else
	{
		ulFlag = ~(MALWF_CREATE);
		m_ulAcctl &= ulFlag;
	}

	UpdateCheckBoxState();

}


void CMalwFinderDlg::OnBtnClk_CheckOpen(void)
{
	ULONG ulFlag = 0x00;
	BOOL  bCheck = m_btnOpen.GetCheck();

	if(bCheck)
	{
		m_ulAcctl |= MALWF_OPEN;
	}
	else
	{
		ulFlag = ~(MALWF_OPEN);
		m_ulAcctl &= ulFlag;
	}

	UpdateCheckBoxState();
}


void CMalwFinderDlg::OnBtnClk_CheckRead(void)
{
	ULONG ulFlag = 0x00;
	BOOL  bCheck = m_btnRead.GetCheck();

	if(bCheck)
	{
		m_ulAcctl |= MALWF_READ;
	}
	else
	{
		ulFlag = ~(MALWF_READ);
		m_ulAcctl &= ulFlag;
	}

	UpdateCheckBoxState();
}


void CMalwFinderDlg::OnBtnClk_CheckWrite(void)
{
	
	ULONG ulFlag = 0x00;
	BOOL  bCheck = m_btnWrite.GetCheck();

	if(bCheck)
	{
		m_ulAcctl |= MALWF_WRITE;
	}
	else
	{
		ulFlag = ~(MALWF_WRITE);
		m_ulAcctl &= ulFlag;
	}

	UpdateCheckBoxState();
}

void CMalwFinderDlg::OnBtnClk_CheckExecute(void)
{
	ULONG ulFlag = 0x00;
	BOOL  bCheck = m_btnExecute.GetCheck();

	if(bCheck)
	{
		m_ulAcctl |= MALWF_EXECUTE;
	}
	else
	{
		ulFlag = ~(MALWF_EXECUTE);
		m_ulAcctl &= ulFlag;
	}
	UpdateCheckBoxState();

}

void CMalwFinderDlg::OnBtnClk_CheckDelete(void)
{

	ULONG ulFlag = 0x00;
	BOOL  bCheck = m_btnDelete.GetCheck();

	if(bCheck)
	{
		m_ulAcctl |= MALWF_DELETE;
	}
	else
	{
		ulFlag = ~(MALWF_DELETE);
		m_ulAcctl &= ulFlag;
	}

	UpdateCheckBoxState();

}

void CMalwFinderDlg::OnBtnClk_CheckRename(void)
{

	ULONG ulFlag = 0x00;
	BOOL  bCheck = m_btnRename.GetCheck();

	if(bCheck)
	{
		m_ulAcctl |= MALWF_RENAME;
	}
	else
	{
		ulFlag = ~(MALWF_RENAME);
		m_ulAcctl &= ulFlag;
	}

	UpdateCheckBoxState();
}




// 드라이버 설치
void CMalwFinderDlg::OnBtnClk_DriverInstall()
{
	int       nItem=0;
	CStringA  strmsg;

	g_MalwFMgr.Driver_Install();

	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_Install" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );
}


// 드라이버 삭제
void CMalwFinderDlg::OnBtnClk_DriverUninstall()
{
	int       nItem=0;
	CStringA  strmsg;

	g_MalwFMgr.Driver_UnInstall();

	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_UnInstall" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}

// 드라이버 시작
void CMalwFinderDlg::OnBtnClk_DriverStart()
{
	int       nItem=0;
	CStringA  strmsg;

	if(!g_MalwFMgr.Driver_Start())
	{
		AfxMessageBox( _T("Driver_Start 실패") );
	}
	else
	{
		AfxMessageBox( _T("Driver_Start 성공") );
	}

	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_Start" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}




// 드라이버 중지
void CMalwFinderDlg::OnBtnClk_DriverStop()         
{
	int       nItem=0;
	CStringA  strmsg;

	g_MalwFMgr.Driver_Stop();

	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_Stop" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}


// 전역제어 플래그
void CMalwFinderDlg::OnBtnClk_DriverSetGlobalCtrl()
{
	int       nItem=0;
	CStringA  strmsg;

	g_GlobalCtrl = !g_GlobalCtrl;
	g_MalwFMgr.Driver_SetGlobalCtrl( g_GlobalCtrl );

	strmsg.Format( "g_GlobalCtrl=%d", g_GlobalCtrl );
	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_SetGlobalControl" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}

// 설치폴더 설정
void CMalwFinderDlg::OnBtnClk_DriverSetupDir()     
{
	int       nItem=0;
	CStringA  strmsg;

	char czSetupDir[MAX_PATH], czSysDir[MAX_PATH];
	RtlZeroMemory( czSetupDir, MAX_PATH );
	RtlZeroMemory( czSysDir,   MAX_PATH );
	g_MalwFMgr.GetHomeDirectory( czSetupDir, MAX_PATH );
	GetWindowsDirectoryA( czSysDir, MAX_PATH );	
	
	g_MalwFMgr.Driver_SetupDir( czSetupDir, czSysDir );

	strmsg.Format( "SetupDir=%s | SysDir=%s", czSetupDir, czSysDir );
	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_SetupDir" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}



// 프로세스 선택
void CMalwFinderDlg::OnBtnClk_SelectProcess(void)
{
	CString strFullPath, strFileName;
	TCHAR szFilter[] = _T("실행파일 (*.exe, *.dll, *.sys) | *.exe;*.dll;*.sys | All Files(*.*)|*.*||");
	CFileDialog dlg( TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if (IDOK == dlg.DoModal())
	{
		strFullPath = dlg.GetPathName();
		strFileName = dlg.GetFileName();
		m_edit_PolicyProc.SetWindowText(_T(""));
		m_edit_PolicyProc.SetWindowText(strFullPath);

		StringCchCopy(m_szProcName, MAX_PATH, strFileName.GetBuffer());
		StringCchCopy(m_szProcName, MAX_PATH, strFileName.GetBuffer());
	}
}

// 레지스트리
// 파일
// 네트워크
// 분석시작
void CMalwFinderDlg::OnBtnClk_PolicyReg(void)
{
}

void CMalwFinderDlg::OnBtnClk_PolicyFile(void)
{
}

void CMalwFinderDlg::OnBtnClk_PolicyNetwork(void)
{
}

void CMalwFinderDlg::OnBtnClk_JobStart(void)
{
}



/*
#define FLT_ACCESS       0x00000000
#define FLT_CREATE       (2  <<  1)
#define FLT_OPEN         (2  <<  2)
#define FLT_READ         (2  <<  3)
#define FLT_WRITE        (2  <<  4)
#define FLT_EXECUTE      (2  <<  5)
#define FLT_DELETE       (2  <<  6)
#define FLT_RENAME       (2  <<  7)
#define FLT_REPLACE      (2  <<  8)
#define FLT_REPARSE      (2  <<  9) 
#define FLT_SET_SECURITY (2  << 10) 
#define FLT_SET_LINK     (2  << 11) 
#define FLT_MAX          0xFFFFFFFF
*/

// FDD제어 설정
void CMalwFinderDlg::OnBtnClk_DriverSetFDDVolCtrl(void)
{
	int       nItem=0;
	CStringA  strmsg;
	RtlZeroMemory( m_Policy_Fdd.czDrives, MAX_PATH );
	m_Policy_Fdd.ulDrives = g_MalwFMgr.GetFdd_Drives( m_Policy_Fdd.czDrives, MAX_PATH );
	m_Policy_Fdd.bLog     = TRUE;
	m_Policy_Fdd.ulAcctl  = m_ulAcctl;
	
	g_MalwFMgr.DriverSetFDDVolCtrl( m_Policy_Fdd.ulDrives, m_Policy_Fdd.ulAcctl, m_Policy_Fdd.bLog );
	strmsg.Format( "Log=%d/Acctl=%08x/Fdd=%s", m_Policy_Fdd.bLog, m_Policy_Fdd.ulAcctl, m_Policy_Fdd.czDrives );
	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_F_SetFDDCtrl" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}


// 이동식저장장치 설정
void CMalwFinderDlg::OnBtnClk_DriverSetUSBVolCtrl(void)
{
	int       nItem=0;
	CStringA  strmsg;

	RtlZeroMemory( m_Policy_Usb.czDrives, MAX_PATH );
	m_Policy_Usb.ulDrives = g_MalwFMgr.GetUsb_Drives( m_Policy_Usb.czDrives, MAX_PATH );	
	m_Policy_Usb.bLog     = TRUE;
	m_Policy_Usb.ulAcctl  = m_ulAcctl;
	
	g_MalwFMgr.DriverSetUSBVolCtrl( m_Policy_Usb.ulDrives, m_Policy_Usb.ulAcctl, m_Policy_Usb.bLog );
	strmsg.Format( "Log=%d/Acctl=%08x/Usb=%s", m_Policy_Usb.bLog, m_Policy_Usb.ulAcctl, m_Policy_Usb.czDrives );
	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_F_SetUSBStorCtrl" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}


// 확장자 제어설정
void CMalwFinderDlg::OnBtnClk_DriverSetExtCtrl(void)
{	
	int       nItem=0;
	CStringA  strmsg;
	CString   strFileExt;
	char      czFileExt[ MAX_PATH ];

	RtlZeroMemory( m_Policy_Fixed.czDrives, MAX_PATH );
	m_Policy_Fixed.ulDrives = g_MalwFMgr.GetFixed_Drives( m_Policy_Fixed.czDrives, MAX_PATH );
	m_Policy_Fixed.bLog     = TRUE;
	m_Policy_Fixed.ulAcctl  = m_ulAcctl;
		
	// m_edit_F_FileExtCtrl.GetWindowText( strFileExt );
	if(strFileExt.IsEmpty())
	{
		AfxMessageBox( _T("확장자를 입력하여 주세요") );
		//m_edit_F_FileExtCtrl.SetWindowText( _T("") );
		//m_edit_F_FileExtCtrl.SetSel( -1, 0 );
		//m_edit_F_FileExtCtrl.SetFocus();
		return; 
	}

	RtlZeroMemory( czFileExt,  MAX_PATH );
#ifdef _UNICODE 
	WideCharToMultiByte( CP_ACP, 0, strFileExt.GetBuffer(), -1, czFileExt, MAX_PATH, NULL, NULL );
#else
	StringCchCopyA( czFileExt, MAX_PATH, strFileExt.GetBuffer() );
#endif
	strFileExt.ReleaseBuffer();

	//
	// TRUE: 확장자제어 차단리스트 FALSE: 확장자제어 차단 예외리스트
	// 
	g_MalwFMgr.DriverSetFileExtCtrl( TRUE, m_Policy_Fixed.ulDrives, m_Policy_Fixed.ulAcctl, m_Policy_Fixed.bLog, czFileExt );	


	strmsg.Format( "TRUE, Log=%d/Acctl=%08x/Fixed=%s/FileExt=%s", m_Policy_Fixed.bLog, m_Policy_Fixed.ulAcctl, m_Policy_Fixed.czDrives, czFileExt );
	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "OnBtnClk_DriverSetExtCtrl" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}





// 확장자 제어해제
void CMalwFinderDlg::OnBtnClk_DriverClrExtCtrl(void)
{
	int       nItem=0;
	CStringA  strmsg;

	g_MalwFMgr.DriverClrFileExtCtrl();

	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_F_ClrFileExtCtrl" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}





// 공유폴더 제어설정
void CMalwFinderDlg::OnBtnClk_DriverSetSFolderCtrl(void)
{
	int       nItem=0;
	CStringA  strmsg;
		
	RtlZeroMemory( m_Policy_SFolder.czDrives, MAX_PATH );
	m_Policy_SFolder.ulDrives = g_MalwFMgr.GetNetwork_Drives( m_Policy_SFolder.czDrives, MAX_PATH );
	m_Policy_SFolder.bLog     = m_bLog;
	m_Policy_SFolder.ulAcctl  = m_ulAcctl;


	g_MalwFMgr.DriverSetSFolderCtrl( m_Policy_SFolder.ulDrives, m_Policy_SFolder.ulAcctl,  m_Policy_SFolder.bLog );
	strmsg.Format( "Log=%d/Acctl=%08x/Drv=%s", m_Policy_SFolder.bLog, m_Policy_SFolder.ulAcctl, m_Policy_SFolder.czDrives );
	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_SF_SetSFolderCtrl" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}


// 공유폴더 제어해제
void CMalwFinderDlg::OnBtnClk_DriverClrSFolderCtrl(void)
{
	int       nItem=0;
	CStringA  strmsg;

	g_MalwFMgr.DriverClrSFolderCtrl();

	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_SF_ClrSFolderCtrl" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}


// 공유폴더 예외폴더 설정
void CMalwFinderDlg::OnBtnClk_DriverSetExcept_SFDir(void)
{
	int       nItem=0;
	CStringA  strmsg;

	char czExceptDir[ MAX_PATH ];
	RtlZeroMemory( czExceptDir, MAX_PATH );

	g_MalwFMgr.DriverSetExcept_SFDir( czExceptDir );
	strmsg.Format( "ExceptDir=%s", czExceptDir );

	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "DriverSetExcept_SFDir" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}


// 공유폴더 예외폴더 해제
void CMalwFinderDlg::OnBtnClk_DriverClrExcept_SFDir(void)
{
	int       nItem=0;
	CStringA  strmsg;

	g_MalwFMgr.DriverClrExcept_SFDir();

	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_SF_ClrExceptDir" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}


// 공유폴더 예외프로세스 설정
void CMalwFinderDlg::OnBtnClk_DriverSetExcept_SFProc(void)
{
	int       nItem=0;
	CStringA  strmsg;
	char      czExceptProc[ MAX_PATH ];

	RtlZeroMemory( czExceptProc, MAX_PATH );

	g_MalwFMgr.DriverSetExcept_SFProc( czExceptProc );

	strmsg.Format( "ExceptProc=%s", czExceptProc );

	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_SF_SetExceptProc" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );


}

// 공유폴더 예외프로세스 해제
void CMalwFinderDlg::OnBtnClk_DriverClrExcept_SFProc(void)
{
	int       nItem=0;
	CStringA  strmsg;

	g_MalwFMgr.DriverClrExcept_SFProc();

	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_SF_ClrExceptProc" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );
}


#define MAX_STAT_BUFFER  (64*4096)

// 프로세스 살고죽고 통계 얻기
void CMalwFinderDlg::OnBtnClk_DriverGetProcStat(void)
{
	int       nItem=0;
	CStringA  strmsg;

	char czOutBuffer[ MAX_STAT_BUFFER ];
	RtlZeroMemory( czOutBuffer, MAX_STAT_BUFFER );
	g_MalwFMgr.DriverGetProcStat( czOutBuffer, MAX_STAT_BUFFER );

	strmsg.Format( "OutBuffer=%s", czOutBuffer );
	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "Driver_P_GetProcessStat" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}




// 프로세스제어 설정 
void CMalwFinderDlg::OnBtnClk_DriverSetProcCtrl(void)
{
	int       nItem=0;
	CStringA  strmsg;
	CString   strProcCtrl;

	char  czProcCtrl[MAX_PATH];
	RtlZeroMemory( czProcCtrl, MAX_PATH );

	//m_edit_P_ProcCtrl.GetWindowText( strProcCtrl );
	if(strProcCtrl.IsEmpty())
	{
		AfxMessageBox( _T("프로세스 이름을 입력하여 주세요") );
		//m_edit_P_ProcCtrl.SetWindowText( _T("") );
		//m_edit_P_ProcCtrl.SetSel( -1, 0 );
		//m_edit_P_ProcCtrl.SetFocus();
		return; 
	}

	RtlZeroMemory( czProcCtrl,  MAX_PATH );
#ifdef _UNICODE 
	WideCharToMultiByte( CP_ACP, 0, strProcCtrl.GetBuffer(), -1, czProcCtrl, MAX_PATH, NULL, NULL );
#else
	StringCchCopyA( czProcCtrl, MAX_PATH, strProcCtrl.GetBuffer() );
#endif
	strProcCtrl.ReleaseBuffer();


	RtlZeroMemory( m_Policy_Proc.czDrives, MAX_PATH );
	m_Policy_Proc.ulDrives = g_MalwFMgr.GetFixed_Drives( m_Policy_Proc.czDrives, MAX_PATH );
	m_Policy_Proc.bLog     = m_bLog;
	m_Policy_Proc.ulAcctl  = m_ulAcctl;

	g_MalwFMgr.DriverSetProcCtrl( TRUE, m_Policy_Proc.ulDrives, m_Policy_Proc.ulAcctl, m_Policy_Proc.bLog, czProcCtrl );

	strmsg.Format( "Log=%d/Acctl=0x%08x/Drv=%s/Process=%s", m_Policy_Proc.bLog, m_Policy_Proc.ulAcctl, m_Policy_Proc.czDrives, czProcCtrl );
	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "DriverSetProcCtrl" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}

// 프로세스제어 해제
void CMalwFinderDlg::OnBtnClk_DriverClrProcCtrl(void)
{
	int       nItem=0;
	CStringA  strmsg;

	g_MalwFMgr.DriverClrProcCtrl();

	nItem = m_RetListCtrl.GetItemCount();
	m_RetListCtrl.AddItem( nItem, 0, "DriverClrProcCtrl" );
	m_RetListCtrl.AddItem( nItem, 1, strmsg );

}


void CMalwFinderDlg::OnBtnClk_Clear()
{
	m_RetListCtrl.DeleteAllItems();
}

void CMalwFinderDlg::OnBtnClk_Ok()
{
	CDialog::OnOK();
}


void CMalwFinderDlg::OnBtnClk_Cancel()
{
	CDialog::OnCancel();
}
