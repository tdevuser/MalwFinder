
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


// �������� ����ó�� �Ǵ����μ��� 	
BOOLEAN  
ISExpProcList_PolicyAll( IN PWCHAR pProcessName );

// ��ü��ȣ ��ŵ �������μ���
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
// PROCESS ���� �˻�
/******************************************************************************************************/

// ���μ��� �����Ʈ �Ǻ�
BOOLEAN  ISExist_P_ProcCtrl( IN PWCHAR pwzProcName );

// ���μ��� �������� ����
BOOLEAN  ISExist_P_ExceptDir( IN PWCHAR pwzExceptPath );

// ���μ��� �������� ����( �������� ����
BOOLEAN  ISExist_P_ExceptDir_Equal( IN PWCHAR pwzExceptPath );

/******************************************************************************************************/
// ����Ȯ���� ����˻�
/******************************************************************************************************/

// Ȯ���� ���� ����Ʈ �˻�
BOOLEAN ISExist_F_FileExtCtrl( IN PWCHAR pwzFileName );

// Ȯ���� ���� ���� ����Ʈ �˻�
BOOLEAN ISExist_F_FileExtCtrl_Except( IN PWCHAR pwzFileName );

// Ȯ���� �������� �˻�
BOOLEAN ISExist_F_ExceptDir( IN PWCHAR pwzExceptPath );
// Ȯ���� �������� �����̸� �˻�
BOOLEAN ISExist_F_ExceptDir_Equal( IN PWCHAR pwzExceptPath );

// Ȯ���� ���� ���μ��� �˻�
BOOLEAN ISExist_F_ExceptProc( IN PWCHAR pwzExceptProc );

/******************************************************************************************************/
// �������� ����˻�
/******************************************************************************************************/

// �������� �����Ʈ �˻�
// BOOLEAN  ISExist_SF_SFolderCtrl( IN PWCHAR pwzFilePath );

// ������������ �������� �˻�
BOOLEAN  ISExist_SF_SFolderExceptDir( IN PWCHAR pwzExceptPath );

// ������������ ���� ���μ��� �˻�
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