#include <ntddk.h>
#include <wdm.h> 


#define DEVICE_NAME L"\\Device\\MyFirstDevice"
#define SYM_NAME L"\\??\\MyFirstDevice"

#define IOCTL_MUL CTL_CODE(FILE_DEVICE_UNKNOWN,0x9000,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define OTTODBG(fmt,...)   DbgPrintEx(77,0,fmt,##__VA_ARGS__);

NTSTATUS DriverEntry(PDRIVER_OBJECT, PUNICODE_STRING);
VOID DrvUnload(DRIVER_OBJECT* pDriverObject);
VOID IoTimeoutRountine(PDEVICE_OBJECT DeviceObject,PVOID Context);

VOID DpcRoutine(PKDPC Dpc,PVOID DeferredContext,PVOID SystemArgument1,PVOID SystemArgument2);

// 未文档化的函数,获取函数名
//EXTERN_C PCHAR PsGetProcessImageFileName(PEPROCESS Process);

KTIMER gTimer = { 0 };
KDPC gDpcObj = { 0 };

//typedef struct _SE_AUDIT_PROCESS_CREATION_INFO
//{
//	struct _OBJECT_NAME_INFORMATION* ImageFileName;
//}SE_AUDIT_PROCESS_CREATION_INFO;

WORK_QUEUE_ITEM gWorkItem = { 0 };
VOID workItemRoutine(PDEVICE_OBJECT,PVOID);

extern NTSYSAPI PUCHAR NTAPI PsGetProcessImageFileName(PEPROCESS);