#include "Driver.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT pCurDrv,PUNICODE_STRING pReg) {
	NTSTATUS status = STATUS_SUCCESS;
	pCurDrv->DriverUnload = DrvUnload;
	FindProcessNotify();
	return status;
}


VOID FindProcessNotify() {
	UNICODE_STRING apiName = { 0 };
	PUCHAR apiAddr = NULL;
	LONG offset = 0;
	PUCHAR PspSetCreateProcessNotifyRoutine = NULL;

	RtlInitUnicodeString(&apiName, L"PsSetCreateProcessNotifyRoutine");
	apiAddr = MmGetSystemRoutineAddress(&apiName);
	if (!apiAddr) {
		OTTODBG("MmGetSystemRoutineAddress Fail\n");
		return;
	}
	else {
		OTTODBG("PsSetCreateProcessNotifyRoutine apiAddr:%p\n", apiAddr);
	}
	for (INT i = 0; i < 1000; i++) {//找到PspSetCreateProcessNotifyRoutine
		/*
		fffff803`7cb955fa 0f95c2          setne   dl
		fffff803`7cb955fd e8 b6 01 00 00      call    nt!PspSetCreateProcessNotifyRoutine (fffff803`7cb957b8)
		fffff803`7cb95602 4883c428        add     rsp,28h
		易知，7cb957b8 - 7cb95602 = 01b6 ,e8b6010000 分为 e8   01b6
		*/
		if (*(apiAddr + i) == 0xe8) // call    nt!PspSetCreateProcessNotifyRoutine
		{
			apiAddr += i;
			offset = *(PLONG)(apiAddr + 1);
			PUCHAR basePos = apiAddr + 5;//当前汇编结束位置为
			PspSetCreateProcessNotifyRoutine = basePos + offset;
			break;
		}
	}
	if (!PspSetCreateProcessNotifyRoutine) {
		OTTODBG("PsSetCreateProcessNotifyRoutine NOT FOUND\n");
		return;
	}
	OTTODBG("PspSetCreateProcessNotifyRoutine :%p\n", PspSetCreateProcessNotifyRoutine);
	for (INT i = 0; i < 1000; i++) {//在代码段中找匹配的内容
		if (*(apiAddr + i) == 0x4c
			&& *(apiAddr + i + 1) == 0x8d
			&& *(apiAddr + i + 2) == 0x35) // LEA R14,offset
		{
			apiAddr += i;
			offset = *(PLONG)(apiAddr + 3);
			PUCHAR basePos = apiAddr + 7;//当前汇编结束位置为
			PspSetCreateProcessNotifyRoutine = basePos  + offset;
			break;
		}
	}
	if (!PspSetCreateProcessNotifyRoutine) {
		OTTODBG("PsSetCreateProcessNotifyRoutine still NOT FOUND\n");
		return;
	}
	PDWORD64 tempNotifyList = (PDWORD64)PspSetCreateProcessNotifyRoutine;
	DWORD64 tempNotify = 0;
	for (int i = 0; i < 64; i++) {
		tempNotify = *(tempNotifyList + i);
		if (!tempNotify) {
			OTTODBG("tempNotify no more\n");
			break;
		}

		OTTODBG("tempNotify[%d]=%p\n",i, (PDWORD64)(tempNotify& 0xfffffffffffffff8));
	}

}

VOID DrvUnload(PDRIVER_OBJECT DriverObject) {
	OTTODBG("DrvUnload");
}