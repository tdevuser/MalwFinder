#include "MalwFind_DataTypes.h"
#include "MalwFind.h"
#include "MalwFind_FastIoFunc.h"
#include "MalwFind_FileFunc.h"
#include "MalwFind_Command.h"
#include "MalwFind_Acctl.h"
#include "MalwFind_Util.h"


extern G_MALW_FIND  g_MalwFind;


FAST_IO_DISPATCH  g_FastIoDispatch = 
{
    sizeof(FAST_IO_DISPATCH), 
    MalwFind_FastIoCheckifPossible,
    MalwFind_FastIoRead,
    MalwFind_FastIoWrite,
    MalwFind_FastIoQueryBasicInfo,
    MalwFind_FastIoQueryStandardInfo,
    MalwFind_FastIoLock,
    MalwFind_FastIoUnlockSingle,
    MalwFind_FastIoUnlockAll,
    MalwFind_FastIoUnlockAllByKey,
    MalwFind_FastIoDeviceControl,
    MalwFind_FastIoAcquireFile,
    MalwFind_FastIoReleaseFile,
    MalwFind_FastIoDetachDevice,
    //
    // new for NT 4.0
    //
    MalwFind_FastIoQueryNetworkOpenInfo,
    MalwFind_FastIoAcquireForModWrite,
    MalwFind_FastIoMdlRead,
    MalwFind_FastIoMdlReadComplete,
    MalwFind_FastIoPrepareMdlWrite,
    MalwFind_FastIoMdlWriteComplete,
    MalwFind_FastIoReadCompressed,
    MalwFind_FastIoWriteCompressed,
    MalwFind_FastIoMdlReadCompleteCompressed,
    MalwFind_FastIoMdlWriteCompleteCompressed,
    MalwFind_FastIoQueryOpen,
    MalwFind_FastIoReleaseForModWrite,
    MalwFind_FastIoAcquireForCcFlush,
    MalwFind_FastIoReleaseForCcFlush

};



BOOLEAN MalwFind_FastIoCheckifPossible( IN PFILE_OBJECT pFileObject, 
								        IN PLARGE_INTEGER FileOffset, 
								        IN ULONG Length, 
								        IN BOOLEAN Wait, 
								        IN ULONG LockKey, 
								        IN BOOLEAN CheckForReadOperation,
								        OUT PIO_STATUS_BLOCK IoStatus, 
								        IN  PDEVICE_OBJECT pDeviceObject ) 
{
    BOOLEAN            bRetVal = FALSE;
	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );
	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;	
	if (!pDevExt || !pDevExt->pLowerDeviceObject)
	{
		ASSERT(pDevExt && pDevExt->pLowerDeviceObject);
		return FALSE;
	}

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	if (!pFastIoDispatch)
	{
		ASSERT(pFastIoDispatch);
		return FALSE;
	}

	if(VALID_FAST_IO( pFastIoDispatch, FastIoCheckIfPossible ))
	{
		bRetVal = (pFastIoDispatch->FastIoCheckIfPossible)( pFileObject, FileOffset, 
															Length, Wait, LockKey, 
													        CheckForReadOperation, 
															IoStatus, 
															pDevExt->pLowerDeviceObject );
    }

    return bRetVal;
}


BOOLEAN  
MalwFind_FastIoRead( IN PFILE_OBJECT pFileObject, 
					IN PLARGE_INTEGER FileOffset, 
					IN ULONG Length, 
					IN BOOLEAN Wait, 
					IN ULONG LockKey, 
					OUT PVOID Buffer,
					OUT PIO_STATUS_BLOCK IoStatus, 
					IN PDEVICE_OBJECT pDeviceObject ) 
{
	int                nReturn     = 0;
    BOOLEAN            bRetVal = FALSE;
    PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;
	BOOLEAN            bSuc=FALSE;
	
    if(!pDeviceObject) return FALSE;
	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return bRetVal;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	if(!pFastIoDispatch) return bRetVal;

	// 유효성 체크
    bSuc = VALID_FAST_IO( pFastIoDispatch, FastIoRead);
	if(!bSuc) return FALSE;
	
	// 실제함수 호출
	bRetVal = (pFastIoDispatch->FastIoRead)( pFileObject, FileOffset, Length, Wait, LockKey, Buffer, IoStatus, pDevExt->pLowerDeviceObject );		
	return bRetVal;
}


