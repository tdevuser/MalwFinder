#ifndef _MALWFIND_LOG_FUNC_H_
#define _MALWFIND_LOG_FUNC_H_

#include <ntifs.h>
#include "MalwFind_DataTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif


// /**********************************************************************************************************************
//  // LogThread
// /**********************************************************************************************************************

// Thread Start
NTSTATUS  
LOGThreadStart(void);

// Thread Stop
NTSTATUS   
LOGThreadStop(void);



#define MAX_FETCH_LOOP  256

// /**********************************************************************************************************************
// /**********************************************************************************************************************

// LogRec Queue Push
BOOLEAN      
LOG_PushTail( IN PLOG_RECORD  pLogRecord );

// LogRec Queue Push
BOOLEAN  LOG_PushHead( IN PLOG_RECORD pLogRecord );
// LogRec Queue Pop
PLOG_RECORD  LOG_Pop( IN PLIST_ENTRY  pListHead );

void LOG_Delete( IN PLOG_RECORD pLogRecord );
void LOG_DeleteAll(void);

BOOLEAN  LOG_Add( IN PLOG_HDR pHdr, IN PWCHAR pwzProcessName, IN PNAME_BUFFER pNameBuffer );
BOOLEAN  ISLogWritePolicy( IN ULONG ulLogMode, IN WCHAR wcVol );

NTSTATUS LOG_Fetch( char* pczOutBuffer, ULONG* pulOutCopySize, ULONG ulOutBufferSize );



BOOLEAN  
ISCheck_RecentLog( IN PLOG_HDR pLogHdr, IN PNAME_BUFFER pFullPath );

BOOLEAN  
ISCheck_RecentLogEx( IN PLOG_RECORD  pNewLog );

BOOLEAN  
Update_RecentLog ( IN PLOG_RECORD  pNewLog );



#ifdef __cplusplus
}
#endif


#endif