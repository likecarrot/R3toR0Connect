#include	<ntifs.h>



#define		DEVICENAME	L"\\Device\\testDevice"
#define		DEVICESYM	L"\\??\\testDeviceSym"
#define		KDPRINT_PRE	"testDevice:"

VOID	DriverUnload(PDRIVER_OBJECT pDriver);
NTSTATUS	DeviceControl(PDEVICE_OBJECT	pDevice, PIRP irp);
NTSTATUS	DeviceCreateClose(PDEVICE_OBJECT pDevice, PIRP irp);
NTSTATUS	DeviceReadWrite(PDEVICE_OBJECT pDevice, PIRP irp);
NTSTATUS	DevicePassThough(PDEVICE_OBJECT pDevice, PIRP irp);

//判断是不是我们自己的设备
BOOLEAN		CheckMyDevice(PDEVICE_OBJECT pDevice);

PDRIVER_OBJECT	g_pDriver = NULL;

EXTERN_C	NTSTATUS	
DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath) {
	NTSTATUS	status;
	
	g_pDriver = pDriver;

	UNICODE_STRING	DeviceName,DeviceSym;
	DEVICE_OBJECT* pDevice;
	RtlInitUnicodeString(&DeviceName, DEVICENAME);
	RtlInitUnicodeString(&DeviceSym, DEVICESYM);

	status=IoCreateDevice(pDriver, NULL, &DeviceName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pDevice);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("创建设备失败.\n"));
	}
	else
	{
		KdPrint(("创建设备成功.\n"));
		pDevice->Flags |= DO_DIRECT_IO;
	}

	status = IoCreateSymbolicLink(&DeviceSym, &DeviceName);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("创建链接名失败.\n"));
	}
	else
	{
		KdPrint(("创建链接名成功.\n"));
	}

	pDriver->DriverUnload = DriverUnload;
	for (size_t i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriver->MajorFunction[i] = DevicePassThough;
	}
	pDriver->MajorFunction[IRP_MJ_CREATE] = 
		pDriver->MajorFunction[IRP_MJ_CLOSE] = DeviceCreateClose;

	pDriver->MajorFunction[IRP_MJ_READ] =
		pDriver->MajorFunction[IRP_MJ_WRITE] = DeviceReadWrite;

	pDriver->MajorFunction[IRP_MJ_LOCK_CONTROL] = DeviceControl;

	UNREFERENCED_PARAMETER(pRegPath);

	return	status;
}
VOID	DriverUnload(PDRIVER_OBJECT pDriver) {
	if (pDriver->DeviceObject!=NULL)
	{
		IoDeleteDevice(pDriver->DeviceObject);
		KdPrint(("删除设备成功.\n"));
	}
	UNICODE_STRING	Sym;
	RtlInitUnicodeString(&Sym, DEVICESYM);
	IoDeleteSymbolicLink(&Sym);
	UNREFERENCED_PARAMETER(pDriver);
	KdPrint(("驱动卸载.\n"));
}


NTSTATUS	DeviceControl(PDEVICE_OBJECT	pDevice, PIRP irp) {
	CheckMyDevice(pDevice);

	KdPrint((KDPRINT_PRE"DeviceControl.\n"));
	//PIO_STACK_LOCATION	irpsp = IoGetCurrentIrpStackLocation(irp);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return	STATUS_SUCCESS;
}
NTSTATUS	DeviceCreateClose(PDEVICE_OBJECT pDevice, PIRP irp) {
	CheckMyDevice(pDevice);

	
	PIO_STACK_LOCATION	irpsp = IoGetCurrentIrpStackLocation(irp);
	if (irpsp->MajorFunction == IRP_MJ_CREATE) {
		KdPrint((KDPRINT_PRE"Device-Create.\n"));
	}
	if (irpsp->MajorFunction==IRP_MJ_CLOSE)
	{
		KdPrint((KDPRINT_PRE"Device-Close.\n"));
	}
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return	STATUS_SUCCESS;
}
NTSTATUS	DeviceReadWrite(PDEVICE_OBJECT pDevice, PIRP irp) {
	CheckMyDevice(pDevice);
	KdPrint((KDPRINT_PRE"DeviceReadWrite.\n"));
	PIO_STACK_LOCATION	irpsp = IoGetCurrentIrpStackLocation(irp);
	PVOID buffer = NULL;
	NTSTATUS	status;
	ULONG		infoLen = 0;
	//拿到缓冲区,进行检查
	NT_ASSERT(irp->MdlAddress);
	buffer = MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority);
	do
	{
		if (buffer == NULL)
		{
			KdPrint((KDPRINT_PRE"buffer缓冲区错误.\n"));
			status = STATUS_INSUFFICIENT_RESOURCES;
			infoLen = 0;
			break;
		}

		switch (irpsp->MajorFunction)
		{
		case	IRP_MJ_READ:
			KdPrint((KDPRINT_PRE"DeviceReadWrite-IRP_MJ_READ-复制一份ABABAB给R3.\n"));
			memset(buffer, 0, irpsp->Parameters.Read.Length);
			memcpy(buffer,"ABABAB",irpsp->Parameters.Read.Length);
			status = STATUS_SUCCESS;
			infoLen = (ULONG)strlen((char*)buffer);
			break;
		case	IRP_MJ_WRITE:
			KdPrint((KDPRINT_PRE"DevicereadWrite-IRP_MJ_WRITE-拿到缓冲区以后,用kdprint输出.\n"));
			KdPrint(("%s\n", buffer));
			status = STATUS_SUCCESS;
			infoLen = irpsp->Parameters.Write.Length;
			break;
		default:
			status = STATUS_INVALID_DEVICE_REQUEST;
			infoLen = 0;
		}
	} while (FALSE);
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = infoLen;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return	STATUS_SUCCESS;
}
NTSTATUS	DevicePassThough(PDEVICE_OBJECT pDevice, PIRP irp) {
	CheckMyDevice(pDevice);

	KdPrint((KDPRINT_PRE"DevicePassThough,一律返回成功.\n"));
	//PIO_STACK_LOCATION	irpsp = IoGetCurrentIrpStackLocation(irp);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return	STATUS_SUCCESS;
}

BOOLEAN		CheckMyDevice(PDEVICE_OBJECT pDevice) {
	if (g_pDriver->DeviceObject==pDevice)
	{
		KdPrint((KDPRINT_PRE"是我们自己的设备.\n"));
		return	TRUE;
	}
	KdPrint((KDPRINT_PRE"不是我们自己的设备.\n"));
	return	FALSE;
}