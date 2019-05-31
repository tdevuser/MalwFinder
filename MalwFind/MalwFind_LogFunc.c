
#include "MalwFind_DataTypes.h"
#include "MalwFind.h"
#include "MalwFind_Command.h"
#include "MalwFind_Acctl.h"
#include "MalwFind_LogFunc.h"
#include "MalwFind_Util.h"


extern G_MALW_FIND  g_MalwFind;
extern POBJECT_TYPE *IoFileObjectType;
const ULONG FS_LOG_IDENTIFICATION = 'wlaM';



// 로그를 쓸수 있는 정책인지 검사하는 정책인지 판단하는것
BOOLEAN   
ISLogWritePolicy( IN ULONG ulLogMode, IN WCHAR wcVol )
{   // 쓰레드가 돌고 있을때

	switch(ulLogMode)
	{
	case MFLOG_FDD:     
		{
		return g_MalwFind.DrvFile.FddPolicy.bLog;
		}
	case MFLOG_USB:     
		{
		return g_MalwFind.DrvFile.UsbPolicy.bLog;
		}
	case MFLOG_DLP:     
		{
		return g_MalwFind.DrvFile.FileExtPolicy.bLog;
		}
	case MFLOG_PROCESS: 
		{
		return g_MalwFind.DrvProc.Policy.bLog;	
		}
	default:  break;
	} 

	return FALSE;
}


// /***********************************************************************************************


// LogRec Queue Pop 
PLOG_RECORD  
LOG_Pop( IN PLIST_ENTRY  pListHead )
{
	PLOG_RECORD  pLogRecord  = NULL;

	if(!pListHead) return NULL;
	if(IsListEmpty( pListHead )) return NULL;
	if(KeGetCurrentIrql() > PASSIVE_LEVEL) return NULL;
	
	ExAcquireFastMutex( &g_MalwFind.DrvLog.LogLock );
	pLogRecord = (PLOG_RECORD)RemoveHeadList( pListHead );
	ExReleaseFastMutex( &g_MalwFind.DrvLog.LogLock );
	return pLogRecord;

}


// LogRec Queue Push
BOOLEAN      
LOG_PushTail( IN PLOG_RECORD pLogRecord )
{	
	if(!pLogRecord) return FALSE;
	if(KeGetCurrentIrql() > PASSIVE_LEVEL) return FALSE;

	ExAcquireFastMutex( &g_MalwFind.DrvLog.LogLock );
	InsertTailList( &g_MalwFind.DrvLog.LogListHead, &pLogRecord->ListEntry );
	ExReleaseFastMutex( &g_MalwFind.DrvLog.LogLock );
	return TRUE;

}



// LogRec Queue Push
BOOLEAN      
LOG_PushHead( IN PLOG_RECORD pLogRecord )
{
	if(!pLogRecord) return FALSE;
	if(KeGetCurrentIrql() > PASSIVE_LEVEL) return FALSE;
	
	ExAcquireFastMutex( &g_MalwFind.DrvLog.LogLock );
	InsertHeadList( &g_MalwFind.DrvLog.LogListHead, &pLogRecord->ListEntry );
	ExReleaseFastMutex( &g_MalwFind.DrvLog.LogLock );
	return TRUE;

}