BOOLEAN  
MalwFind_FastIoWrite(IN PFILE_OBJECT   pFileObject, 
			   IN PLARGE_INTEGER FileOffset, 
			   IN ULONG Length, 
			   IN BOOLEAN Wait, 
			   IN ULONG LockKey, 
			   IN PVOID Buffer,
			   OUT PIO_STATUS_BLOCK IoStatus, 
			   IN PDEVICE_OBJECT pDeviceObject ) 
{
    BOOLEAN            bRetVal = FALSE;	
	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;
	int                nReturn=0;
	NAME_BUFFER        FullPath = {0};
	
	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, FastIoWrite )) 
	{
        bRetVal = (pFastIoDispatch->FastIoWrite)( pFileObject, FileOffset, Length, 
												  Wait, LockKey, Buffer, IoStatus, 
												  pDevExt->pLowerDeviceObject );            
    }

    return bRetVal;
}


BOOLEAN  
MalwFind_FastIoQueryBasicInfo(IN PFILE_OBJECT pFileObject, 
						IN BOOLEAN Wait, 
						OUT PFILE_BASIC_INFORMATION Buffer,
						OUT PIO_STATUS_BLOCK IoStatus, 
						IN PDEVICE_OBJECT pDeviceObject   ) 
{
    BOOLEAN            bRetVal = FALSE;
	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, FastIoQueryBasicInfo )) 
	{
        bRetVal = 
			(pFastIoDispatch->FastIoQueryBasicInfo)( pFileObject, Wait, Buffer, 
			  									     IoStatus, pDevExt->pLowerDeviceObject );
    }

    return bRetVal;
}



BOOLEAN  MalwFind_FastIoQueryStandardInfo( IN PFILE_OBJECT pFileObject, 
									 IN BOOLEAN Wait, 
									 OUT PFILE_STANDARD_INFORMATION Buffer,
									 OUT PIO_STATUS_BLOCK IoStatus, 
									 IN PDEVICE_OBJECT pDeviceObject    ) 
{
	int                nReturn = 0;
    BOOLEAN            bSuc = FALSE, bRetVal = FALSE;
    PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;	
		
    if(!pDeviceObject) return FALSE;
	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	if(!pFastIoDispatch) return FALSE;

	bSuc = VALID_FAST_IO( pFastIoDispatch, FastIoQueryStandardInfo );
	if(!bSuc) return FALSE;

	bRetVal = (pFastIoDispatch->FastIoQueryStandardInfo)( pFileObject, Wait, Buffer, IoStatus, pDevExt->pLowerDeviceObject );	
	return bRetVal;
}




BOOLEAN  MalwFind_FastIoLock( IN PFILE_OBJECT pFileObject, 
					    IN PLARGE_INTEGER FileOffset,
						IN PLARGE_INTEGER Length, 
						PEPROCESS ProcessId, 
						ULONG Key,
						BOOLEAN FailImmediately, 
						BOOLEAN ExclusiveLock,
						OUT PIO_STATUS_BLOCK IoStatus, 
						IN PDEVICE_OBJECT pDeviceObject  ) 
{
    BOOLEAN            bRetVal = FALSE;
	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, FastIoLock )) 
	{
        bRetVal = 
		 (pFastIoDispatch->FastIoLock)( pFileObject, FileOffset, Length, 
			   						       ProcessId, Key, FailImmediately, ExclusiveLock, IoStatus, pDevExt->pLowerDeviceObject );
    }

    return bRetVal;
}


