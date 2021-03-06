#include <ntifs.h>
#include <ntddk.h>

#define OTTODBG(fmt,...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,fmt, ##__VA_ARGS__);

void drvUnload(PDRIVER_OBJECT DriverObject) {
	OTTODBG(" -------drvUnload -------- \n");
}
NTSTATUS kernelCopyFile(PWCHAR inDestFile, PWCHAR inSrcFile) {
	HANDLE hFileDest = NULL;
	HANDLE hFileSrc = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING destFile = { 0 }, srcFile = { 0 };
	OBJECT_ATTRIBUTES objDest = { 0 }, objSrc = { 0 };
	RtlInitUnicodeString(&destFile, inDestFile);
	RtlInitUnicodeString(&srcFile, inSrcFile);
	IO_STATUS_BLOCK ioStackDest = { 0 }, ioStackSrc = { 0 };
	InitializeObjectAttributes(&objDest, &destFile,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	InitializeObjectAttributes(&objSrc, &srcFile,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	OTTODBG("destFile:%wZ", destFile);
	// 读源文件 GENERIC_ALL可能会导致 STATUS_SHARING_VIOLATION
	status = ZwOpenFile(&hFileSrc, GENERIC_READ, &objSrc, &ioStackSrc, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_SYNCHRONOUS_IO_NONALERT);
	if (!NT_SUCCESS(status)) {
		OTTODBG("Open Source File Failed:%x\n", status);
		return status;
	}
	FILE_STANDARD_INFORMATION fileInfo = { 0 };
	status = ZwQueryInformationFile(hFileSrc, &ioStackSrc, &fileInfo,
		sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
	if (!NT_SUCCESS(status)) {
		OTTODBG("Query Src File Info Failed\n");
		ZwClose(hFileSrc);
		return status;
	}
	PVOID pFileBuffer = NULL;
	pFileBuffer = ExAllocatePoolWithTag(NonPagedPool, fileInfo.EndOfFile.QuadPart, '1gaT');
	if (!pFileBuffer) {
		OTTODBG("ExAllocatePool Failed\n");
		ZwClose(hFileSrc);
		return status;
	}
	RtlZeroMemory(pFileBuffer, fileInfo.EndOfFile.QuadPart);
	LARGE_INTEGER readOffset = { 0 };
	readOffset.QuadPart = 0;
	status = ZwReadFile(hFileSrc, NULL, NULL, NULL, &ioStackSrc,
		pFileBuffer,
		(ULONG)fileInfo.EndOfFile.QuadPart,
		&readOffset, NULL);
	if (!pFileBuffer) {
		OTTODBG("Read File Failed:%x", status);
		ZwClose(hFileSrc);
		ExFreePool(pFileBuffer);
		return status;
	}
	OTTODBG("--- IoInfo:%lld --- \n", ioStackSrc.Information);
	ZwClose(hFileSrc);
	// 写新文件
	status = ZwCreateFile(&hFileDest, GENERIC_ALL, &objDest, &ioStackDest, NULL, FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_WRITE, FILE_SUPERSEDE, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (!NT_SUCCESS(status)) {
		OTTODBG("Create File Failed:%x", status);
		ExFreePool(pFileBuffer);
		return status;
	}
	LARGE_INTEGER writeOffset = { 0 };
	writeOffset.QuadPart = 0;
	status = ZwWriteFile(hFileDest, NULL, NULL, NULL, &ioStackDest, pFileBuffer, (ULONG)fileInfo.EndOfFile.QuadPart,
		&writeOffset, NULL);
	if (!NT_SUCCESS(status)) {
		OTTODBG("Write File Failed:%d\n", status);
		ExFreePool(pFileBuffer);
		ZwClose(hFileDest);
		return status;
	}
	OTTODBG("Really Write:%lld\n", ioStackDest.Information);
	ExFreePool(pFileBuffer);
	ZwClose(hFileDest);
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pPath) {
	NTSTATUS status = STATUS_SUCCESS;
	OTTODBG("-- Deriver Path:%wZ ---- \n",pPath);
	pDriver->DriverUnload = drvUnload;


	// open reg
	HANDLE hKey = NULL;
	OBJECT_ATTRIBUTES obj = { 0 };
	PVOID keyInfo = NULL;
	InitializeObjectAttributes(&obj, pPath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
	
	// ZwOpenKey or ZwCreateKey
	//ULONG keyDisp = 0;
	//status = ZwCreateKey(&hKey, KEY_ALL_ACCESS, &obj, 0, NULL, REG_OPTION_NON_VOLATILE, &keyDisp);
	//if (NT_SUCCESS(status)) {
	//	if (keyDisp == REG_CREATED_NEW_KEY) {
	//		OTTODBG("-- Key has been created. ---- \n");
	//	}
	//	else if (keyDisp == REG_OPENED_EXISTING_KEY) {
	//		OTTODBG("-- Key already exists. ---- \n");
	//	}
	//	else {
	//		OTTODBG("-- Unknown Key Value:%x. ---- \n", keyDisp);
	//	}
	//}
	DWORD32 memSize = 0x1000;
	status = ZwOpenKey(&hKey, KEY_ALL_ACCESS, &obj);
	if (NT_SUCCESS(status)) {
		ULONG memTag = '1gaT';
		keyInfo = ExAllocatePoolWithTag(NonPagedPool, memSize, memTag);
		if (!keyInfo) {
			return STATUS_ABANDONED;
		}
		RtlZeroMemory(keyInfo, memSize);
		UNICODE_STRING keyName = { 0 };
		RtlInitUnicodeString(&keyName, L"ImagePath");
		ULONG resultLen = 0;
		status = ZwQueryValueKey(hKey, &keyName, KeyValuePartialInformation, keyInfo, memSize, &resultLen);
		if (!NT_SUCCESS(status)) {
			OTTODBG("-- ZwQueryValueKey failed:%x.key:%wZ ---- \n", status, keyName);
			ZwClose(hKey);
			ExFreePoolWithTag(keyInfo, memTag);
			return status;
		}
		//success
		PKEY_VALUE_PARTIAL_INFORMATION info = (PKEY_VALUE_PARTIAL_INFORMATION)(keyInfo);
		PWCHAR imagePath = (PWCHAR)(info->Data);
		OTTODBG("-- imagePath:%ws. ---- \n",imagePath);


		//copy File
		status = kernelCopyFile(L"\\??\\C:\\Windows\\System32\\drivers\\startByRegister.sys", imagePath);

		PWCHAR rootPath = L"SystemRoot\\system32\\drivers\\startByRegister.sys";
		//set key-values
		status = ZwSetValueKey(hKey, &keyName, 0, REG_EXPAND_SZ, rootPath, wcslen(rootPath) * 2 + 2);
		if (NT_SUCCESS(status)) {
			OTTODBG("ZwSetValueKey fail,nt_status:%d",status);
		}
		//free
		ZwClose(hKey);
		ExFreePoolWithTag(keyInfo, memTag);

		//另一种方式写注册表
		ULONG startValue = 1;
		RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,pPath->Buffer,L"Start",REG_DWORD,&startValue,4);
		return status;
	}
	return status;
}