// Log Alloc 
BOOLEAN 
LOG_Add( IN PLOG_HDR pHdr, IN PWCHAR pwzProcessName, IN PNAME_BUFFER pNameBuffer )
{
	BOOLEAN        bSuc = FALSE;
	ULONG          ulLength = 0, ulTotalLength = 0, ulRet = 0, ulRet2;
	NTSTATUS       Status = STATUS_SUCCESS;
	MY_CTIME       cTime = {0};
	
	PLOG_RECORD    pRecentLog    = NULL;
	PLOG_RECORD    pNewLogRecord = NULL;
	LARGE_INTEGER  ullSystemTime = {0};
	LARGE_INTEGER  ullLocalTime  = {0};

	if(LOG_OFF == g_MalwFind.DrvLog.ulLogStart) return FALSE;

	ASSERT( pHdr && pwzProcessName && pNameBuffer && pNameBuffer->pBuffer );
	if(!pHdr || !pwzProcessName || !pNameBuffer || !pNameBuffer->pBuffer) return FALSE;

	KeQuerySystemTime( &ullSystemTime ); 
	ulTotalLength  = sizeof(LOG_RECORD); 
	ulLength       = wcslen(pNameBuffer->pBuffer)+1;
	ulTotalLength += ulLength*sizeof(WCHAR);

	RtlZeroMemory( &cTime, sizeof(cTime) );
	ExSystemTimeToLocalTime( &ullSystemTime, &ullLocalTime );
	RtlTimeToTimeFields( &ullLocalTime, (PTIME_FIELDS)&cTime ); 
	

	// 최근 삽입된 로그라면 
	if( ISCheck_RecentLog( pHdr, pNameBuffer ) )
	{
		return FALSE;
	}


	if(KeGetCurrentIrql() > PASSIVE_LEVEL) return FALSE;

	pNewLogRecord = ExAllocatePoolWithTag( NonPagedPool, ulTotalLength, MALWFIND_NAME_TAG );
	if(!pNewLogRecord) return FALSE;
	RtlZeroMemory( pNewLogRecord, ulTotalLength );

	__try 
	{
		pNewLogRecord->Hdr.ulSignature = MF_SIGNATURE;
		pNewLogRecord->Hdr.ulTotalSize = ulTotalLength;
		pNewLogRecord->Hdr.ulLogID     = pHdr->ulLogID;
		pNewLogRecord->Hdr.ulAccess    = pHdr->ulAccess;
		pNewLogRecord->Hdr.ulProcessID = pHdr->ulProcessID;
		pNewLogRecord->Hdr.cTime       = cTime;

		Status = RtlUnicodeToMultiByteN( pNewLogRecord->cProcName, 
										 MAX_PROCESS_LEN, 
										 &ulRet, 
										 pwzProcessName, 
										 wcslen(pwzProcessName)*sizeof(WCHAR) ); 

		pNewLogRecord->ulFileLength = ulLength;
		Status = RtlUnicodeToMultiByteN( pNewLogRecord->cFileName, 
									     pNewLogRecord->ulFileLength*sizeof(WCHAR), 
									     &ulRet2, 
										 pNameBuffer->pBuffer, 
										 wcslen(pNameBuffer->pBuffer)*sizeof(WCHAR) );		
		

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		
		KdPrint(("LOG_Add >> Process=%ws >> FileName=%ws >>  Exception Occured. \n", pwzProcessName, pNameBuffer->pBuffer ));
		if(pNewLogRecord) ExFreePoolWithTag( pNewLogRecord, MALWFIND_NAME_TAG );
		return FALSE;

	}

	LOG_PushTail( pNewLogRecord );
	return TRUE;
}