BOOLEAN  MalwFind_FastIoUnlockSingle( IN PFILE_OBJECT  pFileObject, 
							    IN PLARGE_INTEGER FileOffset,
								IN PLARGE_INTEGER Length, 
								PEPROCESS ProcessId, 
								ULONG Key,
								OUT PIO_STATUS_BLOCK IoStatus, 
								IN PDEVICE_OBJECT pDeviceObject ) 
{
	BOOLEAN            bRetVal = FALSE;
	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;
    
    if(VALID_FAST_IO( pFastIoDispatch, FastIoUnlockSingle )) 
	{
		bRetVal = 
		  (pFastIoDispatch->FastIoUnlockSingle)( pFileObject, FileOffset, Length, ProcessId, Key, IoStatus, pDevExt->pLowerDeviceObject );
    }

    return  bRetVal;
}


BOOLEAN  MalwFind_FastIoUnlockAll( IN PFILE_OBJECT pFileObject, 
							 PEPROCESS ProcessId,
							 OUT PIO_STATUS_BLOCK IoStatus, 
							 IN PDEVICE_OBJECT pDeviceObject ) 
{
	BOOLEAN            bRetVal = FALSE;
	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;
	
    if(VALID_FAST_IO( pFastIoDispatch, FastIoUnlockAll )) 
	{
		bRetVal = 
			(pFastIoDispatch->FastIoUnlockAll)( pFileObject, ProcessId, IoStatus, pDevExt->pLowerDeviceObject );
    }
    return bRetVal;
}


BOOLEAN  MalwFind_FastIoUnlockAllByKey( IN PFILE_OBJECT pFileObject, 
								  PEPROCESS ProcessId, 
								  ULONG Key,
								  OUT PIO_STATUS_BLOCK IoStatus, 
								  IN PDEVICE_OBJECT pDeviceObject  ) 
{
    BOOLEAN            bRetVal = FALSE;	
	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, FastIoUnlockAllByKey )) 
	{
		bRetVal = 
			(pFastIoDispatch->FastIoUnlockAllByKey)( pFileObject, ProcessId, Key, IoStatus, pDevExt->pLowerDeviceObject );
    }

    return bRetVal;
}


BOOLEAN MalwFind_FastIoQueryNetworkOpenInfo( IN PFILE_OBJECT pFileObject,
									   IN BOOLEAN Wait,
									   OUT struct _FILE_NETWORK_OPEN_INFORMATION *Buffer,
									   OUT PIO_STATUS_BLOCK IoStatus,
									   IN PDEVICE_OBJECT pDeviceObject  )
{
    BOOLEAN            bRetVal = FALSE;
   	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, FastIoQueryNetworkOpenInfo )) 
	{
		bRetVal = 
			(pFastIoDispatch->FastIoQueryNetworkOpenInfo)( pFileObject, Wait, Buffer, IoStatus, pDevExt->pLowerDeviceObject );
    }

    return bRetVal;

}

NTSTATUS MalwFind_FastIoAcquireForModWrite( IN PFILE_OBJECT pFileObject,
									  IN PLARGE_INTEGER EndingOffset,
									  OUT struct _ERESOURCE **ResourceToRelease,
									  IN PDEVICE_OBJECT pDeviceObject )
{
    NTSTATUS           bRetVal = STATUS_NOT_IMPLEMENTED;    
	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, AcquireForModWrite)) 
	{
        bRetVal = 
			(pFastIoDispatch->AcquireForModWrite)( pFileObject, EndingOffset, ResourceToRelease, pDevExt->pLowerDeviceObject );
    }

    return bRetVal;
}


