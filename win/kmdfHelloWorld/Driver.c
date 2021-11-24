#include "KernelDbgMacro.h"

#define DEVICE_NAME L"\\Device\\MyFirstDevice"
#define SYM_NAME L"\\??\\MyFirstDevice"

#define IOCTL_MUL CTL_CODE(FILE_DEVICE_UNKNOWN,0x9000,METHOD_BUFFERED, FILE_ANY_ACCESS)


VOID notifyCreateProcess(HANDLE ParentId, HANDLE ProcessId,BOOLEAN Create) 
{
    if (Create)
    {
        OTTODBG("CreateProcess,ProID:%p,ParentID:%p\n", ProcessId, ParentId);
    }
    else {
        OTTODBG("CloseProcess,ProID:%p,ParentID:%p\n",ProcessId,ParentId);
    }
}

void DrvUnload(DRIVER_OBJECT* pDriverObject) 
{
    OTTODBG("Driver %wZ unloaded.", pDriverObject->DriverName);
    if (pDriverObject->DeviceObject) {
        IoDeleteDevice(pDriverObject->DeviceObject);
        UNICODE_STRING uniSymName = { 0 };
        RtlInitUnicodeString(&uniSymName, SYM_NAME);
        IoDeleteSymbolicLink(&uniSymName);
    }
    //PsSetCreateProcessNotifyRoutine(notifyCreateProcess, TRUE);
}

NTSTATUS FDOCreate(PDEVICE_OBJECT pDeviceObject,PIRP pIrp)
{
    OTTODBG("My device has been created.\n");
    UNREFERENCED_PARAMETER(pDeviceObject);
    NTSTATUS status = STATUS_SUCCESS;
    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = 0;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS FDOClose(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    UNREFERENCED_PARAMETER(pDeviceObject);
    NTSTATUS status = STATUS_SUCCESS;
    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = 0;
    OTTODBG("My device has been close.\n");
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS FDOCleanUp(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    UNREFERENCED_PARAMETER(pDeviceObject);
    NTSTATUS status = STATUS_SUCCESS;
    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = 0;
    OTTODBG("My device has been cleanup.\n");
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}
NTSTATUS FDORead(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    UNREFERENCED_PARAMETER(pDeviceObject);
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);
    //LARGE_INTEGER offset = pStack->Parameters.Read.ByteOffset;
    ULONG readSize = pStack->Parameters.Read.Length;
    PCHAR readBuffer = pIrp->AssociatedIrp.SystemBuffer;// 地址重映射
    PCHAR msg = "This Message From Kernel";
    RtlCopyMemory(readBuffer, msg, strlen(msg));

    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = strlen(msg);
    OTTODBG("pSystemBuffer pointer is:%p\n", readBuffer);
    OTTODBG("Want to read size :%d,Really Info len is:%lld\n", readSize, strlen(msg));

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS FDOWrite(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    OTTODBG("[%s %s] is called\n",__FILE__, __FUNCTION__);
    UNREFERENCED_PARAMETER(pDeviceObject);
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);

    ULONG writeSize = pStack->Parameters.Write.Length;
    PCHAR writeBuffer = pIrp->AssociatedIrp.SystemBuffer;// 地址重映射

    RtlZeroMemory(pDeviceObject->DeviceExtension, 200);
    RtlCopyMemory(pDeviceObject->DeviceExtension, writeBuffer, writeSize);
    OTTODBG("DeviceObject->DeviceExtension :%p", pDeviceObject->DeviceExtension);
    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = strlen(writeBuffer);
    OTTODBG("R0 Writer buffer pointer is %p,msg:%s\n", writeBuffer, writeBuffer);
    OTTODBG("Want to write size :%d,Really Info len is:%lld\n", writeSize, strlen(writeBuffer));

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}
/* 自定义虚拟设备控制 */
NTSTATUS FDOControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    OTTODBG("[%s %s] is called\n", __FILE__, __FUNCTION__);
    UNREFERENCED_PARAMETER(pDeviceObject);
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);

    UCHAR IOCode = pStack->MinorFunction;
    ULONG inLen = pStack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG outLen = pStack->Parameters.DeviceIoControl.OutputBufferLength;
    ULONG ioInfo = 0;
    switch (IOCode)
    {
    case IOCTL_MUL:
    {
        DWORD32 inData = *(PDWORD32)pIrp->AssociatedIrp.SystemBuffer;
        inData *= 5;
        *(PDWORD32)pIrp->AssociatedIrp.SystemBuffer = inData;
        ioInfo = sizeof(DWORD32);
        break;
    }
    default:
        status = STATUS_UNSUCCESSFUL;
        ioInfo = 0;
        break;
    }

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT     DriverObject,PUNICODE_STRING    RegistryPath)
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

    // 创建设备成功 创建对应符号链接
    RtlInitUnicodeString(&uniSymName, SYM_NAME);
    status = IoCreateSymbolicLink(&uniSymName, &uniDeviceName);
    if (!NT_SUCCESS(status))
    {
        IoDeleteDevice(pDevice);
        return status;
    }
    // 创建符号链接成功
    DriverObject->MajorFunction[IRP_MJ_CREATE] = FDOCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = FDOClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = FDOCleanUp;
    DriverObject->MajorFunction[IRP_MJ_READ] = FDORead;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = FDOWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = FDOControl;

    
    return status;
}