// Log Free
void LOG_Delete( IN PLOG_RECORD pLogRecord )
{
	if(!pLogRecord) return;

	__try
	{
		// pRecord Delete
		ExFreePoolWithTag( pLogRecord, MALWFIND_NAME_TAG );
		pLogRecord = NULL;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

void LOG_DeleteAll(void)
{
	PLOG_RECORD  pLogRecord = NULL;

	do
	{
		pLogRecord = LOG_Pop( &g_MalwFind.DrvLog.LogListHead );
		if(!pLogRecord) break;

		ExFreePoolWithTag( pLogRecord, MALWFIND_NAME_TAG );
		pLogRecord = NULL;

	} while( TRUE );

}



NTSTATUS	
LOG_Fetch( char* pczOutBuffer, ULONG* pulOutCopySize, ULONG ulOutBufferSize )
{
	BOOLEAN      bRecent=FALSE, bEmpty=FALSE;
	PLOG_RECORD  pLogRecord = NULL;
	NTSTATUS     Status = STATUS_SUCCESS;
	ULONG        ulPos = 0, ulTempLength = 0, ulLogLength=0, nIndex=0;
	char         czBuffer[ MAX_512_PATH ];

	if(!pczOutBuffer)
	{
		KdPrint(("pczOutBuffer == NULL"));
		return STATUS_INFO_LENGTH_MISMATCH;
	}

	if(pulOutCopySize) *pulOutCopySize = 0;
	if(LOG_OFF == g_MalwFind.DrvLog.ulLogStart)
	{
		LOG_DeleteAll();
		return STATUS_SUCCESS;
	}

	// 초기화 되지 않으면 로그패치 못함
	if(!g_MalwFind.bEntryInit) return STATUS_UNSUCCESSFUL;
	if(KeGetCurrentIrql() >= APC_LEVEL) return STATUS_UNSUCCESSFUL;

	// 로그루프는 한번에 64 개씩 가져간다.
	for(nIndex=0; nIndex<MAX_FETCH_LOOP; nIndex++)
	{		
		bEmpty = IsListEmpty( &g_MalwFind.DrvLog.LogListHead );			
		if(bEmpty) 
		{
			break; // 비어있으면 끝낸다.
		}
		pLogRecord = LOG_Pop( &g_MalwFind.DrvLog.LogListHead );
		if(!pLogRecord) 
		{
			break;
		}
		bRecent = ISCheck_RecentLogEx( pLogRecord );
		if(bRecent)
		{
			LOG_Delete( pLogRecord );
			continue;
		}
		// 버퍼길이 계산한다.
		if( (ulPos+pLogRecord->Hdr.ulTotalSize) > ulOutBufferSize )
		{
			LOG_PushHead( pLogRecord );
			break;
		}

		RtlZeroMemory( czBuffer, MAX_512_PATH );
		RtlStringCchPrintfA( czBuffer, MAX_512_PATH, "[%04d%02d%02d%02d%02d%02d]^LogID=%d^Acctl=%d^PID=%d^Proc=%s^File=%s||",
			pLogRecord->Hdr.cTime.Year, pLogRecord->Hdr.cTime.Month,  pLogRecord->Hdr.cTime.Day, 
			pLogRecord->Hdr.cTime.Hour, pLogRecord->Hdr.cTime.Minute, pLogRecord->Hdr.cTime.Second,
			pLogRecord->Hdr.ulLogID, pLogRecord->Hdr.ulAccess, pLogRecord->Hdr.ulProcessID, pLogRecord->cProcName, pLogRecord->cFileName  );

		ulTempLength = strlen( czBuffer );
		RtlStringCchCatNA( (pczOutBuffer + ulPos), ulOutBufferSize, czBuffer, ulTempLength ); 

		ulPos = strlen( pczOutBuffer );
		if(pulOutCopySize) (*pulOutCopySize) = ulPos;

		// RecentLog 에 담는다.
		Update_RecentLog( pLogRecord );
		
		LOG_Delete( pLogRecord );

	}

	return STATUS_SUCCESS;
}






BOOLEAN  
Update_RecentLog ( IN PLOG_RECORD  pNewLog )
{
	int         nIndex  = 0;
	PLOG_RECENT pRecent = NULL;

	if(!pNewLog)
	{
		ASSERT( pNewLog );
		return FALSE;
	}

	for(nIndex=0; nIndex<MAX_RECENT; nIndex++)
	{
		pRecent = &g_MalwFind.DrvLog.RecentTable[ nIndex ];
		if(!pRecent) continue;

		if (g_MalwFind.DrvLog.nRecentIndex == nIndex || pRecent->ulLogID == MFLOG_NONE)
		{
			RtlZeroMemory( &g_MalwFind.DrvLog.RecentTable[ nIndex ], sizeof(LOG_RECENT) );
			g_MalwFind.DrvLog.RecentTable[nIndex].ulLogID     = pNewLog->Hdr.ulLogID;
			g_MalwFind.DrvLog.RecentTable[nIndex].ulProcessID = pNewLog->Hdr.ulProcessID;
			g_MalwFind.DrvLog.RecentTable[nIndex].ulAccess    = pNewLog->Hdr.ulAccess;
			g_MalwFind.DrvLog.RecentTable[nIndex].cTime       = pNewLog->Hdr.cTime;
			RtlStringCchCopyA( g_MalwFind.DrvLog.RecentTable[nIndex].cFileName, MAX_260_PATH, pNewLog->cFileName );
			// 한계값 조정
			g_MalwFind.DrvLog.nRecentIndex = (nIndex+1);
			if(g_MalwFind.DrvLog.nRecentIndex < 0 || g_MalwFind.DrvLog.nRecentIndex >= MAX_RECENT)
			{
				g_MalwFind.DrvLog.nRecentIndex = 0;
			}
			return TRUE;
		}
	}
	return FALSE;
}




BOOLEAN  
ISCheck_RecentLog( IN PLOG_HDR pLogHdr, IN PNAME_BUFFER pFullPath )
{
	int          nIndex = 0;
	ULONG        ulRet  = 0;
	char         czFileName[ MAX_260_PATH ];
	PLOG_RECENT  pRecent = NULL;

	if(!pLogHdr || !pFullPath || !pFullPath->pBuffer)
	{
		return FALSE;
	}

	RtlZeroMemory( czFileName, MAX_260_PATH );
	for(nIndex=0; nIndex<MAX_RECENT; nIndex++)
	{
		pRecent = &g_MalwFind.DrvLog.RecentTable[ nIndex ];
		if(!pRecent)  continue;
		if(pRecent->ulLogID == MFLOG_NONE) continue;

		if(pRecent->ulProcessID == pLogHdr->ulProcessID && pRecent->ulLogID == pLogHdr->ulLogID)
		{
			RtlZeroMemory( czFileName, MAX_260_PATH );
			RtlUnicodeToMultiByteN( czFileName, MAX_260_PATH, &ulRet, pFullPath->pBuffer, wcslen(pFullPath->pBuffer)*sizeof(WCHAR) );		

			if( !_stricmp( pRecent->cFileName, czFileName )     &&
				pLogHdr->cTime.Year   ==  pRecent->cTime.Year   &&
				pRecent->cTime.Month  ==  pLogHdr->cTime.Month  &&
				pRecent->cTime.Day    ==  pLogHdr->cTime.Day    &&
				pRecent->cTime.Hour   ==  pLogHdr->cTime.Hour   &&
				pRecent->cTime.Minute ==  pLogHdr->cTime.Minute &&
				pLogHdr->cTime.Second >= pRecent->cTime.Second  && 
				pLogHdr->cTime.Second <= pRecent->cTime.Second+2  )
			{
				KdPrint(("ISCheck_RecentLog == TRUE >> ProcessID=%08x, LogID=%d, Time=%04d%02d%02d-%02d%02d%02d FilName=%s \n", 
					pLogHdr->ulProcessID, pLogHdr->ulLogID,  		
					pLogHdr->cTime.Year, pLogHdr->cTime.Month,  pLogHdr->cTime.Day, 
					pLogHdr->cTime.Hour, pLogHdr->cTime.Minute, pLogHdr->cTime.Second,
					czFileName ));

				return TRUE;
			}
		}
	}
	return FALSE;
}


BOOLEAN  
ISCheck_RecentLogEx( IN PLOG_RECORD pNewLog )
{
	int  nIndex = 0;
	PLOG_RECENT  pRecent = NULL;

	if(!pNewLog)
	{
		ASSERT( pNewLog );
		return FALSE;
	}

	for(nIndex=0; nIndex<MAX_RECENT; nIndex++)
	{
		pRecent = &g_MalwFind.DrvLog.RecentTable[ nIndex ];
		if(!pRecent)  continue;
		if(pRecent->ulLogID == MFLOG_NONE) continue;
		
		if( pRecent->ulProcessID  ==  pNewLog->Hdr.ulProcessID  && 
			pRecent->ulLogID      ==  pNewLog->Hdr.ulLogID      && 
			!_stricmp( pRecent->cFileName, pNewLog->cFileName)   &&
			pRecent->cTime.Year   ==  pNewLog->Hdr.cTime.Year   &&
			pRecent->cTime.Month  ==  pNewLog->Hdr.cTime.Month  &&
			pRecent->cTime.Day    ==  pNewLog->Hdr.cTime.Day    &&
			pRecent->cTime.Hour   ==  pNewLog->Hdr.cTime.Hour   &&
			pRecent->cTime.Minute ==  pNewLog->Hdr.cTime.Minute &&
			pNewLog->Hdr.cTime.Second >= pRecent->cTime.Second  && 
			pNewLog->Hdr.cTime.Second <= pRecent->cTime.Second+2 )
		{
			KdPrint(("ISCheck_RecentLogEx == TRUE >> ProcessID=%08x, LogID=%d, Time=%04d%02d%02d-%02d%02d%02d FilName=%s \n", 
				    pNewLog->Hdr.ulProcessID, pNewLog->Hdr.ulLogID,  		
					pNewLog->Hdr.cTime.Year, pNewLog->Hdr.cTime.Month,  pNewLog->Hdr.cTime.Day, 
					pNewLog->Hdr.cTime.Hour, pNewLog->Hdr.cTime.Minute, pNewLog->Hdr.cTime.Second,
					pNewLog->cFileName  ));

			return TRUE;
		}

	}

	return FALSE;

}
