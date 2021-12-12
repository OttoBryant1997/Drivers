#include <ntddk.h>

#define OTTODEBUG(fmt,...) DbgPrintEx(77,0,fmt,##__VA_ARGS__);

//
// DECLARE
//
void DrvUnload(PDRIVER_OBJECT obj);
NTSTATUS DriverEntry(PDRIVER_OBJECT pDrvObj, PUNICODE_STRING pPath);

typedef struct __MyNode { int index; LIST_ENTRY Node; }MyNode, * PMyNode;

//
// DEFINE
//



NTSTATUS DriverEntry(PDRIVER_OBJECT pDrvObj, PUNICODE_STRING pPath)
{
	OTTODEBUG("Driver Entry.\n")
	NTSTATUS status = STATUS_SUCCESS;
	pDrvObj->DriverUnload = DrvUnload;
	DWORD32 memSize = 0x1000;
	
	ULONG memTag = 'xxaa';
	PVOID buf = ExAllocatePoolWithTag(NonPagedPool, memSize, memTag);
	if (buf) {
		RtlFillMemory(buf, memSize, 0xcc);
		RtlZeroMemory(buf, memSize);
		ExFreePoolWithTag(buf, memTag);
	}

	// 链表
	LIST_ENTRY head = { 0 };
	//初始化必须调
	InitializeListHead(&head);
	//小心for中的栈变量会立刻销毁
	for (int i = 0; i < 40; i++) {
		PMyNode pmyNode = (PMyNode)ExAllocatePoolWithTag(PagedPool, sizeof(MyNode), memTag);
		if (pmyNode) {
			pmyNode->index = i;
			InsertTailList(&head, &pmyNode->Node);
		}
	}

	
	OTTODEBUG("Start to iterate List\n");
	PMyNode pCurMyNode = NULL;
	for (PLIST_ENTRY pNext = head.Flink; pNext != &head; ) {
		pCurMyNode = CONTAINING_RECORD(pNext, MyNode, Node);
		int curIndex = pCurMyNode->index;
		pNext = pCurMyNode->Node.Flink;
		OTTODEBUG("Current Index :%d,pNext:%p,&head:%p\n", curIndex, pNext ,&head);
	}
	OTTODEBUG("Finish to iterate List\n");

	OTTODEBUG("Start to clear List\n");
	while (!IsListEmpty(&head)) {
		PLIST_ENTRY pEntry = RemoveHeadList(&head);//只是节点操作 不释放内存 如有需要调用匹配的释放函数
		PMyNode pmynode = CONTAINING_RECORD(pEntry, MyNode, Node);
		int index = pmynode->index;
		OTTODEBUG("Current remove Index :%d\n", index);
		ExFreePoolWithTag(pmynode, memTag);
	}
	OTTODEBUG("Finish to clear List\n");
	return status;
}


void DrvUnload(PDRIVER_OBJECT obj) {
	OTTODEBUG("Driver unload.\n")
}