
#include "MalwFind_DataTypes.h"
#include "MalwFind.h"
#include "MalwFind_FastIoFunc.h"
#include "MalwFind_FileFunc.h"
#include "MalwFind_Command.h"
#include "MalwFind_Acctl.h"
#include "MalwFind_LogFunc.h"
#include "MalwFind_Util.h"
#include "MalwFind_Cache.h"


extern G_MALW_FIND  g_MalwFind;

const WCHAR* g_pFsdName[FSD_MAX+1] = 
{ 
	FSD_FAT, 
	FSD_EXFAT,
	FSD_FASTFAT, 
	FSD_NTFS, 
	FSD_CDFS, 
	FSD_UDFS, 
	FSD_FAT_CDROM,
	FSD_FAT_DISK,    
	FSD_UDFS_DISK,
	FSD_UDFS_CDROM,
	FSD_LANMAN, 
	FSD_FSREC_EXFAT,
	FSD_FSREC_CDFS,
	FSD_FSREC_FAT_CDROM,
	FSD_FSREC_UDFS_CDROM,
	FSD_FSREC_FAT_DISK,
	FSD_FSREC_UDFS_DISK,
	FSD_FSREC,
	// Windows Vista
	FSD_FSREC_EXFAT,
	FSD_MUP,
	FSD_RDPDR,  
	FSD_HGFS,   
	NULL 
};


#define MAX_NTFSMETA_FILENAME 12
const WCHAR* g_pNtfsMetaFileName[] = 
{
    L"$Mft",
    L"$MftMirr",
    L"$LogFile",
    L"$Volume",
    L"$AttrDef",
    L"$Root",
    L"$Bitmap",
    L"$Boot",
    L"$BadClus",
    L"$Secure",
    L"$UpCase",
    L"$Extend",
    NULL
};



BOOLEAN  
ISNtfsMetaFileName( IN PNAME_BUFFER pFullPath )
{
	int nIndex=0, nLength=0;

	if(!pFullPath || !pFullPath->pBuffer) return FALSE;
	if(pFullPath->ulLength <= 3) return FALSE;

	if( !_wcsnicmp( &pFullPath->pBuffer[2], L"\\ReadyBoostPerfTest.tmp",      wcslen(L"\\ReadyBoostPerfTest.tmp" )) ||
		!_wcsnicmp( &pFullPath->pBuffer[2], L"\\System Volume Information\\", wcslen(L"\\System Volume Information\\")) ||
		!_wcsnicmp( &pFullPath->pBuffer[2], L"\\RECYCLER\\",    wcslen(L"\\RECYCLER\\"))  ||
		!_wcsnicmp( &pFullPath->pBuffer[2], L"\\Recycled\\",    wcslen(L"\\Recycled\\"))  ||
		!_wcsnicmp( &pFullPath->pBuffer[2], L"\\RECYCLE.BIN\\", wcslen(L"\\RECYCLE.BIN\\"))  )
	{
		return TRUE;
	}

	for(nIndex=0; nIndex<MAX_NTFSMETA_FILENAME; nIndex++)
	{
		if(!g_pNtfsMetaFileName[nIndex]) continue;
		nLength = (ULONG)wcslen( g_pNtfsMetaFileName[nIndex]);
		if(!_wcsnicmp( &pFullPath->pBuffer[3], g_pNtfsMetaFileName[nIndex], nLength ))
		{
			KdPrint(("\nISCheck_NtfsMetaFileName >> FileName=%ws \n", pFullPath->pBuffer ));
			return TRUE;
		}
	}

	return FALSE;
}




ULONG GetAccessMode( ACCESS_MASK  AccessMask )
{
	ULONG  ulRetCap = 0;

	if((AccessMask == FILE_ANY_ACCESS) || (AccessMask == FILE_ALL_ACCESS))
	{
		ulRetCap = MALWF_READ | MALWF_WRITE | MALWF_EXECUTE | MALWF_CREATE | MALWF_DELETE | MALWF_RENAME;
	}
	else 
	{
		ACCESS_MASK  Mask=0;

		Mask = FILE_READ_ACCESS | FILE_READ_EA | FILE_READ_ATTRIBUTES | GENERIC_READ;
		//Mask = FILE_READ_ACCESS |	FILE_READ_EA | GENERIC_READ;
		if (AccessMask & Mask) ulRetCap |= MALWF_READ;

		Mask = FILE_WRITE_ACCESS | FILE_APPEND_DATA | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES | GENERIC_WRITE;
		if (AccessMask & Mask) ulRetCap |= MALWF_WRITE;

		Mask = FILE_EXECUTE | GENERIC_EXECUTE;
		if (AccessMask & Mask) ulRetCap |= MALWF_EXECUTE;
	}

	return ulRetCap;

}


