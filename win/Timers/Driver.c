#include "Driver.h"

VOID DrvUnload(DRIVER_OBJECT* pDriverObject)
{
    OTTODBG("Driver %wZ unloaded.", pDriverObject->DriverName);
    if (pDriverObject->DeviceObject) {
        IoStopTimer(pDriverObject->DeviceObject);
        IoDeleteDevice(pDriverObject->DeviceObject);
        UNICODE_STRING uniSymName = { 0 };
        RtlInitUnicodeString(&uniSymName, SYM_NAME);
        IoDeleteSymbolicLink(&uniSymName);
    }
}


VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
    DbgPrint("DriverUnload()\r\n");
}
ULONG   GetProcessNameOffset(void)
{
    PEPROCESS curproc;
    int i = 0;

    curproc = PsGetCurrentProcess();
    for (i = 0; i < 3 * PAGE_SIZE; i++) {
        if (!strncmp("System", (PCHAR)curproc + i, strlen("System"))) {
            DbgPrint("offset:%d", i);
            return i;
        }
    }
    return 0;
}


//#pragma LOCKEDCODE
//void PrintProcessName(HANDLE dwPid)
//{
//    HANDLE ProcessHandle;
//    NTSTATUS status;
//    OBJECT_ATTRIBUTES  ObjectAttributes;
//    CLIENT_ID myCid;
//    PEPROCESS EProcess;
//    int a = GetProcessNameOffset();
//    InitializeObjectAttributes(&ObjectAttributes, 0, 0, 0, 0);
//
//    myCid.UniqueProcess = dwPid;
//    myCid.UniqueThread = 0;
//
//    //打开进程，获取句柄
//    status = ZwOpenProcess(&ProcessHandle, PROCESS_ALL_ACCESS, &ObjectAttributes, &myCid);
//    if (!NT_SUCCESS(status))
//    {
//        DbgPrint("error/n");
//        return;
//    }
//
//    //得到EPROCESS，结构中取进程名
//    status = ObReferenceObjectByHandle(ProcessHandle, FILE_READ_DATA, 0, KernelMode, &EProcess, 0);
//    if (status == STATUS_SUCCESS)
//    {
//        char* ProcessName = (char*)EProcess + a;
//        char* PsName = PsGetProcessImageFileName(EProcess);
//
//        DbgPrint("ProcessName is %s", ProcessName);
//        DbgPrint("PsName is %s/n", PsName);
//        ObDereferenceObject(EProcess);
//        ZwClose(ProcessHandle);
//    }
//    else
//    {
//        DbgPrint("Get ProcessName error");
//        ObDereferenceObject(EProcess);
//        ZwClose(ProcessHandle);
//    }
//}

NTSTATUS DriverEntry(PDRIVER_OBJECT     DriverObject, PUNICODE_STRING    RegistryPath)
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING uniDeviceName = { 0 };
    UNICODE_STRING uniSymName = { 0 };
    PDEVICE_OBJECT pDevice = NULL;
    OTTODBG("KmdfHelloWorld: DriverEntry\n");
    //PsSetCreateProcessNotifyRoutine(notifyCreateProcess,FALSE);

    OTTODBG("==Driver: %wZ InstallPath:%wZ == \n", DriverObject->DriverName, RegistryPath);
    DriverObject->DriverUnload = DrvUnload;

    RtlInitUnicodeString(&uniDeviceName, DEVICE_NAME);

    status = IoCreateDevice(DriverObject, 200, &uniDeviceName,
        FILE_DEVICE_UNKNOWN, 0, TRUE, &pDevice);

    if (!NT_SUCCESS(status)) {
        OTTODBG("Create Device Fail:%x\n", status);
        return status;
    }
    pDevice->Flags |= DO_BUFFERED_IO;

    RtlInitUnicodeString(&uniSymName, SYM_NAME);
    status = IoCreateSymbolicLink(&uniSymName, &uniDeviceName);
    if (!NT_SUCCESS(status))
    {
        IoDeleteDevice(pDevice);
        return status;
    }

    // ioTimer 一秒一次 DPC_LEVEL 有可能不在当前进程调用
    status = IoInitializeTimer(pDevice, IoTimeoutRountine, NULL);
    IoStartTimer(pDevice);
    // dpc timer
    KeInitializeTimer(&gTimer);
    KeInitializeDpc(&gDpcObj, DpcRoutine, NULL);
    LARGE_INTEGER dpcTime = { 0 };
    LARGE_INTEGER timeOut = { 0 };
    dpcTime.QuadPart = -10 * 1000 * 1000 * 4;
    timeOut.QuadPart = -10 * 1000 * 1000 * 2;

    KeSetTimer(&gTimer, dpcTime, &gDpcObj);
    OTTODBG("DpcTimer has started\n");
    status = KeWaitForSingleObject(&gTimer, Executive, KernelMode, FALSE, &timeOut);
    if (status == STATUS_TIMEOUT) {
        OTTODBG("wait Time out,cancel Tiemr\n");
        KeCancelTimer(&gTimer);
    }
    else {
        OTTODBG("DpcTimer has returned\n");
    }
    // 相对于DPC的级别限制 很多函数不能调用 可以使用WorkItem来做passive_level的任务
    PIO_WORKITEM pWorkItem  = IoAllocateWorkItem(pDevice);
    IoQueueWorkItem(pWorkItem, workItemRoutine, CriticalWorkQueue, NULL);
    return status;
}

#pragma LOCKEDCODE
VOID IoTimeoutRountine(PDEVICE_OBJECT DeviceObject, PVOID Context)
{
    OTTODBG("TimeoutRountine  irql:%d\n", KeGetCurrentIrql());
    //PrintProcessName(PsGetCurrentProcessId());
}

VOID DpcRoutine(PKDPC Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2) {
    OTTODBG("DpcRoutine  irql:%d\n", KeGetCurrentIrql());
}

VOID workItemRoutine(PDEVICE_OBJECT pDevObj,PVOID pContext) {
    OTTODBG("workItemRoutine  irql:%d\n", KeGetCurrentIrql());
}