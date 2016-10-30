#ifndef _MALWFIND_FASTIO_FUNC_H_
#define _MALWFIND_FASTIO_FUNC_H_

#include  <ntifs.h>

#ifdef __cplusplus
extern "C"
{
#endif
	

#define VALID_FAST_IO( pFastIoDispatch, FieldName ) \
        (((pFastIoDispatch) != NULL) && \
        (((pFastIoDispatch)->SizeOfFastIoDispatch) >= \
        (FIELD_OFFSET(FAST_IO_DISPATCH, FieldName) + sizeof(VOID *))) && \
        ((pFastIoDispatch)->FieldName != NULL))


	
BOOLEAN MalwFind_FastIoCheckifPossible( IN PFILE_OBJECT FileObject, 
								  IN PLARGE_INTEGER FileOffset, 						   
								  IN ULONG Length, 						   
								  IN BOOLEAN Wait, 								   
								  IN ULONG LockKey, 								   
								  IN BOOLEAN CheckForReadOperation,								   
								  OUT PIO_STATUS_BLOCK IoStatus, 								   
								  IN PDEVICE_OBJECT DeviceObject  	);

BOOLEAN MalwFind_FastIoRead(  IN PFILE_OBJECT FileObject, 		
					    IN PLARGE_INTEGER FileOffset, 		
					    IN ULONG Length, 		
					    IN BOOLEAN Wait, 		
					    IN ULONG LockKey, 		
					    OUT PVOID Buffer,		
					    OUT PIO_STATUS_BLOCK IoStatus, 		
				        IN PDEVICE_OBJECT DeviceObject 	);
	
BOOLEAN  MalwFind_FastIoWrite( IN PFILE_OBJECT FileObject, 
						 IN PLARGE_INTEGER FileOffset,                             
						 IN ULONG Length, 
						 IN BOOLEAN Wait, 
						 IN ULONG LockKey, 
						 IN PVOID Buffer,
						 OUT PIO_STATUS_BLOCK IoStatus, 
						 IN PDEVICE_OBJECT DeviceObject  );

BOOLEAN  MalwFind_FastIoQueryBasicInfo( IN  PFILE_OBJECT FileObject, 
								  IN  BOOLEAN Wait, 
								  OUT PFILE_BASIC_INFORMATION Buffer,
								  OUT PIO_STATUS_BLOCK IoStatus, 
								  IN  PDEVICE_OBJECT DeviceObject   );

BOOLEAN  MalwFind_FastIoQueryStandardInfo( IN  PFILE_OBJECT FileObject, 
									 IN  BOOLEAN Wait, 
									 OUT PFILE_STANDARD_INFORMATION Buffer,
									 OUT PIO_STATUS_BLOCK IoStatus, 
									 IN  PDEVICE_OBJECT DeviceObject  );

BOOLEAN  MalwFind_FastIoLock( IN PFILE_OBJECT FileObject, 
					    IN PLARGE_INTEGER FileOffset,
						IN PLARGE_INTEGER Length, 
						PEPROCESS ProcessId, 
						ULONG Key,
						BOOLEAN FailImmediately, 
						BOOLEAN ExclusiveLock,
						OUT PIO_STATUS_BLOCK IoStatus, 
						IN PDEVICE_OBJECT DeviceObject   );
	
BOOLEAN  MalwFind_FastIoUnlockSingle( IN PFILE_OBJECT FileObject, 
							    IN PLARGE_INTEGER FileOffset,
								IN PLARGE_INTEGER Length, 
								PEPROCESS ProcessId, 
								ULONG Key,
								OUT PIO_STATUS_BLOCK IoStatus, 
								IN PDEVICE_OBJECT DeviceObject  );
	
BOOLEAN  MalwFind_FastIoUnlockAll( IN PFILE_OBJECT FileObject, 
							 PEPROCESS ProcessId,
							 OUT PIO_STATUS_BLOCK IoStatus, 
							 IN PDEVICE_OBJECT DeviceObject  );
	
BOOLEAN  MalwFind_FastIoUnlockAllByKey( IN PFILE_OBJECT FileObject, 
								       PEPROCESS ProcessId, 
									   ULONG Key,
								       OUT PIO_STATUS_BLOCK IoStatus, 
								       IN PDEVICE_OBJECT DeviceObject  );
	
BOOLEAN  MalwFind_FastIoDeviceControl( IN PFILE_OBJECT FileObject,
								 IN BOOLEAN Wait,
								 IN PVOID InputBuffer, 
								 IN ULONG InputBufferLength, 
								 OUT PVOID OutbufBuffer, 
								 IN ULONG OutputBufferLength, 
								 IN ULONG IoControlCode,
								 OUT PIO_STATUS_BLOCK IoStatus, 
								 IN PDEVICE_OBJECT DeviceObject  );

void  
MalwFind_FastIoAcquireFile( IN PFILE_OBJECT FileObject );

void  
MalwFind_FastIoReleaseFile( IN PFILE_OBJECT FileObject );

void  
MalwFind_FastIoDetachDevice( IN PDEVICE_OBJECT SourceDevice, IN PDEVICE_OBJECT TargetDevice );

	
//
// These are new NT 4.0 Fast I/O calls
//
BOOLEAN MalwFind_FastIoQueryNetworkOpenInfo( IN PFILE_OBJECT FileObject,
									   IN BOOLEAN Wait, 
									   OUT struct _FILE_NETWORK_OPEN_INFORMATION *Buffer,
									   OUT struct _IO_STATUS_BLOCK *IoStatus, 
									   IN PDEVICE_OBJECT DeviceObject  );

NTSTATUS MalwFind_FastIoAcquireForModWrite( IN PFILE_OBJECT FileObject,
									  IN PLARGE_INTEGER EndingOffset, 
									  OUT struct _ERESOURCE **ResourceToRelease,
									  IN PDEVICE_OBJECT DeviceObject  );

BOOLEAN  MalwFind_FastIoMdlRead( IN PFILE_OBJECT FileObject,
						   IN PLARGE_INTEGER FileOffset, 
						   IN ULONG Length,
						   IN ULONG LockKey, 
						   OUT PMDL *MdlChain, 
						   OUT PIO_STATUS_BLOCK IoStatus,
						   IN PDEVICE_OBJECT DeviceObject  );

BOOLEAN  MalwFind_FastIoMdlReadComplete( IN PFILE_OBJECT FileObject,
								   IN PMDL MdlChain, 
								   IN PDEVICE_OBJECT DeviceObject );

BOOLEAN  MalwFind_FastIoPrepareMdlWrite( IN PFILE_OBJECT FileObject,
								   IN PLARGE_INTEGER FileOffset, 
								   IN ULONG Length, 
								   IN ULONG LockKey,
								   OUT PMDL *MdlChain, 
								   OUT PIO_STATUS_BLOCK IoStatus,
								   IN PDEVICE_OBJECT DeviceObject  );

BOOLEAN  MalwFind_FastIoMdlWriteComplete( IN PFILE_OBJECT FileObject,
								    IN PLARGE_INTEGER FileOffset, 
									IN PMDL MdlChain,
									IN PDEVICE_OBJECT DeviceObject );

BOOLEAN  MalwFind_FastIoReadCompressed( IN PFILE_OBJECT FileObject,
								  IN PLARGE_INTEGER FileOffset, 
								  IN ULONG Length,
								  IN ULONG LockKey, 
								  OUT PVOID Buffer, 
								  OUT PMDL *MdlChain,
								  OUT PIO_STATUS_BLOCK IoStatus,
								  OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
								  IN ULONG CompressedDataInfoLength, 
								  IN PDEVICE_OBJECT DeviceObject  );

BOOLEAN  MalwFind_FastIoWriteCompressed( IN PFILE_OBJECT FileObject,
								   IN PLARGE_INTEGER FileOffset, 
								   IN ULONG Length,
								   IN ULONG LockKey, 
								   IN PVOID Buffer, 
								   OUT PMDL *MdlChain,
								   OUT PIO_STATUS_BLOCK IoStatus,
								   IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
								   IN ULONG CompressedDataInfoLength, 
								   IN PDEVICE_OBJECT DeviceObject );

BOOLEAN  MalwFind_FastIoMdlReadCompleteCompressed( IN PFILE_OBJECT FileObject,
											 IN PMDL MdlChain, 
											 IN PDEVICE_OBJECT DeviceObject );

BOOLEAN  MalwFind_FastIoMdlWriteCompleteCompressed( IN PFILE_OBJECT FileObject,
											  IN PLARGE_INTEGER FileOffset, 
											  IN PMDL MdlChain,
											  IN PDEVICE_OBJECT DeviceObject );

BOOLEAN  MalwFind_FastIoQueryOpen( IN struct _IRP *Irp,
							 OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
							 IN PDEVICE_OBJECT DeviceObject  );

NTSTATUS MalwFind_FastIoReleaseForModWrite( IN PFILE_OBJECT FileObject,
									  IN struct _ERESOURCE *ResourceToRelease, 
									  IN PDEVICE_OBJECT DeviceObject  );

NTSTATUS MalwFind_FastIoAcquireForCcFlush( IN PFILE_OBJECT FileObject, IN PDEVICE_OBJECT DeviceObject );

NTSTATUS MalwFind_FastIoReleaseForCcFlush( IN PFILE_OBJECT FileObject, IN PDEVICE_OBJECT DeviceObject );







#ifdef __cplusplus
}
#endif

#endif