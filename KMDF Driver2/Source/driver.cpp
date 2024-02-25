#include <ntifs.h>
#include <ntddk.h>

extern "C" DRIVER_INITIALIZE DriverEntry;
extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
extern "C"

void DriverUnload(PDRIVER_OBJECT pDriverObject);
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath);

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DriverUnload)

extern "C" NTSTATUS NTAPI MmCopyVirtualMemory(
	PEPROCESS SourceProcess,
	PVOID SourceAddress,

	PEPROCESS TargetProcess,
	PVOID TargetAddress,

	SIZE_T BufferSize,

	KPROCESSOR_MODE PreviousMode,
	PSIZE_T ReturnSize
);

PDEVICE_OBJECT pDeviceObject;

#define IOCTL_BAD_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0xdeadbeef, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_TEST_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1200, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_TEST_FUNCTION CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MEMORY_COPY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1300, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MEMORY_COPY2 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1300, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

void DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	DbgPrintEx(0, 0, "Unload driver called!\n");

	UNICODE_STRING dos;
	RtlInitUnicodeString(&dos, L"\\??\\KMDFDriverIOCTL");
	IoDeleteSymbolicLink(&dos);
	IoDeleteDevice(pDriverObject->DeviceObject);
}

NTSTATUS IrpCreateClose(PDEVICE_OBJECT pDeviceObject, PIRP Irp)
{
	DbgPrintEx(0, 0, "Create/Close called!\n");
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

struct MyMemoryCopyArgs_t
{
	HANDLE SourcePID;
	PVOID SourceAddress;
	HANDLE TargetPID;
	PVOID TargetAddress;
	SIZE_T BufferSize;
};

NTSTATUS MemoryCopy(HANDLE SourcePID, PVOID SourceAddress, HANDLE TargetPID, PVOID TargetAddress, SIZE_T BufferSize)
{
	NTSTATUS Status;
	PEPROCESS SourceProcess, TargetProcess;

	if (Status = PsLookupProcessByProcessId(SourcePID, &SourceProcess) != STATUS_SUCCESS)
		return Status;

	if (Status = PsLookupProcessByProcessId(TargetPID, &TargetProcess) != STATUS_SUCCESS)
		return Status;

	SIZE_T returnSize;
	Status = MmCopyVirtualMemory(SourceProcess, SourceAddress, TargetProcess, TargetAddress, BufferSize, KernelMode, &returnSize);
	return Status;
}

NTSTATUS MemoryCopy(MyMemoryCopyArgs_t& args)
{
	return MemoryCopy(args.SourcePID, args.SourceAddress, args.TargetPID, args.TargetAddress, args.BufferSize);
}

NTSTATUS IOCTL(PDEVICE_OBJECT pDriverObject, PIRP Irp)
{
	DbgPrintEx(0, 0, "[event] IOCTL\n");
	NTSTATUS Status;
	ULONG BytesIO = 0;
	PIO_STACK_LOCATION IO;

	IO = IoGetCurrentIrpStackLocation(Irp);

	PVOID inputBuffer = Irp->AssociatedIrp.SystemBuffer;
	if (!inputBuffer)
	{
		Status = STATUS_INVALID_PARAMETER;
		Irp->IoStatus.Status = Status;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return Status;
	}

	auto &deviceIoControl = IO->Parameters.DeviceIoControl;

	switch (IO->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_TEST_CODE:
	case IOCTL_TEST_FUNCTION:
		DbgPrintEx(0, 0, "[-//-] Code received!\n");
		DbgPrintEx(0, 0, "[-//-] Input buffer length: %ul\n", deviceIoControl.InputBufferLength);
		DbgPrintEx(0, 0, "[-//-] Output buffer length: %ul\n", deviceIoControl.OutputBufferLength);

		if (deviceIoControl.InputBufferLength >= sizeof(int))
		{
			int* inputData = (int*)inputBuffer;
			DbgPrintEx(0, 0, "[-//-] Input data: %i\n", *inputData);
		}
		else
		{
			DbgPrintEx(0, 0, "[-//-] Wrong input buffer size (%ul)!\n", deviceIoControl.InputBufferLength);
		}

		Status = STATUS_SUCCESS;
		break;

	case IOCTL_MEMORY_COPY:
	case IOCTL_MEMORY_COPY2:

		if (deviceIoControl.InputBufferLength == sizeof(MyMemoryCopyArgs_t))
		{
			MyMemoryCopyArgs_t* inputData = (MyMemoryCopyArgs_t*)inputBuffer;
			MemoryCopy(*inputData);
		}
		else
		{
			ULONG structSize = sizeof(MyMemoryCopyArgs_t);
			DbgPrintEx(0, 0, "[-//-] Wrong input buffer size (get: %ul, expected: %ul)!\n", deviceIoControl.InputBufferLength, structSize);
		}

		Status = STATUS_SUCCESS;
		break;

	default:
		DbgPrintEx(0, 0, "Unknown code.\n");
		Status = STATUS_INVALID_PARAMETER;
		break;
	}


	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	DbgPrintEx(0, 0, "Driver entry!\n");
	UNICODE_STRING dev, dos;

	RtlInitUnicodeString(&dev, L"\\Device\\KMDFDriverIOCTL");
	RtlInitUnicodeString(&dos, L"\\??\\KMDFDriverIOCTL");

	NTSTATUS status = IoCreateDevice(pDriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(0, 0, "IoCreateDevice failed!\n");
		return STATUS_UNSUCCESSFUL;
	}
	
	status = IoCreateSymbolicLink(&dos, &dev);
	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(0, 0, "IoCreateSymbolicLink failed!\n");
		return STATUS_UNSUCCESSFUL;
	}

	pDriverObject->MajorFunction[IRP_MJ_CREATE] = IrpCreateClose;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = IrpCreateClose;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IOCTL;
	pDriverObject->DriverUnload = DriverUnload;

	pDeviceObject->Flags |= DO_DIRECT_IO;
	pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	return STATUS_SUCCESS;
}