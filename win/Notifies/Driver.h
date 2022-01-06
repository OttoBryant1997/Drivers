#pragma once

#include <ntifs.h>

#include <ntddk.h>


#define OTTODBG(fmt,...)  DbgPrintEx(77,0,fmt,##__VA_ARGS__) 

extern NTSYSAPI PUCHAR NTAPI PsGetProcessImageFileName(PEPROCESS);


NTSTATUS DriverEntry(PDRIVER_OBJECT, PUNICODE_STRING);
VOID DriverUnload(PDRIVER_OBJECT);
NTSTATUS DriverInit(PDRIVER_OBJECT ,PUNICODE_STRING );
VOID PsCreateNotify(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);

VOID LoadImageNotify(PUNICODE_STRING FullImageName,HANDLE ProcessId,PIMAGE_INFO ImageInfo);

VOID CreateThreadNotify(HANDLE ProcessId,HANDLE ThreadId,BOOLEAN Create);



