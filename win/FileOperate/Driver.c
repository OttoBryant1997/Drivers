#include <ntifs.h>
#include <ntddk.h>

#define OTTODBG(fmt,...) \
 DbgPrintEx(DPFLTR_IHVDRIVER_ID,DPFLTR_ERROR_LEVEL,fmt,## __VA_ARGS__);

void DriverUnload(PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	OTTODBG("Driver unload.")
}
NTSTATUS kernelDeleteFile(PWCHAR filePath) {
	UNICODE_STRING file = { 0 };
	NTSTATUS status = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES objAttr = { 0 };
	RtlInitUnicodeString(&file, filePath);
	InitializeObjectAttributes(&objAttr, &file, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwDeleteFile(&objAttr);
	if (NT_SUCCESS(status)) {
		OTTODBG("Delete File Success\n");
	}
	else {
		OTTODBG("Delete File Failed\n");
	}
	return status;
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
	// 读源文件
	status = ZwOpenFile(&hFileSrc, GENERIC_ALL, &objSrc, &ioStackSrc, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_SYNCHRONOUS_IO_NONALERT);
	if (!NT_SUCCESS(status)) {
		OTTODBG("Open Source File Failed:%x\n",status);
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
	pFileBuffer = ExAllocatePoolWithTag(NonPagedPool, fileInfo.EndOfFile.QuadPart,'1gaT');
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
		OTTODBG("Read File Failed:%x",status);
		ZwClose(hFileSrc);
		ExFreePool(pFileBuffer);
		return status;
	}
	OTTODBG("--- IoInfo:%lld --- \n", ioStackSrc.Information);
	ZwClose(hFileSrc);
	// 写新文件
	status = ZwCreateFile(&hFileDest, GENERIC_ALL, &objDest, &ioStackDest, NULL, FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_WRITE, FILE_SUPERSEDE, FILE_SYNCHRONOUS_IO_NONALERT, NULL,0);
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
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver,PUNICODE_STRING pRegPath) {
	UNREFERENCED_PARAMETER(pRegPath);
	OTTODBG("DriverEntry");
	NTSTATUS status = STATUS_SUCCESS;
	pDriver->DriverUnload = DriverUnload;
	/*kernelDeleteFile(L"\\??\\C:\\123.txt");*/
	status =  kernelCopyFile(L"\\??\\c:\\456.exe",L"\\??\\c:\\123.exe");
	if (!NT_SUCCESS(status)) {
		OTTODBG("copy File Failed");
	}
	return status;
}