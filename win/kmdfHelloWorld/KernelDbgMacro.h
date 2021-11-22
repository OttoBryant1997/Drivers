#include <ntddk.h>
#include <wdf.h>

#define __DEBUG

#ifdef __DEBUG
#define OTTODBG(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, ##__VA_ARGS__)
#else
#define OTTODBG(format, ...)
#endif