BOOLEAN MalwFind_FastIoMdlRead( IN PFILE_OBJECT pFileObject,
					   IN PLARGE_INTEGER FileOffset, 
					   IN ULONG Length,
					   IN ULONG LockKey, 
					   OUT PMDL *MdlChain,
					   OUT PIO_STATUS_BLOCK IoStatus,
					   IN PDEVICE_OBJECT pDeviceObject )
{
    BOOLEAN            bRetVal = FALSE;	
	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, MdlRead )) 
	{
        bRetVal = 
			(pFastIoDispatch->MdlRead)( pFileObject, FileOffset, Length, LockKey, MdlChain, IoStatus, pDevExt->pLowerDeviceObject );
    }

    return bRetVal;
}


BOOLEAN MalwFind_FastIoMdlReadComplete( IN PFILE_OBJECT pFileObject,
								  IN PMDL MdlChain, 
								  IN PDEVICE_OBJECT pDeviceObject )								  
{
    BOOLEAN            bRetVal = FALSE;
    PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, MdlReadComplete )) 
	{
		bRetVal = 
			(pFastIoDispatch->MdlReadComplete)( pFileObject, MdlChain, pDevExt->pLowerDeviceObject );
    }
    return bRetVal;
}


BOOLEAN  MalwFind_FastIoPrepareMdlWrite( IN PFILE_OBJECT pFileObject,
								   IN PLARGE_INTEGER FileOffset, 
								   IN ULONG Length,
								   IN ULONG LockKey, 
								   OUT PMDL *MdlChain,
								   OUT PIO_STATUS_BLOCK IoStatus, 
								   IN PDEVICE_OBJECT pDeviceObject  )
{
    BOOLEAN            bRetVal = FALSE;
    PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    IoStatus->Status = STATUS_NOT_IMPLEMENTED;
    IoStatus->Information = 0;

    if(VALID_FAST_IO( pFastIoDispatch, PrepareMdlWrite )) 
	{
		bRetVal = 
		  (pFastIoDispatch->PrepareMdlWrite)( pFileObject, FileOffset, Length, LockKey, MdlChain, IoStatus, pDevExt->pLowerDeviceObject );            
    } 

    return bRetVal;
}


BOOLEAN  MalwFind_FastIoMdlWriteComplete( IN PFILE_OBJECT pFileObject,
								    IN PLARGE_INTEGER FileOffset, 
									IN PMDL MdlChain, 
									IN PDEVICE_OBJECT pDeviceObject )
{
    BOOLEAN            bRetVal = FALSE;
    PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, MdlWriteComplete )) 
	{ 
		bRetVal = 
		  (pFastIoDispatch->MdlWriteComplete)( pFileObject, FileOffset, MdlChain, pDevExt->pLowerDeviceObject );
    }
    return bRetVal;
}


BOOLEAN  MalwFind_FastIoReadCompressed( IN PFILE_OBJECT pFileObject,
								  IN PLARGE_INTEGER FileOffset, 
								  IN ULONG Length,
								  IN ULONG LockKey, 
								  OUT PVOID Buffer,
								  OUT PMDL *MdlChain, 
								  OUT PIO_STATUS_BLOCK IoStatus,
								  OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
								  IN ULONG CompressedDataInfoLength, 
								  IN PDEVICE_OBJECT pDeviceObject     )
{
    BOOLEAN            bRetVal = FALSE;
    PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, FastIoReadCompressed ))
	{
		bRetVal =
			(pFastIoDispatch->FastIoReadCompressed)( pFileObject, FileOffset, Length, 
													 LockKey, Buffer, MdlChain, IoStatus,
													 CompressedDataInfo, CompressedDataInfoLength, 
													 pDevExt->pLowerDeviceObject );
    }

    return bRetVal;
}


