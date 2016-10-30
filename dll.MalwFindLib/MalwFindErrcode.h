

#ifndef _MALWF_ERRCODE_H_
#define _MALWF_ERRCODE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef FLT_STATUS 
#define FLT_STATUS  long
#endif

#ifndef FLT_SUCCESS
#define FLT_SUCCESS( Status ) (((long)Status) >= 0)
#endif

#define FLT_STATUS_SUCCESS               0x00000000L
#define FLT_STATUS_UNSUCCESSFUL          0xC0000001L
#define FLT_STATUS_INVALID_FALGS         0xC0000002L
#define FLT_STATUS_INVALID_PARAMETER     0xC0000003L
#define FLT_STATUS_INVALID_HANDLE        0xC0000004L
#define FLT_STATUS_INSUFFICIENT_MEMORY   0xC0000005L
#define FLT_STATUS_INSUFFICIENT_BUFFER   0xC0000006L

#define FLT_STATUS_ERROR_CREATEFILE      0xC0000011L
#define FLT_STATUS_ERROR_READFILE        0xC0000012L
#define FLT_STATUS_ERROR_WRITEFILE       0xC0000013L
#define FLT_STATUS_END_OF_FILE           0xC0000014L
#define FLT_STATUS_INVALID_POINTER       0xC0000015L

// MultiCharToWideChar FAIL
#define FLT_STATUS_MTOW_CONVERT_FAIL     0xC0000016L
// WideCharToMultiChar FAIL
#define FLT_STATUS_WTOW_CONVERT_FAIL     0xC0000017L

#define FLT_STATUS_NOT_AUTH              0xCFFF0000L
#define FLT_STATUS_NOT_EXIST             0xCFFF0001L
#define FLT_STATUS_SEQNUM_UNDERFLOW      0xCFFF0002L
#define FLT_STATUS_SEQNUM_OVERFLOW       0xCFFF0003L






	
#ifdef __cplusplus
}
#endif

#endif