#include <ntddk.h>
#define OTTODBG(fmt,...) DbgPrintEx(77,0,fmt,##__VA_ARGS__)

VOID DriUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj, PUNICODE_STRING pPath) 
{
	OTTODBG("DriverEntry");
	NTSTATUS status = STATUS_SUCCESS;
	pDriverObj->DriverUnload = DriUnload;
	KIRQL oldIrql = 0;
	KIRQL level =  KeGetCurrentIrql();
	OTTODBG("current Irql is %d", level);


	oldIrql = KeRaiseIrqlToDpcLevel();
	level = KeGetCurrentIrql();
	OTTODBG("after raise to dpc, Irql is %d", level);


	KeLowerIrql(oldIrql);
	level = KeGetCurrentIrql();
	OTTODBG("after low to oldIrql, Irql is %d", level);


	//?????? 
	KSPIN_LOCK kspinLock;
	KeInitializeSpinLock(&kspinLock);



	KeAcquireSpinLock(&kspinLock, &level);
	KIRQL spinlockIrql = KeGetCurrentIrql();
	OTTODBG("after KeAcquireSpinLock to dpc, Irql is %d", spinlockIrql);
	// to operate some global and Api in level dpc
	KeReleaseSpinLock(&kspinLock, level);
	return status;
}

VOID DriUnload(PDRIVER_OBJECT DriverObject) {
	OTTODBG("Driver unload.");
}