BOOLEAN  MalwFind_FastIoWriteCompressed( IN PFILE_OBJECT pFileObject,
								   IN PLARGE_INTEGER FileOffset, 
								   IN ULONG Length,
								   IN ULONG LockKey, 
								   OUT PVOID Buffer,
								   OUT PMDL *MdlChain, 
								   OUT PIO_STATUS_BLOCK IoStatus,
								   OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
								   IN ULONG CompressedDataInfoLength, 
								   IN PDEVICE_OBJECT pDeviceObject     )
{
    BOOLEAN            bRetVal = FALSE;
    PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, FastIoWriteCompressed )) 
	{
        bRetVal = 
			(pFastIoDispatch->FastIoWriteCompressed)( pFileObject, FileOffset, Length, 
													  LockKey, Buffer, MdlChain, IoStatus,
													  CompressedDataInfo, CompressedDataInfoLength, 
													  pDevExt->pLowerDeviceObject );
    }
    return bRetVal;
}


BOOLEAN  MalwFind_FastIoMdlReadCompleteCompressed( IN PFILE_OBJECT pFileObject,
											 IN PMDL MdlChain, 
											 IN PDEVICE_OBJECT pDeviceObject )
{
    BOOLEAN            bRetVal = FALSE;
    PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;
	
	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, MdlReadCompleteCompressed )) 
	{
        bRetVal = 
		  (pFastIoDispatch->MdlReadCompleteCompressed)( pFileObject, MdlChain, pDevExt->pLowerDeviceObject );
	}
    return bRetVal;
}


BOOLEAN  MalwFind_FastIoMdlWriteCompleteCompressed( IN PFILE_OBJECT pFileObject,
											  IN PLARGE_INTEGER FileOffset, 
											  IN PMDL MdlChain, 
											  IN PDEVICE_OBJECT pDeviceObject )
{
    BOOLEAN            bRetVal = FALSE;
    PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, MdlWriteCompleteCompressed )) 
	{
        bRetVal = 
		  (pFastIoDispatch->MdlWriteCompleteCompressed)( pFileObject, FileOffset, MdlChain, pDevExt->pLowerDeviceObject );
    }
    return bRetVal;
}


BOOLEAN  
MalwFind_FastIoQueryOpen( IN PIRP pIrp, OUT PFILE_NETWORK_OPEN_INFORMATION  pNetworkInformation, IN PDEVICE_OBJECT pDeviceObject )
{
    BOOLEAN             bRetVal = FALSE;
    PFLT_EXTENSION      pDevExt = NULL;
	PDEVICE_OBJECT      pLowerDevObj    = NULL;
	PFAST_IO_DISPATCH   pFastIoDispatch = NULL;
    PIO_STACK_LOCATION  pIrpStack   = NULL;
    
	ASSERT( pDeviceObject );
	if(!pDeviceObject)  return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pLowerDevObj = pDevExt->pLowerDeviceObject;
	if(!pLowerDevObj) return FALSE;

	pFastIoDispatch = pLowerDevObj->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;
	
	if(VALID_FAST_IO( pFastIoDispatch, FastIoQueryOpen )) 
	{
		pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
		ASSERT( pIrpStack );
		if(!pIrpStack) return FALSE;

		pIrpStack->DeviceObject = pLowerDevObj;
		bRetVal = 
		  (pFastIoDispatch->FastIoQueryOpen)( pIrp, pNetworkInformation, pLowerDevObj );

		pIrpStack->DeviceObject = pDeviceObject;
		return bRetVal;
	}

    return FALSE;
}



NTSTATUS  MalwFind_FastIoReleaseForModWrite( IN PFILE_OBJECT pFileObject,
								       IN struct _ERESOURCE* pResourceToRelease,
									   IN PDEVICE_OBJECT pDeviceObject )
{
    NTSTATUS           Status  = STATUS_NOT_IMPLEMENTED;
	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return STATUS_NOT_IMPLEMENTED;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, ReleaseForModWrite )) 
	{
        Status = 
		  (pFastIoDispatch->ReleaseForModWrite)( pFileObject, pResourceToRelease, pDevExt->pLowerDeviceObject );
    }
    return Status;
}


