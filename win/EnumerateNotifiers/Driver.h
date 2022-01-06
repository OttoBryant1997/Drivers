#pragma once


#include <ntddk.h>

#define OTTODBG(fmt,...)  DbgPrintEx(77,0,fmt,##__VA_ARGS__)

NTSTATUS DriverEntry(PDRIVER_OBJECT pCurDrv, PUNICODE_STRING pReg);
VOID FindProcessNotify();

VOID DrvUnload(PDRIVER_OBJECT DriverObject);