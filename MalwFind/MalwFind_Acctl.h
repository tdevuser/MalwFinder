
#ifndef _MALWFIND_ACCTL_H_
#define _MALWFIND_ACCTL_H_

#include "ntifs.h"
#include "MalwFind_DataTypes.h"
#include "MalwFind_Command.h"

#ifdef __cplusplus
extern "C"
{
#endif

/********************************************************************************************************************/


// 전역으로 예외처리 되는프로세스 	
BOOLEAN  
ISExpProcList_PolicyAll( IN PWCHAR pProcessName );

// 자체보호 스킵 예외프로세스
BOOLEAN  
ISExpProcList_PolicySelfProtect( IN PWCHAR pProcessName );

ULONG  
GetVolumeFlag( WCHAR cVol );

NTSTATUS Control_FP_SFolder(IN PWCHAR         pwzProcName, 
							IN PNAME_BUFFER   pFileName, 
							IN PULONG         pulDisposition,
							IN PACCESS_MASK   pulAccessMask,
							IN PLOG_HDR       pLogHdr  );

NTSTATUS  Control_FP_Volume(IN PWCHAR         pwzProcName, 
						   IN PNAME_BUFFER   pFileName, 
						   IN PULONG         pulDisposition,
						   IN PACCESS_MASK   pulAccessMask,
						   IN PLOG_HDR       pLogHdr  );
						   
/********************************************************************************************************************/

NTSTATUS  
Control_SF_SFolder( IN PWCHAR        pwzProcName, 
                    IN PNAME_BUFFER  pFileName,
				    IN ULONG         ulAccess,
					IN PLOG_HDR      pLogHdr );
				    
NTSTATUS  
Control_SF_FileExt( IN PWCHAR        pwzProcName,
                    IN PNAME_BUFFER  pFileName,			
			        IN ULONG         nDisposition, // CreateDisposition
			        IN ACCESS_MASK   ulAccess,
					IN PLOG_HDR      pLogHdr  );				    

/********************************************************************************************************************/
NTSTATUS   
Control_F_FileName( IN PWCHAR        pwzProcName,
					IN PNAME_BUFFER  pFileName,		
				    IN ULONG         nDisposition, // CreateDisposition
				    IN ACCESS_MASK   ulAccess,
					IN PLOG_HDR      pLogHdr );

NTSTATUS 
Control_F_FileExt(  IN PWCHAR        pwzProcName,
					IN PNAME_BUFFER  pFileName,		                    
				    IN ULONG         nDisposition, // CreateDisposition
				    IN ACCESS_MASK   ulAccess,
					IN PLOG_HDR      pLogHdr  );
				    
NTSTATUS 
Control_F_FDD( IN PWCHAR       pwzProcName, 
               IN PNAME_BUFFER pFileName, 
			   IN ULONG        ulAccess,
			   IN PLOG_HDR     pLogHdr );

NTSTATUS 
Control_F_Usb( IN PWCHAR        pwzProcName, 
               IN PNAME_BUFFER  pFileName, 
			   IN ULONG         ulAccess,
			   IN PLOG_HDR      pLogHdr );

/********************************************************************************************************************/

NTSTATUS 
Control_P_Process( IN PWCHAR        pwzProcName, 
				   IN PNAME_BUFFER  pFileName, 
				   IN ULONG         ulAccess,
				   IN PLOG_HDR      pLogHdr  );

/********************************************************************************************************************/


/******************************************************************************************************/
// PROCESS 제어 검색
/******************************************************************************************************/

// 프로세스 제어리스트 판별
BOOLEAN  ISExist_P_ProcCtrl( IN PWCHAR pwzProcName );

// 프로세스 예외폴더 판정
BOOLEAN  ISExist_P_ExceptDir( IN PWCHAR pwzExceptPath );

// 프로세스 예외폴더 판정( 동일폴더 판정
BOOLEAN  ISExist_P_ExceptDir_Equal( IN PWCHAR pwzExceptPath );

/******************************************************************************************************/
// 파일확장자 제어검색
/******************************************************************************************************/

// 확장자 제어 리스트 검색
BOOLEAN ISExist_F_FileExtCtrl( IN PWCHAR pwzFileName );

// 확장자 제어 예외 리스트 검색
BOOLEAN ISExist_F_FileExtCtrl_Except( IN PWCHAR pwzFileName );

// 확장자 예외폴더 검색
BOOLEAN ISExist_F_ExceptDir( IN PWCHAR pwzExceptPath );
// 확장자 예외폴더 동일이름 검색
BOOLEAN ISExist_F_ExceptDir_Equal( IN PWCHAR pwzExceptPath );

// 확장자 예외 프로세스 검색
BOOLEAN ISExist_F_ExceptProc( IN PWCHAR pwzExceptProc );

/******************************************************************************************************/
// 공유폴더 제어검색
/******************************************************************************************************/

// 공유폴더 제어리스트 검색
// BOOLEAN  ISExist_SF_SFolderCtrl( IN PWCHAR pwzFilePath );

// 공유폴더제어 예외폴더 검색
BOOLEAN  ISExist_SF_SFolderExceptDir( IN PWCHAR pwzExceptPath );

// 공유폴더제어 예외 프로세스 검색
BOOLEAN  ISExist_SF_SFolderExceptProc( IN PWCHAR pwzProcName );


/******************************************************************************************************/
// ControlObject
/******************************************************************************************************/

// Add ControlObject
PCONTROL_OBJ ControlObjAdd( IN ULONG  ulBufferLen );

// Delete ControlObject
void ControlObjDelete( IN PCONTROL_OBJ pControlObject );


// CONTROL_OBJECT  Init
void ControlHeaderInit( IN PCONTROL_OBJ pObject, IN PLIST_ENTRY pListHead, IN BYTE ucType );


// Find
PCONTROL_OBJ  
ControlObjFind( IN PLIST_ENTRY  pListHead, IN PWCHAR  pObjectName );

PCONTROL_OBJ  
ControlObjFind_Str( IN PLIST_ENTRY  pListHead, IN PWCHAR pwzObjName );


/******************************************************************************************************/
// FileExt - Start
/******************************************************************************************************/

// EXT Add
PFILE_EXT  ControlFileExtAdd( IN ULONG ulFileExtLen );

// EXT Delete
void  ControlFileExtDelete( IN PFILE_EXT  pFileExt );

// FileExt Search
PFILE_EXT ControlFileExtFind( IN PLIST_ENTRY  pListHead, IN PWCHAR pFileExtName );

/******************************************************************************************************/
// FileExt - End
/******************************************************************************************************/



#ifdef __cplusplus
}
#endif


#endif