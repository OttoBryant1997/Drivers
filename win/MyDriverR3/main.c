#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <winioctl.h>

#define IOCTL_MUL CTL_CODE(FILE_DEVICE_UNKNOWN,0x9000,METHOD_BUFFERED, FILE_ANY_ACCESS)

int main(int argc, char** argv, char** env) 
{
	HANDLE hDev = NULL;
	CHAR readBuffer[100] = {0};
	DWORD nReadSize = 0;
	CHAR writeBuffer[50] = { 0 };
	hDev = CreateFile(L"\\\\.\\MyFirstDevice", FILE_GENERIC_READ | FILE_GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hDev == INVALID_HANDLE_VALUE) {
		printf("file create fail.\n");
		system("pause");
		return;
	}
	printf("file open success\n");
	system("pause");

	BOOL bRead = ReadFile(hDev, readBuffer, 100, &nReadSize, NULL);
	if (bRead) {
		printf("read fail:%d\n", nReadSize);
	}
	printf("readBuffer pointer is :%p,readSize:%d\n", readBuffer,nReadSize);
	printf("read msg:%s\n", readBuffer);
	printf("to Write\n");
	PCHAR msg = "This Message is from R3";
	DWORD writeSize = 0;
	BOOL bWrite = WriteFile(hDev,msg,strlen(msg),&writeSize,0);
	system("pause");

	// to do selfdefined io control
	printf("to do IoControl\n");
	DWORD32 inValue = 10, outValue = 0;
	DWORD returnSize = 0;
	DeviceIoControl(hDev, IOCTL_MUL, &inValue, sizeof(DWORD32),
		&outValue, sizeof(DWORD32),&returnSize,0);
	printf("after finish IoControl,outValue is :%d,returnSize:%d\n",outValue, returnSize);
	system("pause");
	CloseHandle(hDev);
	system("pause");
	return 0;
}