NTSTATUS  
MalwFind_FastIoAcquireForCcFlush( IN PFILE_OBJECT pFileObject, IN PDEVICE_OBJECT pDeviceObject )
{
    NTSTATUS          Status  = STATUS_NOT_IMPLEMENTED;
    PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return STATUS_NOT_IMPLEMENTED;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, AcquireForCcFlush )) 
	{
        Status = (pFastIoDispatch->AcquireForCcFlush)( pFileObject, pDevExt->pLowerDeviceObject );
    }
    return Status;
}


NTSTATUS  MalwFind_FastIoReleaseForCcFlush( IN PFILE_OBJECT pFileObject, IN PDEVICE_OBJECT pDeviceObject )
{
    NTSTATUS           Status  = STATUS_NOT_IMPLEMENTED;
    PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;

	ASSERT( pDeviceObject );
    if(!pDeviceObject) return STATUS_NOT_IMPLEMENTED;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt || !pDevExt->pLowerDeviceObject) return FALSE;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
	ASSERT( pFastIoDispatch );
	if(!pFastIoDispatch) return FALSE;

    if(VALID_FAST_IO( pFastIoDispatch, ReleaseForCcFlush )) 
	{
        Status = (pFastIoDispatch->ReleaseForCcFlush)( pFileObject, pDevExt->pLowerDeviceObject );
    }
    return Status;
}


void  
MalwFind_FastIoAcquireFile( PFILE_OBJECT pFileObject ) 
{
	PDEVICE_OBJECT     pDevObj = NULL;
	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;
	
	if(!pFileObject)
	{
		ASSERT( pFileObject );
		return;
	}

    pDevObj = IoGetRelatedDeviceObject( pFileObject );	
	if(!pDevObj) 
	{
		ASSERT( pDevObj );
		return;
	}

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDevObj ) );

	pDevExt = (PFLT_EXTENSION)pDevObj->DeviceExtension;
	if(!pDevExt) 
	{
		ASSERT( pDevExt );
		return;
	}

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;	
	if(!pFastIoDispatch) 
	{
		ASSERT( pFastIoDispatch );
		return;
	}

	if(VALID_FAST_IO( pFastIoDispatch, AcquireFileForNtCreateSection)) 
	{
		(pFastIoDispatch->AcquireFileForNtCreateSection)( pFileObject );
	}

}


void   
MalwFind_FastIoReleaseFile( PFILE_OBJECT pFileObject ) 
{
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;
    PDEVICE_OBJECT     pDevObj = NULL;
	PFLT_EXTENSION     pDevExt = NULL;

	if(!pFileObject) 
	{
		ASSERT( pFileObject );
		return;
	}

	pDevObj = IoGetRelatedDeviceObject( pFileObject );	
	if(!pDevObj)
	{
		ASSERT( pDevObj );
		return;
	}

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDevObj ) );

	pDevExt = (PFLT_EXTENSION)pDevObj->DeviceExtension;
	ASSERT( pDevExt && pDevExt->pLowerDeviceObject );
	if(!pDevExt) return;

	pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;	
	if(!pFastIoDispatch) 
	{
		ASSERT( pFastIoDispatch );
		return;
	}

	if(VALID_FAST_IO( pFastIoDispatch, ReleaseFileForNtCreateSection )) 
	{
		(pFastIoDispatch->ReleaseFileForNtCreateSection)( pFileObject );
	}

}