NTSTATUS 
VALID_CHECK( IN PDEVICE_OBJECT pDeviceObject, 
			 IN PIRP  pIrp, 
			 IN PPFLT_EXTENSION   ppDevExt, 
			 PIO_STACK_LOCATION*  ppIrpStack )
{
	NTSTATUS Status = STATUS_SUCCESS;

	__try
	{
		if(!pDeviceObject || !pIrp || !ppDevExt || !ppIrpStack) 
		{
			ASSERT( pDeviceObject );
			Status = STATUS_INVALID_PARAMETER;
			return Status;
		}

		if( (PVOID)pDeviceObject < (PVOID)0x80000000 || (PVOID)pIrp < (PVOID)0x80000000 )
		{
			Status = STATUS_INVALID_DEVICE_REQUEST;
			KdPrint(("STATUS_INVALID_DEVICE_REQUEST >> VALID_CHECK 1 \n"));
			return Status;
		}

		*ppDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;  
		if(NULL == (*ppDevExt))
		{
			ASSERT( *ppDevExt ); 
			Status = STATUS_INVALID_DEVICE_REQUEST;
			KdPrint(("STATUS_INVALID_DEVICE_REQUEST >> VALID_CHECK 2 \n"));
			return Status;
		}

		*ppIrpStack = IoGetCurrentIrpStackLocation( pIrp );  
		if(NULL == (*ppIrpStack) )
		{
			ASSERT( *ppIrpStack );  
			Status = STATUS_INVALID_DEVICE_REQUEST;
			KdPrint(("STATUS_INVALID_DEVICE_REQUEST >> VALID_CHECK 2 \n"));
			return Status;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint( "VALID_CHECK >> Exception Ocuured. pDeviceObject=0x%08x, pIrp=0x%08x \n", pDeviceObject, pIrp );
		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;

}


NTSTATUS 
CallDriverSkip( IN PFLT_EXTENSION pDevExt, IN PIRP pIrp )
{
	NTSTATUS Status = STATUS_SUCCESS;
	
	if(!pIrp || !pDevExt || !pDevExt->pLowerDeviceObject)
	{
		Status = STATUS_INVALID_PARAMETER;
		KdPrint(("STATUS_INVALID_PARAMETER >> CallDriverSkip 1. \n"));
		return CompleteRequest( pIrp, Status, 0 );	
	}

	if( (PVOID)pIrp < (PVOID)0x80000000 )
	{
		Status = STATUS_INVALID_PARAMETER;
		KdPrint(("STATUS_INVALID_PARAMETER >> CallDriverSkip 2. \n"));
		return CompleteRequest( pIrp, Status, 0 );	
	}

	if( (PVOID)pDevExt->pLowerDeviceObject < (PVOID)0x80000000 )
	{
		Status = STATUS_INVALID_PARAMETER;
		KdPrint(("STATUS_INVALID_PARAMETER >> CallDriverSkip 3. \n"));
		return CompleteRequest( pIrp, Status, 0 );	
	}

	IoSkipCurrentIrpStackLocation( pIrp );
	Status = IoCallDriver( pDevExt->pLowerDeviceObject, pIrp );

	return Status;

}


NTSTATUS 	
CallDriverCopy( IN PFLT_EXTENSION pDevExt, 
	            IN PIRP pIrp, 
	            IN PIO_COMPLETION_ROUTINE  pCompletionRoutine, 
	            IN PVOID pContext )
{	
	NTSTATUS Status = STATUS_SUCCESS;

	if(!pIrp || !pDevExt || !pDevExt->pLowerDeviceObject || !pCompletionRoutine)
	{
		ASSERT( pDevExt && pIrp && pCompletionRoutine ); 
		Status = STATUS_INVALID_PARAMETER;
		KdPrint(("STATUS_INVALID_PARAMETER >> CallDriverCopy 1. \n"));
		return CompleteRequest( pIrp, Status, 0 );	
	}

	if( (PVOID)pIrp < (PVOID)0x80000000 )
	{
		Status = STATUS_INVALID_PARAMETER;
		KdPrint(("STATUS_INVALID_PARAMETER >> CallDriverCopy 2. \n"));
		return CompleteRequest( pIrp, Status, 0 );	
	}

	if( (PVOID)pDevExt->pLowerDeviceObject < (PVOID)0x80000000 )
	{
		Status = STATUS_INVALID_PARAMETER;
		KdPrint(("STATUS_INVALID_PARAMETER >> CallDriverCopy 3. \n"));
		return CompleteRequest( pIrp, Status, 0 );	
	}

	IoCopyCurrentIrpStackLocationToNext( pIrp );
	IoSetCompletionRoutine( pIrp, (PIO_COMPLETION_ROUTINE)pCompletionRoutine, pContext, TRUE, TRUE, TRUE );
	Status = IoCallDriver( pDevExt->pLowerDeviceObject, pIrp );
	return Status;

}


/*********************************************************************************************************************************/
// File & Dir 구분
/*********************************************************************************************************************************/

ULONG  
ISFileWithCreateOption( IN PIO_STACK_LOCATION  pIrpStack )
{
	ULONG  ulRet = MFOBJ_NONE;
	if(!pIrpStack) return ulRet;
	ulRet = (pIrpStack->Parameters.Create.Options & FILE_ATTRIBUTE_DIRECTORY) ? MFOBJ_DIR : MFOBJ_NONE;
	return ulRet;
}


ULONG   
ISFileWithFileObject( IN PIO_STACK_LOCATION pIrpStack, IN PFLT_EXTENSION pDevExt, IN PFILE_OBJECT pFileObject )
{
	ULONG      ulRet = MFOBJ_NONE;
	NTSTATUS   Status  =  STATUS_SUCCESS;
	FILE_BASIC_INFORMATION  FileBasic = {0};
	/*********************************************************************************************************/
	if(!pIrpStack || !pDevExt || !pFileObject)  
	{
		return MFOBJ_NONE;
	}	
	/*********************************************************************************************************/
	if(KeGetCurrentIrql() > APC_LEVEL)  
	{
		return MFOBJ_NONE;
	}

	Status = PreQueryFile( pDevExt->pLowerDeviceObject, pFileObject, &FileBasic, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation );
	if(NT_SUCCESS( Status ))
	{
		ulRet = (FileBasic.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? MFOBJ_DIR : MFOBJ_FILE;
	}
	else
	{
		ulRet = MFOBJ_NONE;
	}
	return ulRet;
}


BOOLEAN  
ISQueryDirectory( IN PDEVICE_OBJECT pDeviceObject, IN PFILE_OBJECT pFileObject, PFILE_STANDARD_INFORMATION  pFileStd /*= NULL */ )
{
	NTSTATUS  Status = STATUS_SUCCESS;

	FILE_STANDARD_INFORMATION    FileStd;
	RtlZeroMemory( &FileStd, sizeof(FileStd) );
	
	if(pFileStd)
	{
		Status = PreQueryFile( pDeviceObject, pFileObject, pFileStd, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation );
		if(NT_SUCCESS( Status ))
		{
			if(pFileStd->Directory) return TRUE;
		}
	}
	else
	{
		Status = PreQueryFile( pDeviceObject, pFileObject, &FileStd, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation );
		if(NT_SUCCESS( Status ))
		{
			if(FileStd.Directory) return TRUE;
		}
	}
	return FALSE;
}




NTSTATUS  
PostQueryFileComplete( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext)
{


	if(pIrp) *pIrp->UserIosb = pIrp->IoStatus;

	if(pContext) KeSetEvent( pContext, 0, FALSE);

	if(pIrp) IoFreeIrp(pIrp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS  
PreQueryFile( IN PDEVICE_OBJECT  pDeviceObject, 
			  IN PFILE_OBJECT    pFileObject,			 
			  IN PVOID           FileQueryBuffer,
			  IN ULONG           FileQueryBufferLength,
			  IN FILE_INFORMATION_CLASS  FileInfoClass )
{
	PIRP                pIrp = NULL;
	PIO_STACK_LOCATION  pIrpSP = NULL;
	IO_STATUS_BLOCK     IoStatus = {0};
	NTSTATUS            Status = STATUS_SUCCESS;
	KEVENT              Event;
	
	IoStatus.Status = STATUS_SUCCESS;
	IoStatus.Information = 0;

	pIrp = IoAllocateIrp( pDeviceObject->StackSize, FALSE );
	if(!pIrp) return STATUS_INSUFFICIENT_RESOURCES;

	pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
	pIrp->RequestorMode = KernelMode;
	pIrp->UserIosb = &IoStatus;
	pIrp->UserEvent = NULL;        
	pIrp->Flags = IRP_SYNCHRONOUS_API;
	pIrp->AssociatedIrp.SystemBuffer = FileQueryBuffer;
	pIrpSP = IoGetNextIrpStackLocation(pIrp);
	pIrpSP->MajorFunction = IRP_MJ_QUERY_INFORMATION;
	pIrpSP->FileObject = pFileObject;
	pIrpSP->Parameters.QueryFile.Length = FileQueryBufferLength;
	pIrpSP->Parameters.QueryFile.FileInformationClass = FileInfoClass;

	RtlZeroMemory( &Event, sizeof(Event) );
	KeInitializeEvent( &Event, SynchronizationEvent, FALSE );
	IoSetCompletionRoutine(	pIrp, 
							PostQueryFileComplete, 
							&Event, 
							TRUE, 
							TRUE, 
							TRUE );
	Status = IoCallDriver( pDeviceObject, pIrp );
	if(STATUS_PENDING == Status) 
	{
		KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );
	}
	return IoStatus.Status;

}




/****************************************************************************************************************************************/



NTSTATUS  
PostSetFileComplete( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, PVOID pContext )
{			
		
	if(pIrp) *pIrp->UserIosb = pIrp->IoStatus;		

	if(pContext) KeSetEvent( pContext, 0, FALSE );

	if(pIrp) IoFreeIrp( pIrp );	

    return STATUS_MORE_PROCESSING_REQUIRED;

}



NTSTATUS 
PreSetFile( IN PDEVICE_OBJECT  pDeviceObject, 
		    IN PFILE_OBJECT    pFileObject,
			IN PVOID           FileSetBuffer,
			IN ULONG           FileSetBufferLength,
			IN FILE_INFORMATION_CLASS FileInfoClass )
{
	PIRP                pNewIrp  = NULL;
	IO_STATUS_BLOCK     IoStatus = {0};
	NTSTATUS            Status   = STATUS_SUCCESS;
	KEVENT              Event    = {0};
	PIO_STACK_LOCATION  pIrpSP   = NULL;

	IoStatus.Status = STATUS_SUCCESS;
	IoStatus.Information = 0;

	pNewIrp = IoAllocateIrp( pDeviceObject->StackSize, FALSE );
	if(!pNewIrp) return STATUS_INSUFFICIENT_RESOURCES;
	
	pNewIrp->UserEvent     = NULL;
	pNewIrp->RequestorMode = KernelMode;
	pNewIrp->UserEvent = NULL;
	pNewIrp->UserIosb  = &IoStatus;
	pNewIrp->Flags     = IRP_SYNCHRONOUS_API;
	pNewIrp->AssociatedIrp.SystemBuffer = FileSetBuffer;
	pNewIrp->Tail.Overlay.OriginalFileObject = pFileObject;
	pNewIrp->Tail.Overlay.Thread = PsGetCurrentThread();

	pIrpSP = IoGetNextIrpStackLocation( pNewIrp );
	pIrpSP->DeviceObject  = pDeviceObject;
	pIrpSP->FileObject    = pFileObject;	
	pIrpSP->MajorFunction = IRP_MJ_SET_INFORMATION;
	pIrpSP->Parameters.SetFile.Length = FileSetBufferLength;
	pIrpSP->Parameters.SetFile.FileInformationClass = FileInfoClass;
	
	RtlZeroMemory( &Event, sizeof(Event) );
	KeInitializeEvent( &Event, SynchronizationEvent , FALSE );
	IoSetCompletionRoutine( pNewIrp, 
							PostSetFileComplete,
							&Event, 
							TRUE, 
							TRUE, 
							TRUE );				
	Status = IoCallDriver( pDeviceObject, pNewIrp );	
	if(Status == STATUS_PENDING) 
	{
		KeWaitForSingleObject( &Event, Executive, KernelMode, TRUE, 0 );
	}			
	return IoStatus.Status;
}

/****************************************************************************************************************************************/


/*********************************************************************************************************************************/
// Encrypt & Decrypt Block-Start
/*********************************************************************************************************************************/


// IRP_MJ_Close COMPLETION
NTSTATUS  
File_PostClose( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext )
{
	BOOLEAN               bExist      = FALSE, bUsbEncrypt = FALSE;
	NTSTATUS              Status      = STATUS_SUCCESS;
	PIO_STACK_LOCATION    pIrpStack   = NULL;
	PFILE_OBJECT          pFileObject = NULL;
	PFLT_EXTENSION        pDevExt     = NULL;
	NAME_BUFFER           FullPath    = {0};
	int                   nReturn     = 0;
	KIRQL                 OldIrql     = 0;	
	PFSRTL_COMMON_FCB_HEADER  pFcb = NULL;
/**********************************************************************************************************/
	Status = pIrp->IoStatus.Status;
	if(pIrp->PendingReturned)  
	{
		IoMarkIrpPending( pIrp );
	}

	if(!pDeviceObject) return Status;
	
	pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
	if(!pIrpStack) return Status;

	pFileObject  = pIrpStack->FileObject;	
	if(!pFileObject) return Status;

	pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;
	if(!pFcb) return Status;

	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	if(!pDevExt) return Status;

	return Status;
}


// IRP_MJ_CREATE COMPLETION
NTSTATUS  
File_PostCreate( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext )
{		
	BOOLEAN                   bOutDataEofWrite=FALSE;
	BOOLEAN                   bOutTailEofWrite=FALSE;
	BOOLEAN                   bExist      = FALSE, bUsbEncrypt = FALSE, bOutFileEncrypt = FALSE;
	NTSTATUS                  Status      = STATUS_SUCCESS;
	PIO_STACK_LOCATION        pIrpStack   = NULL;
	PFILE_OBJECT              pFileObject = NULL;
	PFLT_EXTENSION            pDevExt     = NULL;
	NAME_BUFFER               FullPath    = {0};
	int                       nReturn     = -1;
	KIRQL                     OldIrql     = 0;
	PFSRTL_COMMON_FCB_HEADER  pFcb        = NULL;
	PSECTION_OBJECT_POINTERS  pSecObjPtr  = NULL;
	IO_STATUS_BLOCK			  IoStatus    = {0};
	
/**********************************************************************************************************/

	Status = pIrp->IoStatus.Status;
	if(pIrp->PendingReturned)  
	{
		IoMarkIrpPending( pIrp );
	}
	if(!NT_SUCCESS( pIrp->IoStatus.Status )) return Status;
	
	pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
	if(!pIrpStack) return Status;

	pFileObject = pIrpStack->FileObject;	
	if(!pFileObject || pFileObject->Flags & FO_STREAM_FILE) return Status;

	pFcb = pFileObject->FsContext;
	if(!pFcb) return Status;
	
	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	if(!pDevExt) return Status;

	ALLOCATE_N_POOL( FullPath );
	if(!FullPath.pBuffer)  return Status;
	SET_POOL_ZERO( FullPath );

	nReturn = GetFileFullPath( pDeviceObject, pFileObject, pDevExt, &FullPath );
	if(nReturn <= 0) 
	{
		FREE_N_POOL( FullPath );
		return Status;
	}	

	FREE_N_POOL( FullPath );
	return Status;
}




// 확장자제어 Post
NTSTATUS  
File_PostCreate_FileExtCtrl( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext )
{
	ULONG                ulAccess=0, ulDisPos = 0;
	BOOLEAN              bExist      = FALSE;
	NTSTATUS             Status      = STATUS_SUCCESS;
	PIO_STACK_LOCATION   pIrpStack   = NULL;
	PFILE_OBJECT         pFileObject = NULL;
	PFLT_EXTENSION       pDevExt     = NULL;
	NAME_BUFFER          FullPath    = {0};
	int                  nReturn     = -1;
	WCHAR                wzProcName[ MAX_PROCESS_LEN ] = L"";
	LOG_HDR              LogHdr  = {0};
	
/**********************************************************************************************************/

	Status = pIrp->IoStatus.Status;
	if(pIrp->PendingReturned)  
	{
		IoMarkIrpPending( pIrp );
	}
	if(!NT_SUCCESS( pIrp->IoStatus.Status )) return Status;
	
	pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
	if(!pIrpStack) return Status;

	pFileObject = pIrpStack->FileObject;	
	if(!pFileObject || pFileObject->Flags & FO_STREAM_FILE) return Status;
	if(!pFileObject->FsContext) return Status;
	
	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	if(!pDevExt) return Status;

	ALLOCATE_N_POOL( FullPath );
	if(!FullPath.pBuffer)  return Status;
	SET_POOL_ZERO( FullPath );

	nReturn = GetFileFullPath( pDeviceObject, pFileObject, pDevExt, &FullPath );
	if(nReturn <= 0) 
	{
		FREE_N_POOL( FullPath );
		return Status;
	}	

/*************************************************************************************************************************/
/*************************************************************************************************************************/

	RtlZeroMemory( wzProcName, sizeof(WCHAR)*MAX_PROCESS_LEN );
	GetProcessName( PsGetCurrentProcess(), wzProcName, MAX_PROCESS_LEN  );

	ulDisPos = (pIrpStack->Parameters.Create.Options >> 24) & 0xFF;	
	if(pIrpStack->Parameters.Create.SecurityContext)
	{
		ulAccess = GetAccessMode( pIrpStack->Parameters.Create.SecurityContext->DesiredAccess );
	}

	LogHdr.ulLogID     = MFLOG_DLP;
	LogHdr.ulAccess    = ulAccess;
	LogHdr.ulProcessID = (ULONG)PsGetCurrentProcessId();
	LogHdr.ulTotalSize = 0;

	FullPath.ulFile = ISFileWithFileObject( pIrpStack, pDevExt, pFileObject );		
	if(FullPath.ulFile == MFOBJ_FILE)
	{
		Status = Control_F_FileExt( wzProcName, &FullPath, ulDisPos, ulAccess, &LogHdr );
		if(Status == STATUS_ACCESS_DENIED)
		{
			IoCancelFileOpen( pDevExt->pLowerDeviceObject, pFileObject );
			if(ISLogWritePolicy( LogHdr.ulLogID, FullPath.pBuffer[0] ))
			{
				LOG_Add( &LogHdr, wzProcName, &FullPath );
			}
			pIrp->IoStatus.Status = STATUS_ACCESS_DENIED;
			pIrp->IoStatus.Information = 0;
			FREE_N_POOL( FullPath );
			return STATUS_ACCESS_DENIED;
		}
	}
	
	FREE_N_POOL( FullPath );
	return Status;
}





/*********************************************************************************************************************************/
/*********************************************************************************************************************************/

// 캐싱된 데이터를 Flush 한다.
BOOLEAN  
File_FlushCacheRequest( IN PFILE_OBJECT pFileObject, IN PWCHAR pwzFullPath )
{
	IO_STATUS_BLOCK           IoStatus = {0};
	PFSRTL_COMMON_FCB_HEADER  pFcb = NULL;

	if(!pFileObject) return FALSE;

	pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;
	if(!pFcb) return FALSE;

	if(!pFileObject->SectionObjectPointer) 
	{
		return FALSE;
	}

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
	{
		return FALSE;
	}
	
	if(pFcb->Resource || pFcb->PagingIoResource)
	{
		KeEnterCriticalRegion();

		if(pFcb->Resource)         ExAcquireResourceExclusiveLite( pFcb->Resource, TRUE );
		if(pFcb->PagingIoResource) ExAcquireResourceExclusiveLite( pFcb->PagingIoResource, TRUE );

		CcFlushCache( pFileObject->SectionObjectPointer, 0, 0, &IoStatus );
		if(pFileObject->SectionObjectPointer->ImageSectionObject)
		{	
			MmFlushImageSection(pFileObject->SectionObjectPointer, MmFlushForWrite );
		}

		if(pFileObject->SectionObjectPointer->DataSectionObject)
		{	
			CcPurgeCacheSection(pFileObject->SectionObjectPointer, NULL, 0, FALSE );
		}

		//MmForceSectionClosed( pFileObject->SectionObjectPointer, TRUE );
		if(pFcb->PagingIoResource) ExReleaseResourceLite( pFcb->PagingIoResource );
		if(pFcb->Resource)         ExReleaseResourceLite( pFcb->Resource );

		KeLeaveCriticalRegion();

		// KdPrint(("[성공] File_FlushCacheRequest >> FilePath=%ws \n", pwzFullPath ));
		return TRUE;
	}

	return FALSE;
}



// 캐싱된 데이터를 Flush 한다.
BOOLEAN  
File_FlushCacheRequest_NoWait( IN PFILE_OBJECT pFileObject )
{
	BOOLEAN   bWaitRes=FALSE, bWaitPageIoRes=FALSE;
	IO_STATUS_BLOCK           IoStatus = {0};
	PFSRTL_COMMON_FCB_HEADER  pFcb = NULL;

	if(!pFileObject) return FALSE;

	pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;
	if(!pFcb) return FALSE;

	if(!pFileObject->SectionObjectPointer)
	{
		return FALSE;
	}

	if(pFcb->Resource || pFcb->PagingIoResource)
	{
		KeEnterCriticalRegion();

		if(pFcb->Resource) 
		{
			bWaitRes = ExAcquireResourceExclusiveLite( pFcb->Resource, FALSE );
			if(!bWaitRes)
			{
				KeLeaveCriticalRegion();
				return FALSE;
			}
		}
		if(pFcb->PagingIoResource) 
		{
			bWaitPageIoRes = ExAcquireResourceExclusiveLite( pFcb->PagingIoResource, FALSE );
			if(!bWaitPageIoRes)
			{
				if(pFcb->Resource) 
				{
					if(bWaitRes) ExReleaseResourceLite( pFcb->Resource );
				}
				KeLeaveCriticalRegion();
				return FALSE;
			}
		}

		CcFlushCache( pFileObject->SectionObjectPointer, 0, 0, &IoStatus );
		if(pFileObject->SectionObjectPointer->ImageSectionObject)
		{	
			MmFlushImageSection(pFileObject->SectionObjectPointer, MmFlushForWrite );
		}
		if(pFileObject->SectionObjectPointer->DataSectionObject)
		{	
			CcPurgeCacheSection(pFileObject->SectionObjectPointer, NULL, 0, FALSE );
		}
		//MmForceSectionClosed( pFileObject->SectionObjectPointer, TRUE );

		if(pFcb->PagingIoResource) 
		{
			if(bWaitPageIoRes) ExReleaseResourceLite( pFcb->PagingIoResource );
		}

		if(pFcb->Resource) 
		{
			if(bWaitRes) ExReleaseResourceLite( pFcb->Resource );
		}
		KeLeaveCriticalRegion();
		// KdPrint(("[성공] File_FlushCacheRequest_NoWait >> FilePath=%ws \n", pwzFullPath ));
		return TRUE;
	}

	return FALSE;
}



NTSTATUS   
File_PreRead( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
	KIRQL               OldIrql     = 0;
	ULONG               ulTotalFileSize=0, ulBlockFileSize=0;
	BYTE*               pBuffer     = NULL;
	BOOLEAN             bExist      = FALSE, bUsbEncrypt = FALSE;
	NTSTATUS            Status      = STATUS_SUCCESS;	
	PIO_STACK_LOCATION  pIrpStack   = NULL;
	PFILE_OBJECT        pFileObject = NULL;
	PFLT_EXTENSION      pDevExt     = NULL;
	
	NAME_BUFFER         FullPath    = {0};
	NAME_BUFFER         ProcPath    = {0};

	HANDLE              hProcessID  = NULL;
	ULONG               ulFileSize  = 0, nRet=0, ulAccessMode=0;	
	LOG_HDR             LogHdr = {0};
	PLOG_RECORD         pLogRecord = NULL;
	PFSRTL_COMMON_FCB_HEADER  pFcb = NULL;
	ULONG               ulPaddingSize = 0, ulOutOrgFileSize = 0;
	PIRP                pNewIrp = NULL;
	BYTE*               pNewReadBuffer = NULL;
	BOOLEAN             bOutEncrypt=FALSE, bCheckEnc=FALSE, bSuc=FALSE;
	CONTEXT_READ        ReadCtx  = {0};
	ENCRYPT_CTX         FileCtx  = {0};
	IO_STATUS_BLOCK     IoStatus = {0};

	if(g_MalwFind.pDeviceObject == pDeviceObject) 
	{
		return CompleteRequest( pIrp, STATUS_SUCCESS, 0 );
	}

	Status = VALID_CHECK( pDeviceObject, pIrp, &pDevExt, &pIrpStack );
	if(!NT_SUCCESS( Status ) || !pIrpStack) 
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> File_PreRead \n"));
		return CompleteRequest( pIrp, Status, 0 );
	}
	
	pFileObject = pIrpStack->FileObject;
	if(!pFileObject || (pFileObject->Flags & FO_STREAM_FILE)) 
	{
		return CallDriverSkip( pDevExt, pIrp );
	}

	pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;
	if(!pFcb) 
	{
		return CallDriverSkip( pDevExt, pIrp );
	}
	ulFileSize = pFcb->FileSize.LowPart;
	

	ALLOCATE_N_POOL_PROC( ProcPath );
	if(!ProcPath.pBuffer)  return CallDriverSkip( pDevExt, pIrp );
	SET_POOL_ZERO_PROC( ProcPath );
	GetProcessName( PsGetCurrentProcess(), ProcPath.pBuffer, MAX_PROCESS_LEN );	


	ulAccessMode |= MALWF_READ;
	// 전역기능 예외 프로세스 판단
	if(ISExpProcList_PolicyAll( ProcPath.pBuffer )) 
	{
		if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
		return CallDriverSkip( pDevExt, pIrp );
	}

	hProcessID = PsGetCurrentProcessId();	
	LogHdr.ulLogID  = MFLOG_NONE;
	LogHdr.ulAccess = ulAccessMode;
	LogHdr.ulTotalSize = sizeof(LOG_HDR);

	ALLOCATE_N_POOL( FullPath );
	if(!FullPath.pBuffer) 
	{
		if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
		return CallDriverSkip( pDevExt, pIrp );
	}
	SET_POOL_ZERO( FullPath );

	nRet = GetFileFullPath( pDeviceObject, pFileObject, pDevExt, &FullPath );	
	if(nRet <= 0)
	{
		Status = CallDriverSkip( pDevExt, pIrp );

		if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
		if(FullPath.pBuffer) FREE_N_POOL( FullPath );
		return Status;
	}		

	Status = CallDriverSkip( pDevExt, pIrp );

	if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
	if(FullPath.pBuffer) FREE_N_POOL( FullPath );
	return Status;
}






BOOLEAN     
SetFileUpdate_AllocSize( IN PFILE_OBJECT pFileObject, IN PFSRTL_COMMON_FCB_HEADER pFcb )
{
	ULONG  ulDiff=0, ulAllocSize=0, ulFileSize=0;	

	if(!pFileObject || !pFcb) return FALSE;

	ulFileSize  = pFcb->FileSize.LowPart;
	ulAllocSize = pFcb->AllocationSize.LowPart;
	if(ulAllocSize >= ulFileSize)
	{
		ulDiff = (ulAllocSize - ulFileSize);
		if(ulDiff >= 256) return FALSE;  // 파일업데이트 대상이 아니다.
	}
	else
	{
		ulDiff = (ulFileSize - ulAllocSize);
		if(ulDiff > 0) ulAllocSize += ulDiff;
	}
	
	ulAllocSize = ROUND_TO_PAGES(ulAllocSize + 256);
	if(ulAllocSize > pFcb->AllocationSize.LowPart)
	{
		pFcb->AllocationSize.LowPart = ulAllocSize;

		if(pFileObject->PrivateCacheMap != NULL)
		{
			CcSetFileSizes( pFileObject, (PCC_FILE_SIZES)&(pFcb->AllocationSize) );
		}
		return TRUE;
	}

	return FALSE;
}


BOOLEAN     
SetFileUpdate_AllocSizeEx( IN PDEVICE_OBJECT pDeviceObject, IN PFILE_OBJECT pFileObject, IN PFSRTL_COMMON_FCB_HEADER pFcb  )
{
	ULONG     ulDiff=0, ulFileSize=0, ulAllocSize=0;
	NTSTATUS  Status = STATUS_SUCCESS;
	FILE_ALLOCATION_INFORMATION  FileAllocInfo;
	RtlZeroMemory( &FileAllocInfo, sizeof(FileAllocInfo) );

	if(!pDeviceObject || !pFileObject)
	{
		KdPrint(("[ Fail ] SetFileUpdate_AllocSizeEx FsContext0x%08x Parameter Null Pointer. \n", pFileObject->FsContext ));
		return FALSE;
	}

	ulFileSize  = pFcb->FileSize.LowPart;
	ulAllocSize = pFcb->AllocationSize.LowPart;
	if(ulAllocSize >= ulFileSize)
	{
		ulDiff = (ulAllocSize - ulFileSize);
		if(ulDiff >= 256) return FALSE;  // 파일업데이트 대상이 아니다.
	}
	else
	{
		ulDiff = (ulFileSize - ulAllocSize);
		if(ulDiff > 0) ulAllocSize += ulDiff;
	}

	ulAllocSize = ROUND_TO_PAGES(ulAllocSize + 256);
	if(ulAllocSize > pFcb->AllocationSize.LowPart)
	{
		pFcb->AllocationSize.LowPart = ulAllocSize;

		FileAllocInfo.AllocationSize.HighPart = 0;
		FileAllocInfo.AllocationSize.LowPart  = ulAllocSize;

		Status = PreSetFile( pDeviceObject, pFileObject, &FileAllocInfo, sizeof(FileAllocInfo), FileAllocationInformation );
		if(!NT_SUCCESS( Status ))
		{
			KdPrint(("[ Fail ] SetFileUpdate_AllocSizeEx FsContext0x%08x, Status = 0x%08x \n", pFileObject->FsContext, Status ));
			return FALSE;
		}

		if(pFileObject->PrivateCacheMap != NULL)
		{
			CcSetFileSizes( pFileObject, (PCC_FILE_SIZES)&(pFcb->AllocationSize) );
		}
	}

	return TRUE;
}


BOOLEAN 
SetFileUpdate_EndOfFileSize( IN PDEVICE_OBJECT pDeviceObject, IN PFILE_OBJECT pFileObject, IN LARGE_INTEGER ullEndOfFile )
{
	NTSTATUS  Status = STATUS_SUCCESS;
	FILE_END_OF_FILE_INFORMATION  FileEndInfo;
	RtlZeroMemory( &FileEndInfo, sizeof(FileEndInfo) );

	if(!pDeviceObject || !pFileObject)
	{
		KdPrint(("[ Fail ] SetFileUpdate_EndOfFileSize FsContext0x%08x Parameter Null Pointer. \n", pFileObject->FsContext ));
		return FALSE;
	}

	FileEndInfo.EndOfFile = ullEndOfFile;

	Status = PreSetFile( pDeviceObject, pFileObject, &FileEndInfo, sizeof(FileEndInfo), FileEndOfFileInformation );
	if(!NT_SUCCESS( Status ))
	{
		KdPrint(("[ Fail ] SetFileUpdate_EndOfFileSize FsContext0x%08x, Status = 0x%08x \n", pFileObject->FsContext, Status ));
		return FALSE;
	}

	return TRUE;

}




NTSTATUS   
File_PreWrite( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{		
	PIRP                pNewIrp     = NULL;
	BYTE*               pBuffer     = NULL;
	BOOLEAN             bExist      = FALSE, bUsbEncrypt=FALSE, bOutEncrypt=FALSE;
	KIRQL               OldIrql     = 0;	
	NTSTATUS            Status      = STATUS_SUCCESS;	
	PIO_STACK_LOCATION  pIrpStack   = NULL;
	PFILE_OBJECT        pFileObject = NULL;
	PFLT_EXTENSION      pDevExt     = NULL;
	
	NAME_BUFFER         FullPath    = {0};
	NAME_BUFFER         ProcPath    = {0};

	HANDLE              hProcessID  = NULL;
	ULONG               nRet =0, ulAccessMode = 0;
	ULONG               ulTotalFileSize=0, ulBlockFileSize=0, ulMaxBufSize=0;
	ULONG               ulFileSize=0, ulFileOffset=0, ulWriteLength=0, ulRealWriteLength=0;
	LARGE_INTEGER       ullByteOffset = {0};
	LOG_HDR             LogHdr = {0};
	PLOG_RECORD         pLogRecord = NULL;
	PFSRTL_COMMON_FCB_HEADER  pFcb = NULL;
	CONTEXT_WRITE       WriteCtx;

	if(g_MalwFind.pDeviceObject == pDeviceObject) 
	{
		return CompleteRequest( pIrp, STATUS_SUCCESS, 0 );	
	}

	Status = VALID_CHECK( pDeviceObject, pIrp, &pDevExt, &pIrpStack );
	if(!NT_SUCCESS( Status ) || !pIrpStack)  
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> File_PreWrite \n"));
		return CompleteRequest( pIrp, Status, 0 );
	}

	pFileObject = pIrpStack->FileObject;
	if(!pFileObject || (pFileObject->Flags & FO_STREAM_FILE)) 
	{
		return CallDriverSkip( pDevExt, pIrp );
	}

	pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;
	if(!pFcb)return CallDriverSkip( pDevExt, pIrp );
	

	ALLOCATE_N_POOL_PROC( ProcPath );
	if(!ProcPath.pBuffer)  return CallDriverSkip( pDevExt, pIrp );
	SET_POOL_ZERO_PROC( ProcPath );
	GetProcessName( PsGetCurrentProcess(), ProcPath.pBuffer, MAX_PROCESS_LEN );


	ulAccessMode |= MALWF_WRITE;
	// 전역기능 예외 프로세스 판단
	if(ISExpProcList_PolicyAll( ProcPath.pBuffer )) 
	{
		if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
		return CallDriverSkip( pDevExt, pIrp );
	}

	hProcessID = PsGetCurrentProcessId();
	LogHdr.ulLogID     = MFLOG_NONE;
	LogHdr.ulAccess    = ulAccessMode;
	LogHdr.ulTotalSize = 0; 

	ALLOCATE_N_POOL( FullPath );
	if(!FullPath.pBuffer) 
	{
		if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
		return CallDriverSkip( pDevExt, pIrp );
	}
	SET_POOL_ZERO( FullPath );

	nRet = GetFileFullPath( pDeviceObject, pFileObject, pDevExt, &FullPath );
	if(nRet <= 0)
	{
		Status = CallDriverSkip( pDevExt, pIrp );

		if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
		if(FullPath.pBuffer) FREE_N_POOL( FullPath );
		return Status;
	}

	Status = CallDriverSkip( pDevExt, pIrp );	

	if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
	if(FullPath.pBuffer) FREE_N_POOL( FullPath );
	return Status;
}


// IRP_MJ_CLEANUP  IRP_MJ_CLOSE
NTSTATUS File_PreClose( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
	int                 nRet = 0;
	BYTE*               pBuffer     = NULL;
	ULONG               ulTotalFileSize=0, ulBlockFileSize=0;
	BOOLEAN             bExist      = FALSE, bUsbEncrypt = FALSE;
	NTSTATUS            Status      = STATUS_SUCCESS;	
	KIRQL               OldIrql     = 0;
	NAME_BUFFER         FullPath    = {0};
	PIO_STACK_LOCATION  pIrpStack   = NULL;
	PFILE_OBJECT        pFileObject = NULL;
	PFLT_EXTENSION      pDevExt     = NULL;
	PFSRTL_COMMON_FCB_HEADER  pFcb = NULL;

	if(g_MalwFind.pDeviceObject == pDeviceObject) 
	{
		return CompleteRequest( pIrp, STATUS_SUCCESS, 0 );
	}

	Status = VALID_CHECK( pDeviceObject, pIrp, &pDevExt, &pIrpStack );
	if(!NT_SUCCESS( Status ) || !pIrpStack) 
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> File_PreClose \n"));
		return CompleteRequest( pIrp, Status, 0 );
	}
	
	pFileObject = pIrpStack->FileObject;
	if(!pFileObject || (pFileObject->Flags & FO_STREAM_FILE) )
	{
		return CallDriverSkip( pDevExt, pIrp );
	}

	pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;
	if(!pFcb) return CallDriverSkip( pDevExt, pIrp );

	ALLOCATE_N_POOL( FullPath );
	if(!FullPath.pBuffer)  return CallDriverSkip( pDevExt, pIrp );
	SET_POOL_ZERO( FullPath );

	nRet = GetFileFullPath( pDeviceObject, pFileObject, pDevExt, &FullPath );
	if(nRet <= 0)
	{
		Status = CallDriverSkip( pDevExt, pIrp );
		if(FullPath.pBuffer) FREE_N_POOL( FullPath );
		return Status;
	}

	Status = CallDriverSkip( pDevExt, pIrp );
	if(FullPath.pBuffer) FREE_N_POOL( FullPath );
	return Status;

}


// IRP_MJ_CREATE
NTSTATUS  
File_PreCreate( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
	BOOLEAN               bExist      = FALSE, bUsbEncrypt = FALSE;
	KIRQL                 OldIrql     = 0;
	NTSTATUS              Status      = STATUS_SUCCESS;	
	PIO_STACK_LOCATION    pIrpStack   = NULL;
	PFILE_OBJECT          pFileObject = NULL;
	PFLT_EXTENSION        pDevExt     = NULL;
	NAME_BUFFER           FullPath    = {0};
	NAME_BUFFER           ProcPath    = {0};
	
	ACCESS_MASK           ulDesiredAccess = 0;
	PIO_SECURITY_CONTEXT  pIoSecuContext  = NULL;
	ULONG                 ulDispos=0, ulRet=0, ulVolFlag=0;	
	ULONG                 ulFileSize=0, ulAccessMode=0, ulOption=0;
	HANDLE                hProcessID  = NULL;
	PFSRTL_COMMON_FCB_HEADER  pFcb = NULL;
	LOG_HDR               LogHdr  = {0};

	if(g_MalwFind.pDeviceObject == pDeviceObject) return CompleteRequest( pIrp, STATUS_SUCCESS, 0 );

	Status = VALID_CHECK( pDeviceObject, pIrp, &pDevExt, &pIrpStack );
	if(!NT_SUCCESS( Status ) || !pIrpStack) 
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> File_PreCreate \n"));
		return CompleteRequest( pIrp, Status, 0 );
	}


	pFileObject = pIrpStack->FileObject;
	if(!pFileObject || (pFileObject->Flags & FO_STREAM_FILE)) 
	{
		return CallDriverSkip( pDevExt, pIrp );
	}
	

	hProcessID = PsGetCurrentProcessId();
	pIoSecuContext = pIrpStack->Parameters.Create.SecurityContext;
	if(pIoSecuContext) ulDesiredAccess = pIoSecuContext->DesiredAccess;


	ALLOCATE_N_POOL_PROC( ProcPath );
	if(!ProcPath.pBuffer)  
	{
		return CallDriverSkip( pDevExt, pIrp );
	}
	SET_POOL_ZERO_PROC( ProcPath );
	GetProcessName( PsGetCurrentProcess(), ProcPath.pBuffer, MAX_PROCESS_LEN );	


	ulDispos = (pIrpStack->Parameters.Create.Options >> 24) & 0xFF;	
	// 전역기능 예외프로세스 판단
	if(ISExpProcList_PolicyAll( ProcPath.pBuffer ))
	{
		if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
		return CallDriverSkip( pDevExt, pIrp );
	}

	ulAccessMode = GetAccessMode( ulDesiredAccess );
	if(ulDispos == FILE_OPEN || ulDispos == FILE_OPEN_IF) 
	{
		ulAccessMode |= MALWF_OPEN;
	}
	else
	{
		if(ulDispos == FILE_CREATE) ulAccessMode |= MALWF_CREATE;
		else if(ulDispos == FILE_SUPERSEDE || ulDispos == FILE_OVERWRITE || ulDispos == FILE_OVERWRITE_IF) ulAccessMode |= MALWF_CREATE;
	}	
	if(!ulAccessMode) ulAccessMode |= MALWF_READ;

	LogHdr.ulLogID     = MFLOG_NONE;
	LogHdr.ulAccess    = ulAccessMode;
	LogHdr.ulProcessID = (ULONG)hProcessID;
	LogHdr.ulTotalSize = 0;

	ALLOCATE_N_POOL( FullPath );
	if(!FullPath.pBuffer)  
	{
		if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
		return CallDriverSkip( pDevExt, pIrp );
	}
	SET_POOL_ZERO( FullPath );

	ulRet = GetFileFullPath( pDeviceObject, pFileObject, pDevExt, &FullPath );
	if(ulRet <= 0)
	{
		Status = CallDriverSkip( pDevExt, pIrp );

		if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
		if(FullPath.pBuffer) FREE_N_POOL( FullPath );
		return Status;
	}

	if(g_MalwFind.DrvConfig.bGlobalCtrl && g_MalwFind.DrvConfig.wzSetupDir[0] == FullPath.pBuffer[0])
	{	
		Status = Protect_SetupFolder_Control( ProcPath.pBuffer, &FullPath, ulAccessMode );
		if(Status == STATUS_ACCESS_DENIED)
		{
			Status = CompleteRequest( pIrp, Status, 0 );

			if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
			if(FullPath.pBuffer) FREE_N_POOL( FullPath );
			return Status;
		}		
	}
	
	if(ulRet >= 2)
	{	
		switch(ulDispos)
		{
		case FILE_OVERWRITE:    
		case FILE_SUPERSEDE:    
		case FILE_CREATE:       
		case FILE_OVERWRITE_IF: 				
		case FILE_OPEN_IF:  
			if(IS_NETWORK_DEVICE_TYPE( pDeviceObject->DeviceType ))
			{
				Status = Control_FP_SFolder( ProcPath.pBuffer, &FullPath, &ulDispos, &ulAccessMode, &LogHdr );				
			}
			else if(IS_VOLUME_DEVICE_TYPE( pDeviceObject->DeviceType ))
			{				
				Status = Control_FP_Volume( ProcPath.pBuffer, &FullPath, &ulDispos, &ulAccessMode, &LogHdr );		
				if(Status == STATUS_ACCESS_DENIED) break;
	
				Status = Control_F_FileExt( ProcPath.pBuffer, &FullPath, ulDispos, ulAccessMode, &LogHdr ); 
			}
			break;

		case FILE_OPEN:      	
			if(IS_NETWORK_DEVICE_TYPE( pDeviceObject->DeviceType ))
			{
				Status = Control_FP_SFolder( ProcPath.pBuffer, &FullPath, &ulDispos, &ulAccessMode, &LogHdr );				
			}
			else if(IS_VOLUME_DEVICE_TYPE( pDeviceObject->DeviceType ))
			{
				Status = Control_FP_Volume( ProcPath.pBuffer, &FullPath, &ulDispos, &ulAccessMode, &LogHdr );		
				if(Status == STATUS_ACCESS_DENIED) break;
				
				Status = Control_F_FileExt( ProcPath.pBuffer, &FullPath, ulDispos, ulAccessMode, &LogHdr ); 
			}
			break;

		} // End switch
	} // End if(NT_SUCCESS( Status ))	
	
	if(Status == STATUS_ACCESS_DENIED)
	{
		if(ISLogWritePolicy( LogHdr.ulLogID, FullPath.pBuffer[0] ))
		{
			LOG_Add( &LogHdr, ProcPath.pBuffer, &FullPath );
		}

		if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
		if(FullPath.pBuffer) FREE_N_POOL( FullPath );
		return CompleteRequest( pIrp, Status, 0 );
	}
	
	Status = CallDriverSkip( pDevExt, pIrp );

	if(ProcPath.pBuffer) FREE_N_POOL_PROC( ProcPath );
	if(FullPath.pBuffer) FREE_N_POOL( FullPath );
	return Status;

}


NTSTATUS  
File_PostQueryInformation( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext )
{
	ULONG                     ulOutPadding=0;
	BOOLEAN                   bExist      = FALSE, bUsbEncrypt = FALSE, bOutFileEncrypt=FALSE;
	NTSTATUS                  Status      = STATUS_SUCCESS;
	PIO_STACK_LOCATION        pIrpStack   = NULL;
	PFILE_OBJECT              pFileObject = NULL;
	PFLT_EXTENSION            pDevExt     = NULL;
	NAME_BUFFER               FullPath    = {0};
	int                       nReturn     = 0;
	FILE_INFORMATION_CLASS    FileInfoClass = {0};
	PFSRTL_COMMON_FCB_HEADER  pFcb        = NULL;
	/**********************************************************************************************************/

	Status = pIrp->IoStatus.Status;
	if(pIrp->PendingReturned)  
	{
		IoMarkIrpPending( pIrp );
	}

	if(!pDeviceObject) return Status;
	
	if(!NT_SUCCESS( pIrp->IoStatus.Status )) return Status;
	
	pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
	if(!pIrpStack) return Status;

	pFileObject = pIrpStack->FileObject;	
	if(!pFileObject) return Status;
	if(!pFileObject->FsContext) return Status;
	
	pDevExt = (PFLT_EXTENSION)pDeviceObject->DeviceExtension;
	if(!pDevExt) return Status;

	FileInfoClass = pIrpStack->Parameters.SetFile.FileInformationClass;
	pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;
	if(!pFcb) return Status;
	
	ALLOCATE_N_POOL( FullPath );
	if(!FullPath.pBuffer)  return Status;
	SET_POOL_ZERO( FullPath );

	nReturn = GetFileFullPath( pDeviceObject, pFileObject, pDevExt, &FullPath );
	if(nReturn <= 0) 
	{
		FREE_N_POOL( FullPath );
		return Status;
	}

	switch(FileInfoClass)
	{
	case FileStreamInformation:
		{
			PFILE_STREAM_INFORMATION pFileStream = (PFILE_STREAM_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;
			if(pFileStream) 
			{	
				KdPrint(( "[File_PostQuery]-[FileStreamInfo] >> StreamAllocation=%d StreamSize=%d FileSize=%d StreamName=%ws FileName=[%ws] \n", 
					pFileStream->StreamAllocationSize, pFileStream->StreamSize, pFcb->FileSize.LowPart, pFileStream->StreamName, FullPath.pBuffer ));
			}
		}
		break;
	case FileStandardInformation:  
		{
			PFILE_STANDARD_INFORMATION  pStdInfo = (PFILE_STANDARD_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;
			if(pStdInfo) 
			{								
				KdPrint(( "[File_PostQuery]-[FileStdInfo] >> Allocation=%d EndofFile=%d FileSize=%d [%ws] \n", pStdInfo->AllocationSize.LowPart, pStdInfo->EndOfFile.LowPart, pFcb->FileSize.LowPart, FullPath.pBuffer   ));
			}
		}
		break;
	case FileNetworkOpenInformation:
		{
			PFILE_NETWORK_OPEN_INFORMATION pFileNetOpen = (PFILE_NETWORK_OPEN_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;
			if(pFileNetOpen)
			{
				KdPrint(( "[File_PostQuery]-[FileNetworkOpenInfo] >> EndofFile=%d FileSize=%d [%ws] \n", pFileNetOpen->EndOfFile.LowPart, pFcb->FileSize.LowPart, FullPath.pBuffer   ));
			}
		}
		break;

	case FileAllInformation:
		{
			PFILE_ALL_INFORMATION  pFileAllInfo = (PFILE_ALL_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;
			if(pFileAllInfo)
			{
				KdPrint(( "[File_PostQuery]-[FileAllInfo] >> EndofFile=%d FileSize=%d [%ws] \n", pFileAllInfo->StandardInformation.EndOfFile.LowPart, pFcb->FileSize.LowPart, FullPath.pBuffer   ));
			}
		}
		break;

	case FileAttributeTagInformation:
		  {
			  PFILE_ATTRIBUTE_TAG_INFORMATION  pFileTagInfo = (PFILE_ATTRIBUTE_TAG_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;
			  if(pFileTagInfo)
			  {				  
				  KdPrint(( "[File_PostQuery]-[FileTagInfo] >> ReparseTag=%08x [%ws] \n", pFileTagInfo->ReparseTag, FullPath.pBuffer   ));
			  }
		  }
		  break;
	case FileBasicInformation:
		break;
	case FileNameInformation:
		break;

	default:
		{
			KdPrint(("[QueryComplete]-[FileInfoClass=%d] [%ws] \n", FileInfoClass, FullPath.pBuffer   ));		
		}
		break;
	}

	FREE_N_POOL( FullPath );
	return Status;
}


NTSTATUS   
File_PreQueryInfomation( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
	int                       nRet = 0;
	NTSTATUS                  Status = STATUS_SUCCESS;	
	PFSRTL_COMMON_FCB_HEADER  pFcb = NULL;
	PIO_STACK_LOCATION        pIrpStack   = NULL;
	PFILE_OBJECT              pFileObject = NULL;
	PFLT_EXTENSION            pDevExt     = NULL;		
	NAME_BUFFER               FullPath = {0};
	FILE_INFORMATION_CLASS    FileInfoClass  = {0};
	PFILE_STANDARD_INFORMATION pStdInfo = NULL;
	WCHAR                     wzProcName[ MAX_PROCESS_LEN ] = L"";
	ULONG                     ulFileSize=0;
	BOOLEAN                   bUsbEncrypt=FALSE, bOutFileEncrypt=FALSE;

	if(g_MalwFind.pDeviceObject == pDeviceObject) return CompleteRequest( pIrp, STATUS_SUCCESS, 0 );	

	Status = VALID_CHECK( pDeviceObject, pIrp, &pDevExt, &pIrpStack );
	if(!NT_SUCCESS( Status ) || !pIrpStack) 
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> File_PreQueryInfomation \n"));
		return CompleteRequest( pIrp, Status, 0 );
	}

	pFileObject = pIrpStack->FileObject;
	if(!pFileObject) return CallDriverSkip( pDevExt, pIrp );
	
	GetProcessName( PsGetCurrentProcess(), wzProcName, MAX_PROCESS_LEN  );
	
	FileInfoClass = pIrpStack->Parameters.SetFile.FileInformationClass;
	pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;
	if(pFcb) ulFileSize = pFcb->FileSize.LowPart;

	ALLOCATE_N_POOL( FullPath );
	if(!FullPath.pBuffer) return CallDriverSkip( pDevExt, pIrp );
	SET_POOL_ZERO( FullPath );

	nRet = GetFileFullPath( pDeviceObject, pFileObject, pDevExt, &FullPath );
	if(nRet <= 0)
	{		
		Status = CallDriverSkip( pDevExt, pIrp );
		if(FullPath.pBuffer) FREE_N_POOL( FullPath );
		return Status;
	}
		
	Status = CallDriverSkip( pDevExt, pIrp );
	if(FullPath.pBuffer) FREE_N_POOL( FullPath );
	return Status;
}




NTSTATUS   
File_PreSetInformation( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
	ULONG   nRet = 0;
	NTSTATUS                  Status      = STATUS_SUCCESS;	
	PIO_STACK_LOCATION        pIrpStack   = NULL;
	PFILE_OBJECT              pFileObject = NULL;
	PFLT_EXTENSION            pDevExt     = NULL;		
	NAME_BUFFER               FullPath = {0};
	NAME_BUFFER               FullPath_Dest = {0};
	KPROCESSOR_MODE           ucCurrentMode = 0;
	PFSRTL_COMMON_FCB_HEADER  pFcb = NULL;
	WCHAR                     wzProcName[ MAX_PROCESS_LEN ];
	HANDLE                    hProcessID  = NULL;
	ULONG                     ulFileSize  = 0, ulAccessMode=0;
	FILE_INFORMATION_CLASS         FileInfoClass  = {0};
	PFILE_RENAME_INFORMATION       pRenameInfo = NULL;
	PFILE_DISPOSITION_INFORMATION  pDeleteInfo = NULL;
	LOG_HDR                   LogHdr = {0};

	if(g_MalwFind.pDeviceObject == pDeviceObject) return CompleteRequest( pIrp, STATUS_SUCCESS, 0 );	
	
	Status = VALID_CHECK( pDeviceObject, pIrp, &pDevExt, &pIrpStack );
	if(!NT_SUCCESS( Status ) || !pIrpStack) 
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> File_PreSetInformation \n"));
		return CompleteRequest( pIrp, Status, 0 );
	}

	pFileObject = pIrpStack->FileObject;
	if(!pFileObject || pFileObject->Flags & FO_STREAM_FILE) 
	{
		return CallDriverSkip( pDevExt, pIrp );
	}

	GetProcessName( PsGetCurrentProcess(), wzProcName, MAX_PROCESS_LEN  );
	// 전역기능 예외 프로세스 판단
	if(ISExpProcList_PolicyAll( wzProcName ))
	{
		return CallDriverSkip( pDevExt, pIrp );
	}
	
	ulAccessMode |= MALWF_WRITE;
	ucCurrentMode = ExGetPreviousMode();
	hProcessID    = PsGetCurrentProcessId();

	LogHdr.ulLogID     = MFLOG_NONE;
	LogHdr.ulAccess    = ulAccessMode;
	LogHdr.ulProcessID = (ULONG)hProcessID;
	LogHdr.ulTotalSize = 0;

	FileInfoClass = pIrpStack->Parameters.SetFile.FileInformationClass;
	pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;
	if(pFcb) ulFileSize = pFcb->FileSize.LowPart;

	ALLOCATE_N_POOL( FullPath );
	if(!FullPath.pBuffer) return CallDriverSkip( pDevExt, pIrp );
	SET_POOL_ZERO( FullPath );

	ALLOCATE_N_POOL( FullPath_Dest );
	if(!FullPath_Dest.pBuffer)
	{
		Status = CallDriverSkip( pDevExt, pIrp );
		if(FullPath.pBuffer) FREE_N_POOL( FullPath );
		return Status;
	}
	SET_POOL_ZERO( FullPath_Dest );

	nRet = GetFileFullPath( pDeviceObject, pFileObject, pDevExt, &FullPath );
	if(nRet <= 0)
	{		
		Status = CallDriverSkip( pDevExt, pIrp );
		if(FullPath.pBuffer) FREE_N_POOL( FullPath );
		if(FullPath_Dest.pBuffer) FREE_N_POOL( FullPath_Dest );
		return Status;
	}
	
	switch(FileInfoClass)
	{
	case FileAllocationInformation:
		{
			ULONG ulOutPaddingSize=0;
			PFILE_ALLOCATION_INFORMATION  pAllocInfo = NULL;
			BOOLEAN  bUsbEncrypt=FALSE, bOutEncrypt=FALSE;
			pAllocInfo  = (PFILE_ALLOCATION_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;		
			if(pAllocInfo)
			{
				KdPrint(( "[ Set ]-[FileAllocation] >> AllocationSize=%d Proc=%ws FileName=%ws \n", pAllocInfo->AllocationSize.LowPart, wzProcName, FullPath.pBuffer ));				
			}
		}
		break;

	case FileEndOfFileInformation:
		{
			BOOLEAN  bOutDataWrite=FALSE, bOutTailWrite=FALSE;
			BOOLEAN  bUsbEncrypt=FALSE, bOutEncrypt=FALSE;
			ULONG    ulOutPaddingSize=0;
			PFILE_END_OF_FILE_INFORMATION  pEndOfFile = NULL;	

			pEndOfFile = (PFILE_END_OF_FILE_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;
			if(pEndOfFile)
			{					
				KdPrint(( "[Set]-[FileEndOfFile] >> EndOfFile=%d FileSize=%d [%ws] \n", pEndOfFile->EndOfFile.LowPart, pFcb->FileSize.LowPart, FullPath.pBuffer ));																				
			}
		}
		break;

	case FileValidDataLengthInformation:
		{
			PFILE_VALID_DATA_LENGTH_INFORMATION  pValidDataLength = (PFILE_VALID_DATA_LENGTH_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;
			if(pValidDataLength)
			{
				KdPrint(("[ Set ]-[ FileValidDataLength ] >> ValidDataLength=%d Proc=%ws FileName=%ws \n", pValidDataLength->ValidDataLength.LowPart, wzProcName, FullPath.pBuffer ));
			}
		}
		break;
	default: break;
	}

	switch(FileInfoClass)
	{
	case FileRenameInformation:  // 10  // File Rename
		pRenameInfo = (PFILE_RENAME_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;
		if(pRenameInfo)
		{		
			ulAccessMode = MALWF_RENAME;
			if(IS_NETWORK_DEVICE_TYPE( pDeviceObject->DeviceType ))
			{
				Status = Control_FP_SFolder( wzProcName, &FullPath, NULL, &ulAccessMode, &LogHdr );	
				if(STATUS_ACCESS_DENIED == Status) break;
				// Rename 대상이 제어대상인가?
				if(GetFileRenameDestPath( pIrpStack, pRenameInfo, &FullPath_Dest ))
				{				
					ulAccessMode |= MALWF_CREATE;
					Status = Control_FP_SFolder( wzProcName, &FullPath_Dest, NULL, &ulAccessMode, &LogHdr );	
				}
			}
			else if(IS_VOLUME_DEVICE_TYPE( pDeviceObject->DeviceType ))
			{
				if (g_MalwFind.DrvConfig.bGlobalCtrl && g_MalwFind.DrvConfig.wzSetupDir[0] == FullPath.pBuffer[0])
				{	
					Status = Protect_SetupFolder_Control( wzProcName, &FullPath, ulAccessMode );
					if(Status == STATUS_ACCESS_DENIED) break;					
				}

				Status = Control_FP_Volume( wzProcName, &FullPath, NULL, &ulAccessMode, &LogHdr );						
				if(STATUS_ACCESS_DENIED == Status) break;
				// 확장자 제어
				Status = Control_F_FileExt( wzProcName, &FullPath, 0, ulAccessMode, &LogHdr ); 
				if(STATUS_ACCESS_DENIED == Status) break;

				// Rename 대상이 제어대상인가?
				if(GetFileRenameDestPath( pIrpStack, pRenameInfo, &FullPath_Dest ))				
				{
					ulAccessMode |= MALWF_CREATE;
					Status = Control_FP_Volume( wzProcName, &FullPath_Dest, NULL, &ulAccessMode, &LogHdr );
					if(STATUS_ACCESS_DENIED == Status) break;
					// 확장자제어	
					Status = Control_F_FileExt( wzProcName, &FullPath_Dest, 0, ulAccessMode, &LogHdr ); 
				}
			}
		}
		break;

	case FileDispositionInformation: // 13 File Delete
		pDeleteInfo = (PFILE_DISPOSITION_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;
		if(pDeleteInfo && pDeleteInfo->DeleteFile == TRUE)
		{   						
			ulAccessMode |= MALWF_DELETE;
			if(IS_NETWORK_DEVICE_TYPE( pDeviceObject->DeviceType ))
			{
				Status = Control_FP_SFolder( wzProcName, &FullPath, NULL, &ulAccessMode, &LogHdr );				
			}
			else if(IS_VOLUME_DEVICE_TYPE( pDeviceObject->DeviceType ))
			{
				if (g_MalwFind.DrvConfig.bGlobalCtrl && g_MalwFind.DrvConfig.wzSetupDir[0] == FullPath.pBuffer[0])
				{	
					Status = Protect_SetupFolder_Control( wzProcName, &FullPath, ulAccessMode );
					if(Status == STATUS_ACCESS_DENIED) break;					
				}

				Status = Control_FP_Volume( wzProcName, &FullPath, NULL, &ulAccessMode, &LogHdr );		
				if(STATUS_ACCESS_DENIED == Status) break;
				// 확장자 제어
				Status = Control_F_FileExt( wzProcName, &FullPath, 0, ulAccessMode, &LogHdr ); 
			}
		}
		break;
	}

	if(Status == STATUS_ACCESS_DENIED)
	{
		if(ISLogWritePolicy( LogHdr.ulLogID, FullPath.pBuffer[0] ))
		{
			LOG_Add( &LogHdr, wzProcName, &FullPath );
		}
		if(FullPath.pBuffer) FREE_N_POOL( FullPath );
		if(FullPath_Dest.pBuffer) FREE_N_POOL( FullPath_Dest );
		return CompleteRequest( pIrp, Status, 0 );
	}

	Status = CallDriverSkip( pDevExt, pIrp );
	if(FullPath.pBuffer) FREE_N_POOL( FullPath );
	if(FullPath_Dest.pBuffer) FREE_N_POOL( FullPath_Dest );
	return Status;

}





// IRP_MJ_DEVICE_CONTROL
NTSTATUS  
File_PreDeviceControl( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{	
	int                 nRet = 0;
	NTSTATUS            Status = STATUS_SUCCESS;	
	PIO_STACK_LOCATION  pIrpStack   = NULL;
	PFILE_OBJECT        pFileObject = NULL;
	PFLT_EXTENSION      pDevExt     = NULL;
	
	Status = VALID_CHECK( pDeviceObject, pIrp, &pDevExt, &pIrpStack );
	if(!NT_SUCCESS( Status ) || !pIrpStack)
	{
		Status = STATUS_INVALID_PARAMETER;
		KdPrint(("STATUS_INVALID_PARAMETER >> File_PreDeviceControl \n"));
		return CompleteRequest( pIrp, Status, 0 );
	}

	pFileObject = pIrpStack->FileObject;		
	
	if(pDeviceObject != g_MalwFind.pDeviceObject) 
	{
		return CallDriverSkip( pDevExt, pIrp );
	}

	Status = FltDrv_DeviceControl(  pFileObject, 
									pDeviceObject, 
									pIrpStack->Parameters.DeviceIoControl.IoControlCode,
									pIrpStack->Parameters.DeviceIoControl.Type3InputBuffer,
									pIrpStack->Parameters.DeviceIoControl.InputBufferLength,
									pIrp->AssociatedIrp.SystemBuffer,
									pIrpStack->Parameters.DeviceIoControl.OutputBufferLength,
									&pIrp->IoStatus );

	return CompleteRequest( pIrp, Status, 0 );	

}


// IRP_MJ_SYSTEM_CONTROL
// 마운트 될때 호출한다.
NTSTATUS   
File_PreSystemControl( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
	NTSTATUS            Status      = STATUS_SUCCESS;	
	PIO_STACK_LOCATION  pIrpStack   = NULL;
	PFILE_OBJECT        pFileObject = NULL;
	PFLT_EXTENSION      pDevExt     = NULL;

	Status = VALID_CHECK( pDeviceObject, pIrp, &pDevExt, &pIrpStack );
	if(!NT_SUCCESS( Status ) || !pIrpStack) 
	{
		Status = STATUS_INVALID_PARAMETER;
		KdPrint(("STATUS_INVALID_PARAMETER >> File_PreSystemControl \n"));
		return CompleteRequest( pIrp, Status, 0 );
	}
	pFileObject = pIrpStack->FileObject;

	return CallDriverSkip( pDevExt, pIrp );
}



// IRP_MJ_FILE_SYSTEM_CONTROL
NTSTATUS  
File_PreFileSystemControl( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
	int                 nRet        = 0;
	ULONG               ulControlCode = 0;
	NTSTATUS            Status      = STATUS_SUCCESS;		
	NAME_BUFFER         FullPath    = {0};
	PFILE_OBJECT        pFileObject = NULL;
	PFLT_EXTENSION      pDevExt     = NULL;
	PIO_STACK_LOCATION  pIrpStack   = NULL;
	WCHAR               wzVolume  [ MAX_VOLUME_NAME ];
	WCHAR               wzProcName[ MAX_PROCESS_LEN ];

	Status = VALID_CHECK( pDeviceObject, pIrp, &pDevExt, &pIrpStack );
	if(!NT_SUCCESS( Status ) || !pIrpStack) 
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> File_PreFileSystemControl \n"));
		return CompleteRequest( pIrp, Status, 0 );
	}

	pFileObject = pIrpStack->FileObject;	
	RtlZeroMemory( wzProcName, MAX_PROCESS_LEN*sizeof(WCHAR) );
	GetProcessName( PsGetCurrentProcess(), wzProcName, MAX_PROCESS_LEN );

	switch(pIrpStack->MinorFunction)
	{
	case IRP_MN_MOUNT_VOLUME:		
		return FsdCtrlMountVolume( pDeviceObject, pDevExt, pIrp );		
		
	case IRP_MN_LOAD_FILE_SYSTEM: KdPrint(("\n ---IRP_MN_LOAD_FILE_SYSTEM--- \n")); break;
	case IRP_MN_VERIFY_VOLUME:    KdPrint(("\n ---IRP_MN_VERIFY_VOLUME--- \n"));    break;

	case IRP_MN_USER_FS_REQUEST:
		switch(pIrpStack->Parameters.FileSystemControl.FsControlCode)
		{
		case FSCTL_REQUEST_OPLOCK_LEVEL_1: KdPrint(("\n ---FSCTL_REQUEST_OPLOCK_LEVEL_1--- Proc=%ws \n", wzProcName )); break;
		case FSCTL_REQUEST_OPLOCK_LEVEL_2: KdPrint(("\n ---FSCTL_REQUEST_OPLOCK_LEVEL_2--- Proc=%ws \n", wzProcName ));	break;
		case FSCTL_REQUEST_BATCH_OPLOCK:   KdPrint(("\n ---FSCTL_REQUEST_BATCH_OPLOCK  --- Proc=%ws \n", wzProcName ));	break;
		
		case FSCTL_SET_REPARSE_POINT:
			if (TRUE == g_MalwFind.DrvConfig.bGlobalCtrl)
			{
				if( UserMode == ExGetPreviousMode() && (!_wcsicmp(wzProcName, L"vds.exe") || !_wcsicmp(wzProcName, L"dmadmin.exe")) )
				{					
					ALLOCATE_N_POOL( FullPath );
					if(!FullPath.pBuffer) break;
					SET_POOL_ZERO( FullPath );

					nRet = GetFileFullPath( pDeviceObject, pFileObject, pDevExt, &FullPath );	
					if(nRet <= 0)
					{
						if(FullPath.pBuffer) FREE_N_POOL( FullPath );
						return CallDriverSkip( pDevExt, pIrp );
					}
										
					if(pFileObject && pFileObject->Flags & FO_STREAM_FILE)
					{
						KdPrint(("[Skip] ---FSCTL_SET_REPARSE_POINT---StreamFile Proc=%ws Path=%ws \n", wzProcName, FullPath.pBuffer )); 	
						if(FullPath.pBuffer) FREE_N_POOL( FullPath );
						return CallDriverSkip( pDevExt, pIrp );
					}				

					KdPrint(("[Deny] ---FSCTL_SET_REPARSE_POINT--- Proc=%ws Path=%ws \n", wzProcName, FullPath.pBuffer )); 
					if(FullPath.pBuffer) FREE_N_POOL( FullPath );
					return CompleteRequest( pIrp, STATUS_ACCESS_DENIED, 0 );
				}				
				
			}
			break;
			
		default: break;
		}
		break;

	default: break;
	}
	
	return CallDriverSkip( pDevExt, pIrp );

}



/*
void Parsing_ReparsePoint( IN PDEVICE_OBJECT pDeviceObject, IN PFILE_OBJECT pFileObject )
{
	ULONG        ulControlCode = 0x00;
	NAME_BUFFER  FullPath = {0};
	ULONG                      ulBufferLength  = 0;
	PREPARSE_DATA_BUFFER       pDataBuffer     = NULL;
	PREPARSE_GUID_DATA_BUFFER  pGuidDataBuffer = NULL;


	ALLOCATE_N_POOL( FullPath );
	if(!FullPath.pBuffer) return;
	SET_POOL_ZERO( FullPath );

	GetFileFullPath( pDeviceObject, pFileObject, pDevExt, &FullPath );	

	ulBufferLength  = pIrpStack->Parameters.FileSystemControl.InputBufferLength;
	pDataBuffer     = (PREPARSE_DATA_BUFFER)pIrp->AssociatedIrp.SystemBuffer;
	pGuidDataBuffer = (PREPARSE_GUID_DATA_BUFFER)pDataBuffer;

	if(IsReparseTagMicrosoft( pDataBuffer->ReparseTag )) Parsing_ReparsePoint_DataBuffer( pDataBuffer, ulBufferLength );
	else  Parsing_ReparsePoint_GuidDataBuffer( pGuidDataBuffer, ulBufferLength );

	if(FullPath.pBuffer) FREE_N_POOL( FullPath );

}
*/



void Parsing_ReparsePoint_DataBuffer( PREPARSE_DATA_BUFFER pDataBuffer, ULONG ulBufferLength )
{
	if(!pDataBuffer || !ulBufferLength)
	{
		return;
	}

	KdPrint(("[ Microsoft ReparseTag. ] \n"));
	KdPrint(("ReparseTag=0x%08x \n",    pDataBuffer->ReparseTag   ));
	KdPrint(("ReparseDataLength=%d \n", pDataBuffer->ReparseDataLength ));
	KdPrint(("Reserved=%d \n",          pDataBuffer->Reserved  ));

	KdPrint(("SubstituteNameOffset=%d \n", pDataBuffer->SymbolicLinkReparseBuffer.SubstituteNameOffset ));
	KdPrint(("SubstituteNameLength=%d \n", pDataBuffer->SymbolicLinkReparseBuffer.SubstituteNameLength ));
	KdPrint(("PrintNameOffset=%d \n",      pDataBuffer->SymbolicLinkReparseBuffer.PrintNameOffset      ));
	KdPrint(("PrintNameLength=%d \n",      pDataBuffer->SymbolicLinkReparseBuffer.PrintNameLength      ));
	KdPrint(("Flag=0x%08x \n",    pDataBuffer->SymbolicLinkReparseBuffer.Flags      ));
	KdPrint(("PathBuffer=%ws \n", pDataBuffer->SymbolicLinkReparseBuffer.PathBuffer ));

	

}


void Parsing_ReparsePoint_GuidDataBuffer( PREPARSE_GUID_DATA_BUFFER pGuidDataBuffer, ULONG ulBufferLength )
{
	if(!pGuidDataBuffer || !ulBufferLength)
	{
		return;
	}

	KdPrint(("[ Third Party ReparseTag. ] \n"));
	KdPrint(("ReparseTag=0x%08x \n",    pGuidDataBuffer->ReparseTag        ));
	KdPrint(("ReparseDataLength=%d \n", pGuidDataBuffer->ReparseDataLength ));
	KdPrint(("Reserved=%d \n",          pGuidDataBuffer->Reserved  ));
	KdPrint(("ReparseGuid=%s \n",      pGuidDataBuffer->ReparseGuid       ));
	KdPrint(("DataBuffer=%s \n",      pGuidDataBuffer->GenericReparseBuffer.DataBuffer ));


}





// IRP_MJ_DIRECTORY_CONTROL
NTSTATUS 
File_PreDirControl( IN PDEVICE_OBJECT  pDeviceObject, IN PIRP pIrp )
{
	NTSTATUS             Status      = STATUS_SUCCESS;	
	PIO_STACK_LOCATION   pIrpStack   = NULL;
	PFILE_OBJECT         pFileObject = NULL;
	PFLT_EXTENSION       pDevExt     = NULL;
	NAME_BUFFER          FullPath    =  {0};
	WCHAR                wzProcessName[ MAX_PROCESS_LEN ] = L"";
	ULONG                nRet=0, ulLength =0, ulMinorFunc=0;
	FILE_INFORMATION_CLASS  FileInfoClass = 0;
	
	if(g_MalwFind.pDeviceObject == pDeviceObject) 
	{
		return CompleteRequest( pIrp, STATUS_SUCCESS, 0 );
	}

	Status = VALID_CHECK( pDeviceObject, pIrp, &pDevExt, &pIrpStack );
	if(!NT_SUCCESS( Status ) || !pIrpStack) 
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> File_PreDirControl \n"));
		return CompleteRequest( pIrp, Status, 0 );	
	}
	
	if (FALSE == g_MalwFind.DrvConfig.bGlobalCtrl) return CallDriverSkip(pDevExt, pIrp);

	pFileObject = pIrpStack->FileObject;
	if(!pFileObject || pFileObject->Flags & FO_STREAM_FILE) return CallDriverSkip( pDevExt, pIrp );
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) return CallDriverSkip( pDevExt, pIrp );


	RtlZeroMemory( wzProcessName, sizeof(wzProcessName) );
	GetProcessName( PsGetCurrentProcess(), wzProcessName, MAX_PROCESS_LEN );
	
	FileInfoClass = pIrpStack->Parameters.QueryDirectory.FileInformationClass;	
	if(pIrpStack->MinorFunction != IRP_MN_QUERY_DIRECTORY)
	{
		return CallDriverSkip( pDevExt, pIrp );
	}

	ALLOCATE_N_POOL( FullPath );
	if(!FullPath.pBuffer) 
	{
		return CallDriverSkip( pDevExt, pIrp );
	}
	SET_POOL_ZERO( FullPath );

	nRet = GetFileFullPath( pDeviceObject, pFileObject, pDevExt, &FullPath );	
	if(nRet <= 0)
	{
		Status = CallDriverSkip( pDevExt, pIrp );	
		if(FullPath.pBuffer) FREE_N_POOL( FullPath );
		return Status;
	}
	
	if( FileInfoClass == FileBothDirectoryInformation || FileInfoClass == FileIdBothDirectoryInformation  )
	{
		if( !_wcsnicmp(FullPath.pBuffer, g_MalwFind.DrvConfig.wzUpperDir, wcslen(g_MalwFind.DrvConfig.wzUpperDir)) )
		{
			CONTEXT_DIR  DirCtx;
			RtlZeroMemory( &DirCtx, sizeof(DirCtx) );
			KeInitializeEvent( &DirCtx.EventDir, SynchronizationEvent , FALSE );
			DirCtx.pFullPath = &FullPath;
			
			if(g_MalwFind.ulOSVer >= OS_VER_WLH)
			{	// Windows Vista, Server 2008, Win7, Win8 Win2012		
				if(FileInfoClass == FileIdBothDirectoryInformation)
				{
					Status = CallDriverCopy( pDevExt, pIrp, File_PostDirControlComplete_WLH, &DirCtx );
					if(Status == STATUS_PENDING) KeWaitForSingleObject( &DirCtx.EventDir, Executive, KernelMode, TRUE, 0 ); 
				
					if(FullPath.pBuffer) FREE_N_POOL( FullPath );
					return Status;		
				}
			}
			else
			{	// Windows w2k, Windows XP, Windows 2003
				if(FileInfoClass == FileBothDirectoryInformation)
				{
					Status = CallDriverCopy( pDevExt, pIrp, File_PostDirControlComplete_WXP, &DirCtx );
					if(Status == STATUS_PENDING) KeWaitForSingleObject( &DirCtx.EventDir, Executive, KernelMode, TRUE, 0 ); 
					
					if(FullPath.pBuffer) FREE_N_POOL( FullPath );
					return Status;		
				}
			}				
		}
	}
	
	Status = CallDriverSkip( pDevExt, pIrp );	
	if(FullPath.pBuffer) FREE_N_POOL( FullPath );
	return Status;
}



// Windows w2k, Windows XP, Windows 2003
// IRP_MJ_DIRECTORY_CONTROL
NTSTATUS 
File_PostDirControlComplete_WXP( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext )
{
	ULONG                   nRet        = 0;
	NTSTATUS                Status      = STATUS_SUCCESS;
	NTSTATUS                StatusChk   = STATUS_SUCCESS;
	PIO_STACK_LOCATION      pIrpStack   = NULL;
	PFILE_OBJECT            pFileObject = NULL;
	PFLT_EXTENSION          pDevExt     = NULL;
	PWCHAR                  pwzHideDir  = NULL;
	PFILE_BOTH_DIR_INFORMATION  pPrevDirInfo    = NULL;
	PFILE_BOTH_DIR_INFORMATION  pCurrentDirInfo = NULL;
	FILE_INFORMATION_CLASS      FileInfoClass;	
	PCONTEXT_DIR            pDirCtx = NULL;        

	Status = pIrp->IoStatus.Status;
	if(pIrp->PendingReturned)  
	{
		IoMarkIrpPending( pIrp );
	}

	pDirCtx = (PCONTEXT_DIR)pContext;
	if(!pDirCtx) return Status;	
	pDirCtx->IoStatus = pIrp->IoStatus;

	StatusChk = VALID_CHECK( pDeviceObject, pIrp, &pDevExt, &pIrpStack );
	if(!pIrpStack) return Status;
	
	if (FALSE == g_MalwFind.DrvConfig.bGlobalCtrl)
	{
		return Status;
	}

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		return Status;
	}

	pwzHideDir = g_MalwFind.DrvConfig.wzHideFolder;
	if(!pwzHideDir) return Status;

	FileInfoClass = pIrpStack->Parameters.QueryDirectory.FileInformationClass;
	
	__try
	{				
		if(FileInfoClass == FileBothDirectoryInformation)  
		{
			pCurrentDirInfo = (PFILE_BOTH_DIR_INFORMATION)pIrp->UserBuffer;
			// if(pCurrentDirInfo) KdPrint(("[WXP] > FileName=%ws \n", pCurrentDirInfo->FileName ));

			do
			{	// Valid Check
				if(!pCurrentDirInfo) break;				
				if(!MmIsAddressValid( pCurrentDirInfo )) break;

				// Hide 할 폴더인지 비교					
				if( !_wcsnicmp( pCurrentDirInfo->FileName, pwzHideDir, wcslen( pwzHideDir ) ))
				{
					Protect_FolderHide_WXP( pIrp, pCurrentDirInfo, pPrevDirInfo );
				}

				// Pointer 조정
				pPrevDirInfo = pCurrentDirInfo;
				pCurrentDirInfo = (PFILE_BOTH_DIR_INFORMATION)( (PUCHAR)pCurrentDirInfo + pCurrentDirInfo->NextEntryOffset );
			} while(pPrevDirInfo->NextEntryOffset);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("File_PostControlComplete_WLH Exception Occured. \n"));
	}


	KeSetEvent( &pDirCtx->EventDir, 0, FALSE );
	return Status;
}


// Windows Vista, Server 2008, Win7, Win8 Win2012		
// IRP_MJ_DIRECTORY_CONTROL
NTSTATUS 
File_PostDirControlComplete_WLH( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext )
{
	ULONG                  nRet        = 0;
	NTSTATUS               Status      = STATUS_SUCCESS;
	NTSTATUS               StatusChk   = STATUS_SUCCESS;	
	PIO_STACK_LOCATION     pIrpStack   = NULL;
	PFILE_OBJECT           pFileObject = NULL;
	PFLT_EXTENSION         pDevExt     = NULL;
	PNAME_BUFFER           pFullPath   = NULL;
	PWCHAR                 pwzHideDir  = NULL;
	PFILE_ID_BOTH_DIR_INFORMATION  pPrevDirInfo    = NULL;
	PFILE_ID_BOTH_DIR_INFORMATION  pCurrentDirInfo = NULL;
	FILE_INFORMATION_CLASS         FileInfoClass;
	PCONTEXT_DIR           pDirCtx = NULL;             

	Status = pIrp->IoStatus.Status;
	if(pIrp->PendingReturned)  
	{
		IoMarkIrpPending( pIrp );
	}

	pDirCtx = (PCONTEXT_DIR)pContext;
	if(!pDirCtx) return Status;
	pDirCtx->IoStatus = pIrp->IoStatus;

	StatusChk = VALID_CHECK( pDeviceObject, pIrp, &pDevExt, &pIrpStack );
	if(!pIrpStack) return Status;

	if (FALSE == g_MalwFind.DrvConfig.bGlobalCtrl)
	{
		return Status;
	}

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		return Status;
	}

	pwzHideDir = g_MalwFind.DrvConfig.wzHideFolder;
	if(!pwzHideDir) return Status;
		
	FileInfoClass = pIrpStack->Parameters.QueryDirectory.FileInformationClass;

	__try
	{		
		if(FileInfoClass == FileIdBothDirectoryInformation)	
		{
			pCurrentDirInfo = (PFILE_ID_BOTH_DIR_INFORMATION)pIrp->UserBuffer;
			if(pCurrentDirInfo)
			{
				// KdPrint(("[WLH] > FileName=%ws \n", pCurrentDirInfo->FileName ));
			}

			do
			{					
				// Valid Check
				if(!pCurrentDirInfo) break;
				if(!MmIsAddressValid( pCurrentDirInfo )) break;

				// Hide 할 폴더인지 비교					
				if( !_wcsnicmp( pCurrentDirInfo->FileName, pwzHideDir, wcslen(pwzHideDir) ))
				{
					Protect_FolderHide_WLH( pIrp, pCurrentDirInfo, pPrevDirInfo );
				}

				// Pointer 조정
				pPrevDirInfo = pCurrentDirInfo;
				pCurrentDirInfo = (PFILE_ID_BOTH_DIR_INFORMATION)((PUCHAR)pCurrentDirInfo + pCurrentDirInfo->NextEntryOffset );
			} while(pPrevDirInfo->NextEntryOffset);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}

	KeSetEvent( &pDirCtx->EventDir, 0, FALSE );
	return Status;

}


void FsdGetObjectName( IN PVOID pObject, IN OUT PUNICODE_STRING  pObjectName )
{
	NTSTATUS                  Status          = STATUS_SUCCESS;
	ULONG                     ulRet           = 0;
	POBJECT_NAME_INFORMATION  pObjectNameInfo =  NULL;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		return;
	}

	PAGED_CODE();
	

	if(!pObject || !pObjectName || !pObjectName->Buffer)  
	{
		ASSERT( pObject && pObjectName && pObjectName->Buffer );
		return;
	}


	pObjectNameInfo = (POBJECT_NAME_INFORMATION)ExAllocatePoolWithTag( PagedPool, MAX_POOL_LEN, MALWFIND_NAME_TAG );
	// ASSERT( pObjectNameInfo );
	if(!pObjectNameInfo) return;
	RtlZeroMemory( pObjectNameInfo, MAX_POOL_LEN );

	__try
	{
		Status = ObQueryNameString( pObject, pObjectNameInfo, MAX_POOL_LEN, &ulRet );
		pObjectName->Length = 0;
		if(NT_SUCCESS( Status ))
		{			
			RtlCopyUnicodeString( pObjectName, &pObjectNameInfo->Name  );
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("FsdGetObjectName Exception Occured. \n"));
	}

	ExFreePoolWithTag( pObjectNameInfo, MALWFIND_NAME_TAG );

}

// 필터하려는 Device 인지 알아오는 함수
BOOLEAN  
TrueFltDevice( IN PDEVICE_OBJECT pDeviceObject, IN PUNICODE_STRING pDeviceName )
{
	USHORT  usIndex = 0;
	UNICODE_STRING FsdName;

	ASSERT( pDeviceObject && pDeviceName );
	if(!pDeviceObject || !pDeviceName ) return FALSE;

	for(usIndex=0; usIndex<FSD_MAX; usIndex++)
	{
		RtlInitUnicodeString( &FsdName, g_pFsdName[ usIndex ] );

		if(RtlCompareUnicodeString( pDeviceName, &FsdName, TRUE ) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;

}


// FileSystem Attach Detach
void   
FltFs_Notification( IN PDEVICE_OBJECT pDeviceObject, IN BOOLEAN bFsActive )
{
	NTSTATUS        Status = STATUS_SUCCESS;
	PWCHAR          pDeviceNameBuffer = NULL;
	UNICODE_STRING  DeviceName;
	BOOLEAN         bFltDevice = FALSE;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{
		return;
	}

	PAGED_CODE();

	if(!pDeviceObject) 
	{
		ASSERT( pDeviceObject );
		return;
	}

	pDeviceNameBuffer = (PWCHAR)ExAllocatePoolWithTag( PagedPool, MAX_POOL_LEN, MALWFIND_NAME_TAG );
	if(!pDeviceNameBuffer)  return;		


	RtlZeroMemory( pDeviceNameBuffer, MAX_POOL_LEN );
	RtlInitEmptyUnicodeString( &DeviceName, pDeviceNameBuffer, MAX_POOL_LEN );	
	FsdGetObjectName( pDeviceObject, &DeviceName );

	bFltDevice = TrueFltDevice( pDeviceObject, &DeviceName );
	if(!bFltDevice)
	{
		if(pDeviceNameBuffer) 
		{
			ExFreePoolWithTag( pDeviceNameBuffer, MALWFIND_NAME_TAG );
			pDeviceNameBuffer = NULL;
		}
		return;
	}

	if(bFsActive)  // Attach Implement
	{		
		Status = FsdAttachToFileSystem( pDeviceObject, &DeviceName );

	}
	else  // Detach Implement
	{	
		FsdDetachFromFileSystem( pDeviceObject, &DeviceName );
	}

	if(pDeviceNameBuffer) 
	{
		ExFreePoolWithTag( pDeviceNameBuffer, MALWFIND_NAME_TAG );
	}

}


// ATTACH DETACH 관련함수
NTSTATUS  
FsdAttachToDeviceStack( IN PDEVICE_OBJECT pDevObjSrc, 
						IN PDEVICE_OBJECT pDevObjDest, 
						IN OUT PDEVICE_OBJECT*  ppLowerDeviceObject )
{
	NTSTATUS Status = STATUS_SUCCESS;

	*ppLowerDeviceObject = IoAttachDeviceToDeviceStack( pDevObjSrc, pDevObjDest );
	ASSERT( *ppLowerDeviceObject );

	if(!(*ppLowerDeviceObject))  Status = STATUS_NO_SUCH_DEVICE;

	return Status;
}


NTSTATUS  
FsdCreateDevice( IN PDEVICE_OBJECT pDeviceObject, IN OUT PDEVICE_OBJECT*  ppNewDeviceObject )
{
	NTSTATUS        Status  = STATUS_SUCCESS;
	PFLT_EXTENSION  pDevExt = NULL;

	Status = IoCreateDevice( g_MalwFind.pDriverObject,
							sizeof(FLT_EXTENSION),
							(PUNICODE_STRING)NULL,
							pDeviceObject->DeviceType,
							0,
							FALSE,
							ppNewDeviceObject );

	if(NT_SUCCESS( Status ))
	{
		pDevExt = (PFLT_EXTENSION)(*ppNewDeviceObject)->DeviceExtension;
		ASSERT( pDevExt );
		if(pDevExt)
		{
			FltDeviceExtension_Init( pDevExt );
		}
	}
	return Status;

}


void   
FsdCleanupMountDevice( IN PDEVICE_OBJECT pDeviceObject )
{
	UNREFERENCED_PARAMETER( pDeviceObject );
	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );
}

// Registry 에서 마운트 정보를 읽어 온다.
NTSTATUS  
GetRegMountedDevices(IN OUT PPMOUNTED_DEVICE ppMntDevice )
{
	NTSTATUS           Status   = STATUS_SUCCESS;
	NTSTATUS           StatusEx = STATUS_SUCCESS;
	HANDLE             pKey = NULL;
	OBJECT_ATTRIBUTES  ObjectAttr;
	OBJECT_ATTRIBUTES  ObjectAttrFile;
	UNICODE_STRING     MountedDevice;
	UNICODE_STRING     VolumeDevice;
	UNICODE_STRING     VolumeName;
	ULONG              nIndex =0;
	ULONG              nRetLenth=0;	
	PMOUNTED_DEVICE    pNewMntDevice  = NULL;
	PFILE_OBJECT       pFileObject    = NULL;
	PDEVICE_OBJECT     pDeviceObject  = NULL;
	PDEVICE_OBJECT     pVolFindDevObj = NULL;

	IO_STATUS_BLOCK    IoStatus       = {0}; 
	WCHAR              wzBuffer[ MAX_DOSDEVICE_NAME ];
	PKEY_VALUE_BASIC_INFORMATION  pBasicInfo = NULL;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) return STATUS_INVALID_LEVEL;


	PAGED_CODE();

	// 일단지운다.
	if(g_MalwFind.pMountedDevice)  ClearRegMountedDevices();

	RtlInitUnicodeString( &MountedDevice, FSD_MOUNTED_DEVICES );
	InitializeObjectAttributes( &ObjectAttr, &MountedDevice, 0, NULL, NULL);
	Status = ZwCreateKey( &pKey, KEY_ALL_ACCESS, &ObjectAttr, 0, NULL, REG_OPTION_NON_VOLATILE, NULL );
	if(!NT_SUCCESS( Status ))
	{
		KdPrint(("ZwCreateKey: Failed. Status[ %08x ] \n", Status));
		return Status;
	}

	pBasicInfo = ExAllocatePoolWithTag( PagedPool, MAX_POOL_LEN , MALWFIND_NAME_TAG );
	if(!pBasicInfo) 
	{
		Status = STATUS_INSUFFICIENT_RESOURCES;
		KdPrint(("ExAllocatePoolWithTag Failed. Status[ %08x ] \n", Status ));
		return Status;
	}

	do 
	{
		RtlZeroMemory( pBasicInfo, MAX_POOL_LEN  );
		Status = ZwEnumerateValueKey( pKey, nIndex++, KeyValueBasicInformation, pBasicInfo, MAX_POOL_LEN , &nRetLenth );
		if(!NT_SUCCESS( Status ))
		{			
			KdPrint(("ZwEnumerateValueKey Failed. Status[ %08x ] \n", Status ));
			break;
		}

		// FSD_DOSDEVICES Filter 한다. 12
		//		KdPrint(("pBasicInfo->Name[ %ws ] \n", pBasicInfo->Name ));
		if(_wcsnicmp( pBasicInfo->Name, FSD_DOSDEVICES, (ULONG)wcslen( FSD_DOSDEVICES )))  continue;

		RtlZeroMemory( wzBuffer, sizeof(wzBuffer) );
		RtlStringCchCopyW( wzBuffer, (sizeof(wzBuffer) >> 1), pBasicInfo->Name );

		// Floopy Drive Mount Check
		if(!_wcsnicmp( wzBuffer, FDD_A, (ULONG)wcslen(FDD_A)) || !_wcsnicmp( wzBuffer, FDD_B, wcslen(FDD_B)))
		{
			if(!FloppyCheckMounted( wzBuffer ))
			{
				KdPrint(("FloppyCheckMounted: return FALSE; [ %ws ] \n", wzBuffer ));
				continue;
			}
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Device Verify
		pFileObject   = NULL;  
		pDeviceObject = NULL;

		RtlInitUnicodeString( &VolumeDevice, wzBuffer );
		StatusEx = IoGetDeviceObjectPointer( &VolumeDevice, FILE_ANY_ACCESS, &pFileObject, &pDeviceObject );		

		if(!NT_SUCCESS( StatusEx ) || !pFileObject || !pDeviceObject)
		{
			KdPrint(("IoGetDeviceObjectPointer--Failed. VolumeDevice=[%ws] StatusEx[0x%08x] \n", wzBuffer, StatusEx ));
			continue;
		}
		ObDereferenceObject( pFileObject );

		pVolFindDevObj = NULL;		 
		if(!IS_DeviceStack_Mounted( pDeviceObject, &pVolFindDevObj ))
		{			
			KdPrint(("IS_DeviceStack_Mounted--FALSE--[%ws]  \n\n", VolumeDevice.Buffer ));
			continue;
		}

		if(!pVolFindDevObj)
		{
			KdPrint(("IS_DeviceStack_Mounted--TRUE--[pVolFindDevObj IsNull]--[%ws]  \n\n", VolumeDevice.Buffer ));
			continue;			
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		pNewMntDevice = ExAllocatePoolWithTag( PagedPool, sizeof(MOUNTED_DEVICE), MALWFIND_NAME_TAG );
		// ASSERT( pNewMntDevice );
		if(!pNewMntDevice) 
		{
			KdPrint(("ExAllocatePoolWithTag: MountDevice Failed! \n" ));
			continue;
		}

		RtlZeroMemory( pNewMntDevice, sizeof(MOUNTED_DEVICE) );
		// DriveName Copy 
		pNewMntDevice->pDeviceObject = pVolFindDevObj;
		pNewMntDevice->Flags         = 0x00;
		pNewMntDevice->pNext         = NULL;
		RtlStringCchCopyNW( pNewMntDevice->DosDeviceName, 
							(sizeof(pNewMntDevice->DosDeviceName) >> 1), 
							pBasicInfo->Name, 
							pBasicInfo->NameLength  ); 

		ppMntDevice = &g_MalwFind.pMountedDevice;
		if(*ppMntDevice)
		{
			PMOUNTED_DEVICE  pDevPos = *ppMntDevice;
			while(pDevPos->pNext) pDevPos = pDevPos->pNext;
			if(pDevPos) pDevPos->pNext = pNewMntDevice;
		}
		else
		{	
			*ppMntDevice = pNewMntDevice;
		}	
	} while( Status != STATUS_NO_MORE_ENTRIES );

	// Atfer Service
	if(pBasicInfo)
	{
		ExFreePoolWithTag( pBasicInfo, MALWFIND_NAME_TAG );
		pBasicInfo = NULL;
	}

	if(pKey) ZwClose( pKey );
	return Status;
}



void  
ClearRegMountedDevices()
{
	PMOUNTED_DEVICE  pMntDel    = NULL;
	PMOUNTED_DEVICE  pMntDevice = g_MalwFind.pMountedDevice;

	__try
	{
		while(pMntDevice)
		{
			pMntDel = pMntDevice;
			pMntDevice = pMntDevice->pNext;
			ExFreePoolWithTag( pMntDel, MALWFIND_NAME_TAG );
			pMntDel = NULL;
		}

		g_MalwFind.pMountedDevice = NULL;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{

		KdPrint(("ClearMountDriveNames Exception Occured! \n"));

	}

}




NTSTATUS FsdAttachToFileSystem( IN PDEVICE_OBJECT pDeviceObject, IN PUNICODE_STRING  pDeviceName )
{
	NTSTATUS         Status           = STATUS_SUCCESS;
	BOOLEAN          bSuccess         = FALSE;
	PDRIVER_OBJECT   pDriverObject    = NULL;
	PDEVICE_OBJECT   pVolDevObj       = NULL;
	PDEVICE_OBJECT   pDiskDevObj      = NULL;
	PDEVICE_OBJECT   pNewDeviceObject = NULL;
	PMOUNTED_DEVICE  pMntDevice       = NULL;
	WCHAR            wzVolume[ MAX_VOLUME_NAME ] = L""; // 8

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
	{
		return STATUS_INVALID_LEVEL;
	}

	PAGED_CODE();

	if(!pDeviceObject || !pDeviceName ) 
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> FsdAttachToFileSystem 1. \n"));
		return STATUS_INVALID_PARAMETER;
	}

	// DriverEntry 에서 호출하는것은 Sikp
	if(g_MalwFind.bLoadMode == FALSE) // DriverEntry 에서 호출하는것이 아니다.
	{
		PMOUNTED_DEVICE pFetchMountDevice = NULL;
		GetRegMountedDevices( &pFetchMountDevice );
	}

	// Filter DeviceObject Associated!
	if(!IS_FLT_DEVICE_TYPE( pDeviceObject->DeviceType ))  
	{
		return STATUS_NOT_SUPPORTED;
	}

	pDriverObject = pDeviceObject->DriverObject;
	if(!pDriverObject) 
	{
		KdPrint(("STATUS_INVALID_PARAMETER >> FsdAttachToFileSystem 1. \n"));
		return STATUS_INVALID_PARAMETER;
	}


	RtlZeroMemory( wzVolume, sizeof(wzVolume) );
	// Control 은 먼저 올린다.
	pNewDeviceObject = NULL;
	Status = FsdAttachDeviceWithCreate( pDeviceObject, pDeviceName, NULL, &pNewDeviceObject );
	if(pNewDeviceObject)
	{
		KdPrint(("CtrlDeviceObject:[ %wZ ] Attached Successfully. \n", pDeviceName ));
	}

	if(!IS_VOLUME_DEVICE_TYPE( pDeviceObject->DeviceType ))  
	{
		return Status;
	}

	pVolDevObj = pDriverObject->DeviceObject;
	while(pVolDevObj)
	{	// W2000 Sp4 Later	

#if (NTDDI_VERSION >= NTDDI_WINXP)
		Status = IoGetDiskDeviceObject( pVolDevObj, &pDiskDevObj );
#else 
		Status = IoGetDiskDeviceObject_2K( pVolDevObj, &pDiskDevObj );
#endif

		if(NT_SUCCESS( Status ))
		{
			bSuccess = FALSE;
			ObDereferenceObject( pDiskDevObj ); 
			pMntDevice  = g_MalwFind.pMountedDevice;
			while(pMntDevice)
			{	// 관련되어 있는 스택인지 검사한다.
				bSuccess = FsdEqualDeviceStack( pVolDevObj, pMntDevice->pDeviceObject );
				if(bSuccess) 
				{
					RtlStringCchCopyW( wzVolume, (sizeof(wzVolume) >> 1), pMntDevice->DosDeviceName + (ULONG)wcslen(FSD_DOSDEVICES) );
					break;
				}
				pMntDevice = pMntDevice->pNext;
			}
			// 검색실패
			if(!bSuccess) 
			{
				pVolDevObj = pVolDevObj->NextDevice;
				continue;
			}		
			pNewDeviceObject = NULL;
			Status = FsdAttachDeviceWithCreate( pVolDevObj, NULL, wzVolume, &pNewDeviceObject );
			if(!pNewDeviceObject)  
			{
				pVolDevObj = pVolDevObj->NextDevice;
				continue;
			}	
			KdPrint(("Volume:[ %ws ] Attached Successfully. \n", wzVolume ));
		}
		else
		{
			//KdPrint(("[ %wZ ] IoGetDiskDeviceObject Failed: Status[ %08x ] \n", pDeviceName, Status ));
		}
		pVolDevObj = pVolDevObj->NextDevice;
	}
	return STATUS_SUCCESS;
}


// pDeviceObject : FileSystem DeviceOBject
BOOLEAN  FsdDetachDeleteDevice( IN PDEVICE_OBJECT pDeviceObject )
{
	PDEVICE_OBJECT  pMalwFindDevObj   = NULL;
	PDEVICE_OBJECT  pLowerDevObj = NULL;
	PFLT_EXTENSION  pDevExt = NULL;

	pLowerDevObj = pDeviceObject;	
	ASSERT( pLowerDevObj );
	if(!pLowerDevObj) return FALSE;

	pMalwFindDevObj = pLowerDevObj->AttachedDevice;
	while (pMalwFindDevObj)
	{
		if (IS_MALFIND_DEVICE_OBJECT(pMalwFindDevObj))
		{
			if (IS_NETWORK_DEVICE_TYPE(pMalwFindDevObj->DeviceType))
			{
				IoDetachDevice( pLowerDevObj );
				IoDeleteDevice(pMalwFindDevObj);
				pMalwFindDevObj = NULL;
				return TRUE;
			}
			pDevExt = pMalwFindDevObj->DeviceExtension;
			ASSERT( pDevExt );
			FsdCleanupMountDevice( pMalwFindDevObj );
			IoDetachDevice( pLowerDevObj );
			IoDeleteDevice(pMalwFindDevObj);
			pMalwFindDevObj = NULL;
			return TRUE;
		}

		pLowerDevObj = pMalwFindDevObj;
		if (pLowerDevObj)
		{
			pMalwFindDevObj = pLowerDevObj->AttachedDevice;
		}

	} // End while

	return FALSE;

}


// pDeviceObject : ControlDeviceObject   pDeviceName : ControlDeviceObject Name
void   
FsdDetachFromFileSystem( IN PDEVICE_OBJECT pDeviceObject, IN PUNICODE_STRING pDeviceName )
{
	BOOLEAN         bSuccess      = FALSE;
	PDEVICE_OBJECT  pFsdDevObj    = NULL;  // FileSystem DeviceObject : control, Volume
	PDRIVER_OBJECT  pDriverObject = NULL;

	ASSERT( pDeviceObject );
	if(!pDeviceObject)  return;
	if(pDeviceName)  KdPrint(("FsdDetachFromFileSystem: [%wZ] \n", pDeviceName ));

	// FileSystem Driver ex> ntfs fat 
	pDriverObject = pDeviceObject->DriverObject;
	ASSERT( pDriverObject );
	if(!pDriverObject )  return;

	pFsdDevObj = pDriverObject->DeviceObject;
	ASSERT( pFsdDevObj );
	if(!pFsdDevObj) return;

	while(pFsdDevObj)
	{
		// Detach Delete Device
		bSuccess = FsdDetachDeleteDevice( pFsdDevObj );
		if(bSuccess && pDeviceName && pFsdDevObj == pDeviceObject)
		{
			KdPrint(("[ %wZ ] : Detach Delete Device \n", pDeviceName ));
		}		
		pFsdDevObj = pFsdDevObj->NextDevice;
	}
}


// 완료루틴
NTSTATUS  FsdCtrlCompletion( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PVOID pContext )
{
	PFSCTL_CONTEXT pFsCtlContext = NULL;

	UNREFERENCED_PARAMETER( pIrp );
	UNREFERENCED_PARAMETER( pDeviceObject );
	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ));
	ASSERT( pContext );

	if(g_MalwFind.ulOSVer >= OS_VER_WXP)
	{
		if(pContext)
		{
			KeSetEvent( (PRKEVENT)pContext, IO_NO_INCREMENT, FALSE );
		}
	}
	else
	{ // Win2000

		pFsCtlContext = (PFSCTL_CONTEXT)pContext;
		ASSERT( pFsCtlContext );

		if(KeGetCurrentIrql() > PASSIVE_LEVEL)
		{
			if(pFsCtlContext)  ExQueueWorkItem( &pFsCtlContext->WorkItem,  DelayedWorkQueue );
		} 
		else 
		{	
			if(pFsCtlContext)  (pFsCtlContext->WorkItem.WorkerRoutine)( pFsCtlContext->WorkItem.Parameter );
		}
	}

	return STATUS_MORE_PROCESSING_REQUIRED;

}



// IRP_MN_MOUNT_VOLUME --> Call
NTSTATUS  
FsdCtrlMountVolume( IN PDEVICE_OBJECT pDeviceObject, IN PFLT_EXTENSION pDevExt, IN PIRP pIrp )
{
	NTSTATUS            Status  = STATUS_SUCCESS;
	PFLT_EXTENSION      pNewDevExt = NULL;
	PDEVICE_OBJECT      pNewDeviceObject = NULL;
	PIO_STACK_LOCATION  pIrpStack     = NULL;
	KEVENT              Event;
	PFSCTL_CONTEXT      pFsctlContext = NULL;

	PAGED_CODE();

	ASSERT( pDeviceObject && pDevExt && pIrp );
	ASSERT( IS_MALFIND_DEVICE_OBJECT( pDeviceObject ) );

	KdPrint(("FsdCtrlMountVolume 1 \n"));
	pIrpStack = IoGetCurrentIrpStackLocation( pIrp );	
	if(!pIrpStack)
	{
		return CallDriverSkip( pDevExt, pIrp );
	}

	// NHCAFlt DeviceObject 생성
	Status = FsdCreateDevice( pDeviceObject, &pNewDeviceObject );
	if(!NT_SUCCESS( Status )) 
	{
		DbgPrint( "FsdCtrlMountVolume >> FsdCreateDevice Error. Status=%08x \n", Status );
		return CallDriverSkip( pDevExt, pIrp );
	}

	pNewDevExt = pNewDeviceObject->DeviceExtension;
	if(!pNewDevExt) 
	{
		ASSERT( pNewDevExt );
		IoDeleteDevice( pNewDeviceObject );
		pNewDeviceObject = NULL;
		return CallDriverSkip( pDevExt, pIrp );
	}

	// DiskDeviceObject
	pNewDevExt->pRealDeviceObject = pIrpStack->Parameters.MountVolume.Vpb->RealDevice;     
	if(!pNewDevExt->pRealDeviceObject)
	{
		IoDeleteDevice( pNewDeviceObject );
		pNewDeviceObject = NULL;
		return CallDriverSkip( pDevExt, pIrp );		
	}

	if(g_MalwFind.ulOSVer >= OS_VER_WXP)
	{
		RtlZeroMemory( &Event, sizeof(Event) );
		KeInitializeEvent( &Event, NotificationEvent, FALSE );

		Status = CallDriverCopy( pDevExt, pIrp, FsdCtrlCompletion, &Event );
		if(Status == STATUS_PENDING)
		{
			Status = KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );
			ASSERT( Status == STATUS_SUCCESS );
		}  		
		Status = FsdCtrlMountVolumeComplete( pDeviceObject, pIrp, pNewDeviceObject );

	}
	else
	{  // Windows-2000
		pFsctlContext = ExAllocatePoolWithTag( NonPagedPool, sizeof(FSCTL_CONTEXT), MALWFIND_NAME_TAG );
		ASSERT( pFsctlContext );
		if(!pFsctlContext)
		{
			Status = CallDriverSkip( pDevExt, pIrp );
		}
		else
		{  
			ExInitializeWorkItem( &pFsctlContext->WorkItem, FsdCtrlMountVolumeRoutine, pFsctlContext );
			pFsctlContext->pIrp = pIrp;
			pFsctlContext->pDeviceObject    = pDeviceObject;
			pFsctlContext->pNewDeviceObject = pNewDeviceObject;
			Status = CallDriverCopy( pDevExt, pIrp, FsdCtrlCompletion, pFsctlContext );       
		}
	}

	return Status;
}


// WorkItem --> WorkThread Routine Win2000 만 호출
void FsdCtrlMountVolumeRoutine( IN PVOID pContext )
{
	PFSCTL_CONTEXT pFsctlContext = (PFSCTL_CONTEXT)pContext;
	ASSERT( pFsctlContext );	
	if(pFsctlContext)
	{ 
		FsdCtrlMountVolumeComplete( pFsctlContext->pDeviceObject, pFsctlContext->pIrp, pFsctlContext->pNewDeviceObject );
		
		ExFreePoolWithTag( pFsctlContext, MALWFIND_NAME_TAG );
		pFsctlContext = NULL;
	}

}


// 실제로 어태치 하는 함수
NTSTATUS  
FsdCtrlMountVolumeComplete( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp, IN PDEVICE_OBJECT pNewDeviceObject )
{
	BOOLEAN         bAttached = FALSE;
	PFLT_EXTENSION  pNewDevExt = NULL;
	NTSTATUS        Status = STATUS_SUCCESS;
	PVPB            pVpb  =  NULL;
	NAME_BUFFER     FullPath = {0};

	if(!pDeviceObject || !pNewDeviceObject)  
	{		
		if(pNewDeviceObject)
		{
			FsdCleanupMountDevice( pNewDeviceObject );
			IoDeleteDevice( pNewDeviceObject );
		}
		Status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		KdPrint(("STATUS_INVALID_PARAMETER >> FsdCtrlMountVolumeComplete 1. \n"));
		return STATUS_INVALID_PARAMETER;
	}

	pNewDevExt = pNewDeviceObject->DeviceExtension;	
	if(!pNewDevExt || !pNewDevExt->pRealDeviceObject) 
	{		
		if(pNewDeviceObject)
		{
			FsdCleanupMountDevice( pNewDeviceObject );
			IoDeleteDevice( pNewDeviceObject );
			pNewDeviceObject = NULL;
		}
		Status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		return Status;
	}

	pVpb = pNewDevExt->pRealDeviceObject->Vpb;
	if(!pVpb)
	{	
		if(pNewDeviceObject)
		{
			FsdCleanupMountDevice( pNewDeviceObject );
			IoDeleteDevice( pNewDeviceObject );
			pNewDeviceObject = NULL;
		}
		Status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		return Status;
	}

	if(!pVpb->DeviceObject) 
	{
		if(pNewDeviceObject)
		{
			FsdCleanupMountDevice( pNewDeviceObject );
			IoDeleteDevice( pNewDeviceObject );
			pNewDeviceObject = NULL;
		}		
		Status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		return Status;
	}

	// VolumeDevice Object Attach
	pNewDevExt->pVolumeDeviceObject = pVpb->DeviceObject;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL) 
	{	
		KdPrint(("FsdCtrlMountVolumeComplete >> Irql >= DISPATCH_LEVEL. \n"));
		if(pNewDeviceObject)
		{
			FsdCleanupMountDevice( pNewDeviceObject );
			IoDeleteDevice( pNewDeviceObject );
			pNewDeviceObject = NULL;
		}
		Status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		return Status;
	}
	//
	//
	FsRtlEnterFileSystem();
	ExAcquireResourceExclusiveLite( &g_MalwFind.LockResource, TRUE );

	Status = GetVolumeDosName( pNewDevExt );
	if(!NT_SUCCESS( Status ))
	{
		KdPrint(("\n\nFsdCtrlMountVolumeComplete >> GetVolumeDosName Failed. Status=0x%08x  \n\n\n", Status ));
		if(pNewDeviceObject)
		{
			FsdCleanupMountDevice( pNewDeviceObject );
			IoDeleteDevice( pNewDeviceObject );
			pNewDeviceObject = NULL;
		}
		ExReleaseResourceLite( &g_MalwFind.LockResource );
		FsRtlExitFileSystem();

		Status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		return Status;
	}

	ExReleaseResourceLite( &g_MalwFind.LockResource );
	FsRtlExitFileSystem();
	//
	//
	//

	bAttached = Attached_MalwFind_DeviceObject( pVpb->DeviceObject );
	// Mount Failed.
	if( (TRUE == bAttached) || !NT_SUCCESS( pIrp->IoStatus.Status ) )
	{
		if(pNewDeviceObject)
		{
			FsdCleanupMountDevice( pNewDeviceObject );
			IoDeleteDevice( pNewDeviceObject );
			pNewDeviceObject = NULL;
		}
		
		Status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		return Status;
	}

	ExAcquireFastMutex( &g_MalwFind.LockDevice );
	// Volume Attached > // DeviceObject 에 우리의 DeviceObject 를 Attach 한다.
	Status = FsdAttachToDeviceStack( pNewDeviceObject, pVpb->DeviceObject, &pNewDevExt->pLowerDeviceObject );
	if(!NT_SUCCESS( Status ))  
	{
		if(pNewDeviceObject)
		{
			FsdCleanupMountDevice( pNewDeviceObject );
			IoDeleteDevice( pNewDeviceObject );
			pNewDeviceObject = NULL;
		}

		ExReleaseFastMutex( &g_MalwFind.LockDevice );
		Status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		return Status;
	} 
	ExReleaseFastMutex( &g_MalwFind.LockDevice );
	
	
	ALLOCATE_N_POOL( FullPath );
	if(FullPath.pBuffer)
	{
		SET_POOL_ZERO( FullPath );
		FullPath.pBuffer[0] = pNewDevExt->wcVol;
		FullPath.pBuffer[1] = L':';
		FullPath.pBuffer[2] = L'\\';
		FullPath.pBuffer[3] = L'\0';
		FREE_N_POOL( FullPath );
	}
	DbgPrint( "FsdCtrlMountVolumeComplete >> [%c:\\] Volume-Attach Successfully! \n", pNewDevExt->wcVol );


	if(IS_VOLUME_DEVICE_TYPE( pVpb->DeviceObject->DeviceType )) 
	{			
		if(FlagOn( pVpb->DeviceObject->Flags,  DO_DIRECT_IO ))    
		{
			SetFlag( pNewDeviceObject->Flags, DO_DIRECT_IO   );
		}

		if(FlagOn( pVpb->DeviceObject->Flags, DO_BUFFERED_IO))  
		{
			SetFlag( pNewDeviceObject->Flags, DO_BUFFERED_IO);
		}

		if(FlagOn( pVpb->DeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN)) 
		{
			SetFlag( pNewDeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN);
		}
	}

	// 위에 디바이스가 어태치 할수 있도록 플래그를 클리어 한다.
	ClearFlag( pNewDeviceObject->Flags, DO_DEVICE_INITIALIZING );
		
	Status = pIrp->IoStatus.Status;
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	return Status;

}


/*
BOOLEAN 
FileEncrypt_RequestWorkItem( IN ULONG ulMajorFunc, IN PDEVICE_OBJECT pDeviceObject, IN PWCHAR pwzFullPath, IN PVOID pFcb )
{
	PFILE_WORKITEM  pNewWorkItem = NULL;

	if(!pwzFullPath) 
	{
		KdPrint(("FileEncrypt_RequestWorkItem >> pwzFullPath == NULL \n"));
		return FALSE;
	}

	pNewWorkItem = ExAllocatePoolWithTag( NonPagedPool, sizeof(FILE_WORKITEM), MALWFIND_NAME_TAG );
	if(!pNewWorkItem)
	{
		pNewWorkItem = ExAllocatePoolWithTag( NonPagedPool, sizeof(FILE_WORKITEM), MALWFIND_NAME_TAG );
		if(!pNewWorkItem)
		{
			KdPrint(("FileEncrypt_RequestWorkItem >> ExAllocatePoolWithTag Failed. FileName=%ws \n", pwzFullPath ));
			return FALSE;
		}
	}

	RtlZeroMemory( pNewWorkItem, sizeof(FILE_WORKITEM) );
	pNewWorkItem->ulMajorFuction = ulMajorFunc;
	pNewWorkItem->pFcb           = pFcb;
	pNewWorkItem->pDeviceObject  = pDeviceObject;

	RtlStringCchCopyW( pNewWorkItem->wzFilePath, MAX_260_PATH, pwzFullPath );
	ExInitializeWorkItem( &pNewWorkItem->WorkItem, FileEncrypt_WorkItem_Func, pNewWorkItem );							
	ExQueueWorkItem( &pNewWorkItem->WorkItem, CriticalWorkQueue );
	return TRUE;

}
*/