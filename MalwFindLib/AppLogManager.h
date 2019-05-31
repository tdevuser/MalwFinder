#pragma once



namespace APP
{

	class CAppLogManager
	{
	public:
		CAppLogManager(void);
		virtual ~CAppLogManager(void);

	private:
		void InitInstance();
		void ExitInstance();

		int  GetWindowsVersion();
		void GetLocalAppDataPath( char* pczOutPath, int nMaxLength );

	public:
		void Write(const TCHAR* fmt, ...);

	private:
		bool   m_bLog;
		TCHAR* m_pszFileName;

	};

	extern CAppLogManager g_LogManager;

};