void  
MalwFind_FastIoDetachDevice( IN PDEVICE_OBJECT pSourceDevice, IN PDEVICE_OBJECT pTargetDevice ) 
{
	PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;
	NAME_BUFFER        FullPath = {0};

	ASSERT( pSourceDevice && pTargetDevice );
	if(!pSourceDevice || !pTargetDevice) return;
	if(!MmIsAddressValid( pSourceDevice ) || !MmIsAddressValid( pTargetDevice )) return;

	__try
	{

		if(IS_MALFIND_DEVICE_OBJECT( pSourceDevice ))
		{
			pDevExt = (PFLT_EXTENSION)pSourceDevice->DeviceExtension;
			if(pDevExt) KdPrint(("MalwFind_DeviceObject--MalwFind_FastIoDetachDevice: [ %c:\\ ] \n", pDevExt->wcVol ));
			else        KdPrint(("MalwFind_DeviceObject--MalwFind_FastIoDetachDevice: \n"));
	
			ALLOCATE_N_POOL( FullPath );
			if(FullPath.pBuffer)
			{
				SET_POOL_ZERO( FullPath );
				FullPath.pBuffer[0] = pDevExt->wcVol;
				FullPath.pBuffer[1] = L':';
				FullPath.pBuffer[2] = L'\\';
				FullPath.pBuffer[3] = L'\0';
				FREE_N_POOL( FullPath );
			}

			FsdCleanupMountDevice( pSourceDevice );
			IoDetachDevice( pTargetDevice );
			IoDeleteDevice( pSourceDevice );
			pSourceDevice = NULL;
			return;
		}
		
		/*
		pDevExt = (PFLT_EXTENSION)pSourceDevice->DeviceExtension;
		ASSERT( pDevExt );
		if(!pDevExt) return;

		pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
		ASSERT( pFastIoDispatch );
		if(!pFastIoDispatch) return;

		if(VALID_FAST_IO( pFastIoDispatch, FastIoDetachDevice ))
		{
			(pFastIoDispatch->FastIoDetachDevice)( pSourceDevice, pTargetDevice );
		}
		*/

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{

		KdPrint(("MalwFind_FastIoDetachDevice Exception Occured. \n"));

	}


}



BOOLEAN  
MalwFind_FastIoDeviceControl( IN  PFILE_OBJECT  pFileObject, 
						     IN  BOOLEAN Wait,
						     IN  PVOID InputBuffer, 
						     IN  ULONG InputBufferLength, 
						     OUT PVOID OutputBuffer, 
						     IN  ULONG OutputBufferLength, 
						     IN  ULONG IoControlCode,
						     OUT PIO_STATUS_BLOCK IoStatus, 
						     IN  PDEVICE_OBJECT pDeviceObject ) 
{
    BOOLEAN            bRetVal = FALSE;
    PFLT_EXTENSION     pDevExt = NULL;
	PFAST_IO_DISPATCH  pFastIoDispatch = NULL;
	NTSTATUS           Status = STATUS_SUCCESS;

	ASSERT( pDeviceObject );
	if(!pDeviceObject) return FALSE;

	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );
    pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	ASSERT( pDevExt);
	if(!pDevExt) return FALSE;

	if(g_MalwFind.pDeviceObject == pDeviceObject)
	{   
		IoStatus->Status = STATUS_SUCCESS; 
        IoStatus->Information = 0; 

		Status = FltDrv_DeviceControl( pFileObject, 
								       pDeviceObject, 
								       IoControlCode,
								       InputBuffer,
								       InputBufferLength, 
								       OutputBuffer,
								       OutputBufferLength,
									   IoStatus );
    }
	else // AttachedDevice
	{	
		if(!pDevExt->pLowerDeviceObject) return FALSE;

		pFastIoDispatch = pDevExt->pLowerDeviceObject->DriverObject->FastIoDispatch;
		ASSERT( pFastIoDispatch );
		if(!pFastIoDispatch) return FALSE;

		if(VALID_FAST_IO( pFastIoDispatch, FastIoDeviceControl ))
		{
            bRetVal = (pFastIoDispatch->FastIoDeviceControl)( pFileObject, 
												   			  Wait, 
															  InputBuffer, 
															  InputBufferLength, 
															  OutputBuffer, 
															  OutputBufferLength, 
															  IoControlCode, 
															  IoStatus, 
															  pDevExt->pLowerDeviceObject );
        }
	}

    return bRetVal;
}

