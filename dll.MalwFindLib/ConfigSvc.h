#pragma once
#include <WinSvc.h>



namespace APP
{

	class CConfigSvc 
	{
	public:
		CConfigSvc(void);
		virtual ~CConfigSvc(void);

	public:
		// /*************************************************************************************************************/
		virtual void   SetSvcStatus(DWORD dwState, DWORD dwAccept) = 0;
		virtual ULONG  RunService(void)  = 0;
		virtual void   StopService(void) = 0;
		virtual BOOL   ExecuteServiceFunc(void) =0;

		// /*************************************************************************************************************/
		BOOL  ExistService(TCHAR*  pszServiceNameName);
		// /*************************************************************************************************************/
		BOOL  InstallService(void);
		BOOL  DeleteService(void);

		// /*************************************************************************************************************/

	public:
		// /*************************************************************************************************************/
		//  ServiceName  Description Name
		TCHAR*   GetServiceName(void);
		void     SetServiceName(const TCHAR* pszServiceName, ULONG ulServiceNameLen );

		// ServiceImagepath  
		TCHAR*   GetServiceImagePath(void);
		void     SetServiceImagePath(const TCHAR* pszServiceImagePath, ULONG  ulServiceImagePathLen );
		
		BOOL     SafeModeInstallService(const TCHAR* pServiceName, ULONG ulServiceNameLen );

		// /*************************************************************************************************************/
		// /*************************************************************************************************************/
		// ServiceType
		ULONG    GetServiceType(void);
		void     SetServiceType( ULONG ulServiceType );
		// /*************************************************************************************************************/
		// ServiceType SERVICE_FILE_SYSTEM_DRIVER
		void     SetServiceType_SystemDriver(void);
		// ServiceType  SERVICE_KERNEL_DRIVER
		void     SetServiceType_KernelDriver(void);
		// ServiceType  SERVICE_WIN32_OWN_PROCESS
		void	 SetServiceType_Win32OwnProcess(void);
		// ServiceType  SERVICE_WIN32_SHARE_PROCESS
		void     SetServiceType_Win32ShareProcess(void);
		// /*************************************************************************************************************/
		// /*************************************************************************************************************/
		// StartType 
		ULONG    GetStartType(void);
		void     SetStartType( ULONG ulStartType );
		// /*************************************************************************************************************/
		// SERVICE_BOOT_START     0x00000000
		void     SetStartType_BootStart(void);
		// SERVICE_SYSTEM_START   0x00000001
		void     SetStartType_SystemStart(void);
		// SERVICE_AUTO_START     0x00000002
		void     SetStartType_AutoStart(void);
		// SERVICE_DEMAND_START   0x00000003
		void     SetStartType_DemandStart(void);
		// /*************************************************************************************************************/
		// /*************************************************************************************************************/
	private:
		ULONG   m_ulStartType;
		ULONG   m_ulServiceType;
	protected:
		TCHAR*  m_pServiceName;
		TCHAR*  m_pServiceImagePath;

	};


};