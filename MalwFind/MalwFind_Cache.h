#ifndef _MALWFIND_CACHE_H_
#define _MALWFIND_CACHE_H_

#include  <ntifs.h>

#ifdef __cplusplus
extern "C"
{
#endif



typedef struct _BITMAP_RANGE {
    LIST_ENTRY      Links;
    LONGLONG        BasePage;
    ULONG           FirstDirtyPage;
    ULONG           LastDirtyPage;
    ULONG           DirtyPages;
    PULONG          Bitmap;
} BITMAP_RANGE, *PBITMAP_RANGE;

typedef struct _MBCB {
    CSHORT          NodeTypeCode;
    CSHORT          NodeIsInZone;
    ULONG           PagesToWrite;
    ULONG           DirtyPages;
    ULONG           Reserved;
    LIST_ENTRY      BitmapRanges;
    LONGLONG        ResumeWritePage;
    BITMAP_RANGE    BitmapRange1;
    BITMAP_RANGE    BitmapRange2;
    BITMAP_RANGE    BitmapRange3;
} MBCB, *PMBCB;

//
// Virtual Address Control BLock
//
typedef struct _VACB
{
    PVOID BaseAddress;
    struct _SHARED_CACHE_MAP *SharedCacheMap;
    union
    {
        LARGE_INTEGER FileOffset;
        USHORT ActiveCount;
    } Overlay;
    LIST_ENTRY LruList;
} VACB, *PVACB;

//
// Private Cache Map Structure and Flags
//
typedef struct _PRIVATE_CACHE_MAP_FLAGS
{
    ULONG DontUse:16;
    ULONG ReadAheadActive:1;
    ULONG ReadAheadEnabled:1;
    ULONG Available:14;
} PRIVATE_CACHE_MAP_FLAGS;

typedef struct _PRIVATE_CACHE_MAP
{
    union
    {
        CSHORT NodeTypeCode;
        PRIVATE_CACHE_MAP_FLAGS Flags;
        ULONG UlongFlags;
    };
    ULONG ReadAheadMask;
    PFILE_OBJECT FileObject;
    LARGE_INTEGER FileOffset1;
    LARGE_INTEGER BeyondLastByte1;
    LARGE_INTEGER FileOffset2;
    LARGE_INTEGER BeyondLastByte2;
    LARGE_INTEGER ReadAheadOffset[2];
    ULONG ReadAheadLength[2];
    KSPIN_LOCK ReadAheadSpinLock;
    LIST_ENTRY PrivateLinks;
} PRIVATE_CACHE_MAP, *PPRIVATE_CACHE_MAP;


//
// Shared Cache Map
//
typedef struct _SHARED_CACHE_MAP
{
    SHORT NodeTypeCode;
    SHORT NodeByteSize;
    ULONG OpenCount;
    LARGE_INTEGER FileSize;
    LIST_ENTRY BcbList;
    LARGE_INTEGER SectionSize;
    LARGE_INTEGER ValidDataLength;
    LARGE_INTEGER ValidDataGoal;
    PVACB InitialVacbs[4];
    PVACB *Vacbs;
    PFILE_OBJECT FileObject;
    PVACB ActiveVacb;
    PVOID NeedToZero;
    ULONG ActivePage;
    ULONG NeedToZeroPage;
    ULONG ActiveVacbSpinLock;
    ULONG VacbActiveCount;
    ULONG DirtyPages;
    LIST_ENTRY SharedCacheMapLinks;
    ULONG Flags;
    ULONG Status;
    PMBCB Mbcb;
    PVOID Section;
    PKEVENT CreateEvent;
    PKEVENT WaitOnActiveCount;
    ULONG PagesToWrite;
    LONGLONG BeyondLastFlush;
    PCACHE_MANAGER_CALLBACKS Callbacks;
    PVOID LazyWriteContext;
    LIST_ENTRY PrivateList;
    PVOID LogHandle;
    PVOID FlushToLsnRoutine;
    ULONG DirtyPageThreshold;
    ULONG LazyWritePassCount;
    PCACHE_UNINITIALIZE_EVENT UninitializeEvent;
    PVACB NeedToZeroVacb;
    ULONG BcbSpinLock;
    PVOID Reserved;
    KEVENT Event;
    EX_PUSH_LOCK VacbPushLock;
    PRIVATE_CACHE_MAP PrivateCacheMap;
} SHARED_CACHE_MAP, *PSHARED_CACHE_MAP;


//
// Memory Information Structures for NtQueryVirtualMemory
//
typedef struct
{
    UNICODE_STRING SectionFileName;
    WCHAR NameBuffer[ANYSIZE_ARRAY];
} MEMORY_SECTION_NAME, *PMEMORY_SECTION_NAME;

//
// Section Information Structures for NtQuerySection
//
typedef struct _SECTION_BASIC_INFORMATION
{
    PVOID           BaseAddress;
    ULONG           Attributes;
    LARGE_INTEGER   Size;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

typedef struct _SECTION_IMAGE_INFORMATION
{
    PVOID TransferAddress;
    ULONG ZeroBits;
    ULONG MaximumStackSize;
    ULONG CommittedStackSize;
    ULONG SubSystemType;
    USHORT SubSystemMinorVersion;
    USHORT SubSystemMajorVersion;
    ULONG GpValue;
    USHORT ImageCharacteristics;
    USHORT DllCharacteristics;
    USHORT Machine;
    UCHAR ImageContainsCode;
    UCHAR Spare1;
    ULONG LoaderFlags;
    ULONG ImageFileSize;
    ULONG Reserved[1];
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;





//
// Page Table Entry Definitions
//
typedef struct _HARDWARE_PTE
{
    ULONG64 Valid:1;
    ULONG64 Write:1;
    ULONG64 Owner:1;
    ULONG64 WriteThrough:1;
    ULONG64 CacheDisable:1;
    ULONG64 Accessed:1;
    ULONG64 Dirty:1;
    ULONG64 LargePage:1;
    ULONG64 Global:1;
    ULONG64 CopyOnWrite:1;
    ULONG64 Prototype:1;
    ULONG64 reserved0:1;
    ULONG64 PageFrameNumber:28;
    ULONG64 reserved1:12;
    ULONG64 SoftwareWsIndex:11;
    ULONG64 NoExecute:1;
} HARDWARE_PTE, *PHARDWARE_PTE;

typedef struct _MMPTE_SOFTWARE
{
    ULONG64 Valid:1;
    ULONG64 PageFileLow:4;
    ULONG64 Protection:5;
    ULONG64 Prototype:1;
    ULONG64 Transition:1;
    ULONG64 UsedPageTableEntries:10;
    ULONG64 Reserved:10;
    ULONG64 PageFileHigh:32;
} MMPTE_SOFTWARE, *PMMPTE_SOFTWARE;

typedef struct _MMPTE_TRANSITION
{
    ULONG64 Valid:1;
    ULONG64 Write:1;
    ULONG64 Owner:1;
    ULONG64 WriteThrough:1;
    ULONG64 CacheDisable:1;
    ULONG64 Protection:5;
    ULONG64 Prototype:1;
    ULONG64 Transition:1;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    ULONG64 PageFrameNumber:36;
    ULONG64 Unused:16;
#else
    ULONG64 PageFrameNumber:28;
    ULONG64 Unused:24;
#endif
} MMPTE_TRANSITION;

typedef struct _MMPTE_PROTOTYPE
{
    ULONG64 Valid:1;
    ULONG64 Unused0:7;
    ULONG64 ReadOnly:1;
    ULONG64 Unused1:1;
    ULONG64 Prototype:1;
    ULONG64 Protection:5;
    LONG64 ProtoAddress:48;
} MMPTE_PROTOTYPE;

typedef struct _MMPTE_SUBSECTION
{
    ULONG64 Valid:1;
    ULONG64 Unused0:4;
    ULONG64 Protection:5;
    ULONG64 Prototype:1;
    ULONG64 Unused1:5;
    LONG64 SubsectionAddress:48;
} MMPTE_SUBSECTION;

typedef struct _MMPTE_LIST
{
    ULONG64 Valid:1;
    ULONG64 OneEntry:1;
    ULONG64 filler0:3;
    ULONG64 Protection:5;
    ULONG64 Prototype:1;
    ULONG64 Transition:1;
    ULONG64 filler1:20;
    ULONG64 NextEntry:32;
} MMPTE_LIST;

typedef struct _MMPTE_HARDWARE
{
    ULONG64 Valid:1;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    ULONG64 Dirty1:1;
#else
#ifdef CONFIG_SMP
    ULONG64 Writable:1;
#else
    ULONG64 Write:1;
#endif
#endif
    ULONG64 Owner:1;
    ULONG64 WriteThrough:1;
    ULONG64 CacheDisable:1;
    ULONG64 Accessed:1;
    ULONG64 Dirty:1;
    ULONG64 LargePage:1;
    ULONG64 Global:1;
    ULONG64 CopyOnWrite:1;
    ULONG64 Prototype:1;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    ULONG64 Write:1;
    ULONG64 PageFrameNumber:36;
    ULONG64 reserved1:4;
#else
#ifdef CONFIG_SMP
    ULONG64 Write:1;
#else
    ULONG64 reserved0:1;
#endif
    ULONG64 PageFrameNumber:28;
    ULONG64 reserved1:12;
#endif
    ULONG64 SoftwareWsIndex:11;
    ULONG64 NoExecute:1;
} MMPTE_HARDWARE, *PMMPTE_HARDWARE;



//
// PTE Structures
//
typedef struct _MMPTE
{
    union
    {
        ULONG Long;
        HARDWARE_PTE Flush;
        MMPTE_HARDWARE Hard;
        MMPTE_PROTOTYPE Proto;
        MMPTE_SOFTWARE Soft;
        MMPTE_TRANSITION Trans;
        MMPTE_SUBSECTION Subsect;
        MMPTE_LIST List;
    } u;
} MMPTE, *PMMPTE;

//
// Section Extension Information
//
typedef struct _MMEXTEND_INFO
{
    ULONGLONG CommittedSize;
    ULONG ReferenceCount;
} MMEXTEND_INFO, *PMMEXTEND_INFO;

//
// Segment and Segment Flags
//
typedef struct _SEGMENT_FLAGS
{
    ULONG TotalNumberOfPtes4132:10;
    ULONG ExtraSharedWowSubsections:1;
    ULONG LargePages:1;
    ULONG Spare:20;
} SEGMENT_FLAGS, *PSEGMENT_FLAGS;

typedef struct _SEGMENT
{
    struct _CONTROL_AREA *ControlArea;
    ULONG TotalNumberOfPtes;
    ULONG NonExtendedPtes;
    ULONG Spare0;
    ULONGLONG SizeOfSegment;
    MMPTE SegmentPteTemplate;
    ULONG NumberOfCommittedPages;
    PMMEXTEND_INFO ExtendInfo;
    SEGMENT_FLAGS SegmentFlags;
    PVOID BasedAddress;
    union
    {
        SIZE_T ImageCommitment;
        PEPROCESS CreatingProcess;
    } u1;
    union
    {
        PSECTION_IMAGE_INFORMATION ImageInformation;
        PVOID FirstMappedVa;
    } u2;
    PMMPTE PrototypePte;
    MMPTE ThePtes[1];
} SEGMENT, *PSEGMENT;

//
// Event Counter Structure
//
typedef struct _EVENT_COUNTER
{
    SLIST_ENTRY ListEntry;
    ULONG RefCount;
    KEVENT Event;
} EVENT_COUNTER, *PEVENT_COUNTER;

//
// Flags
//
typedef struct _MMSECTION_FLAGS
{
    ULONG BeingDeleted:1;
    ULONG BeingCreated:1;
    ULONG BeingPurged:1;
    ULONG NoModifiedWriting:1;
    ULONG FailAllIo:1;
    ULONG Image:1;
    ULONG Based:1;
    ULONG File:1;
    ULONG Networked:1;
    ULONG NoCache:1;
    ULONG PhysicalMemory:1;
    ULONG CopyOnWrite:1;
    ULONG Reserve:1;
    ULONG Commit:1;
    ULONG FloppyMedia:1;
    ULONG WasPurged:1;
    ULONG UserReference:1;
    ULONG GlobalMemory:1;
    ULONG DeleteOnClose:1;
    ULONG FilePointerNull:1;
    ULONG DebugSymbolsLoaded:1;
    ULONG SetMappedFileIoComplete:1;
    ULONG CollidedFlush:1;
    ULONG NoChange:1;
    ULONG filler0:1;
    ULONG ImageMappedInSystemSpace:1;
    ULONG UserWritable:1;
    ULONG Accessed:1;
    ULONG GlobalOnlyPerSession:1;
    ULONG Rom:1;
    ULONG WriteCombined:1;
    ULONG filler:1;
} MMSECTION_FLAGS, *PMMSECTION_FLAGS;

typedef struct _MMSUBSECTION_FLAGS
{
    ULONG ReadOnly:1;
    ULONG ReadWrite:1;
    ULONG SubsectionStatic:1;
    ULONG GlobalMemory:1;
    ULONG Protection:5;
    ULONG Spare:1;
    ULONG StartingSector4132:10;
    ULONG SectorEndOffset:12;
} MMSUBSECTION_FLAGS, *PMMSUBSECTION_FLAGS;

typedef struct _MMSUBSECTION_FLAGS2
{
    ULONG SubsectionAccessed:1;
    ULONG SubsectionConverted:1;
    ULONG Reserved:30;
} MMSUBSECTION_FLAGS2;

//
// Control Area Structures
//
typedef struct _CONTROL_AREA
{
    PSEGMENT Segment;
    LIST_ENTRY DereferenceList;
    ULONG NumberOfSectionReferences;
    ULONG NumberOfPfnReferences;
    ULONG NumberOfMappedViews;
    ULONG NumberOfSystemCacheViews;
    ULONG NumberOfUserReferences;
    union
    {
        ULONG LongFlags;
        MMSECTION_FLAGS Flags;
    } u;
    PFILE_OBJECT FilePointer;
    PEVENT_COUNTER WaitingForDeletion;
    USHORT ModifiedWriteCount;
    USHORT FlushInProgressCount;
    ULONG WritableUserReferences;
    ULONG QuadwordPad;
} CONTROL_AREA, *PCONTROL_AREA;

typedef struct _LARGE_CONTROL_AREA
{
    PSEGMENT Segment;
    LIST_ENTRY DereferenceList;
    ULONG NumberOfSectionReferences;
    ULONG NumberOfPfnReferences;
    ULONG NumberOfMappedViews;
    ULONG NumberOfSystemCacheViews;
    ULONG NumberOfUserReferences;
    union
    {
        ULONG LongFlags;
        MMSECTION_FLAGS Flags;
    } u;
    PFILE_OBJECT FilePointer;
    PEVENT_COUNTER WaitingForDeletion;
    USHORT ModifiedWriteCount;
    USHORT FlushInProgressCount;
    ULONG WritableUserReferences;
    ULONG QuadwordPad;
    ULONG StartingFrame;
    LIST_ENTRY UserGlobalList;
    ULONG SessionId;
} LARGE_CONTROL_AREA, *PLARGE_CONTROL_AREA;

//
// Subsection and Mapped Subsection
//
typedef struct _SUBSECTION
{
    PCONTROL_AREA ControlArea;
    union
    {
        ULONG LongFlags;
        MMSUBSECTION_FLAGS SubsectionFlags;
    } u;
    ULONG StartingSector;
    ULONG NumberOfFullSectors;
    PMMPTE SubsectionBase;
    ULONG UnusedPtes;
    ULONG PtesInSubsection;
    struct _SUBSECTION *NextSubsection;
} SUBSECTION, *PSUBSECTION;

typedef struct _MSUBSECTION
{
    PCONTROL_AREA ControlArea;
    union
    {
        ULONG LongFlags;
        MMSUBSECTION_FLAGS SubsectionFlags;
    } u;
    ULONG StartingSector;
    ULONG NumberOfFullSectors;
    PMMPTE SubsectionBase;
    ULONG UnusedPtes;
    ULONG PtesInSubsection;
    struct _SUBSECTION *NextSubsection;
    LIST_ENTRY DereferenceList;
    ULONG_PTR NumberOfMappedViews;
    union
    {
        ULONG LongFlags2;
        MMSUBSECTION_FLAGS2 SubsectionFlags2;
    } u2;
} MSUBSECTION, *PMSUBSECTION;

//
// Segment Object
//
typedef struct _SEGMENT_OBJECT
{
    PVOID BaseAddress;
    ULONG TotalNumberOfPtes;
    LARGE_INTEGER SizeOfSegment;
    ULONG NonExtendedPtes;
    ULONG ImageCommitment;
    PCONTROL_AREA ControlArea;
    PSUBSECTION Subsection;
    PLARGE_CONTROL_AREA LargeControlArea;
    PMMSECTION_FLAGS MmSectionFlags;
    PMMSUBSECTION_FLAGS MmSubSectionFlags;
} SEGMENT_OBJECT, *PSEGMENT_OBJECT;

//
// Section Object
//
typedef struct _SECTION_OBJECT
{
    PVOID StartingVa;
    PVOID EndingVa;
    PVOID LeftChild;
    PVOID RightChild;
    PSEGMENT_OBJECT Segment;
} SECTION_OBJECT, *PSECTION_OBJECT;





#ifdef __cplusplus
}
#endif

#endif
