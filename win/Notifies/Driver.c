#include "Driver.h"

///
/// 通知是仅仅获取该消息
/// 回调是对接下来的内核流程有影响的
///
NTSTATUS DriverEntry(PDRIVER_OBJECT pCurDrvObj, PUNICODE_STRING pReg) {
	OTTODBG("Driver Entry\n");
	NTSTATUS status = STATUS_SUCCESS;
	pCurDrvObj->DriverUnload = DriverUnload;
	pCurDrvObj->DriverInit = DriverInit;
    //status = PsSetCreateProcessNotifyRoutine(PsCreatNotify,FALSE);
    //status = PsSetLoadImageNotifyRoutine(LoadImageNotify);
    PsSetCreateThreadNotifyRoutine(CreateThreadNotify);
    
	return status;
}
VOID DriverUnload(PDRIVER_OBJECT pDrvObj) {
	OTTODBG("Driver Unload\n");
	//PsSetCreateProcessNotifyRoutine(PsCreatNotify, TRUE);
 //   PsRemoveLoadImageNotifyRoutine(LoadImageNotify);
    PsRemoveCreateThreadNotifyRoutine(CreateThreadNotify);
}

NTSTATUS DriverInit(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	OTTODBG("Driver Init\n");
	NTSTATUS status = STATUS_SUCCESS;
	return status;
}

VOID PsCreatNotify(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create) {
    PEPROCESS ParentEprocess = NULL;
    PEPROCESS ChildEprocess = NULL;
    NTSTATUS status;


    // 获取EPROCESS结构体
    status = PsLookupProcessByProcessId(ParentId, &ParentEprocess);
    if (!NT_SUCCESS(status))
    {
        OTTODBG("Get Parent Eprocess Failed\n");
        return;
    }
    status = PsLookupProcessByProcessId(ProcessId, &ChildEprocess);
    if (!NT_SUCCESS(status))
    {
        OTTODBG("Get Child Eprocess Failed\n");
        if (ParentEprocess)
            ObDereferenceObject(ParentEprocess);
        return;
    }
    if (Create)
    {
        // 通过EPROCESS获取进程名
        OTTODBG("CRETAE ParentName:%s---> ChildName：%s\n",
            PsGetProcessImageFileName(ParentEprocess),
            PsGetProcessImageFileName(ChildEprocess));
    }
    else {
        OTTODBG("CLOSE ParentName:%s---> ChildName：%s\n",
            PsGetProcessImageFileName(ParentEprocess),
            PsGetProcessImageFileName(ChildEprocess));
    }
    if (ParentEprocess)
        ObDereferenceObject(ParentEprocess);
    if (ChildEprocess)
        ObDereferenceObject(ChildEprocess);
}

VOID LoadImageNotify(PUNICODE_STRING FullImageName,
    HANDLE ProcessId, PIMAGE_INFO ImageInfo) {
    PEPROCESS loaderProcess = NULL;
    NTSTATUS status;
    status = PsLookupProcessByProcessId(ProcessId, &loaderProcess);
    if (!NT_SUCCESS(status))
    {
        OTTODBG("Get Parent Eprocess Failed\n");
        return;
    }

    OTTODBG("process :%s ---> loadImage：%wZ\n",
        PsGetProcessImageFileName(loaderProcess), FullImageName);

    if (loaderProcess)
        ObDereferenceObject(loaderProcess);
}

VOID CreateThreadNotify(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create) {
    PEPROCESS process = NULL;
    NTSTATUS status;
    status = PsLookupProcessByProcessId(ProcessId, &process);
    if (!NT_SUCCESS(status))
    {
        OTTODBG("Get Parent Eprocess Failed\n");
        return;
    }
    OTTODBG("process :%s --> Thread id: %p \n",
        PsGetProcessImageFileName(process),
        ThreadId);
    if (process)
        ObDereferenceObject(process);
}