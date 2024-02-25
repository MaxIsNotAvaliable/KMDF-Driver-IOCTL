#include <ntifs.h>
#include <ntddk.h>
#include "log.h"
#include "mem/routines.h"

extern "C" DRIVER_INITIALIZE DriverEntry;
extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
extern "C"

//#define ROUTINES_CAPTURE

void DriverUnload(PDRIVER_OBJECT pDriverObject);
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath);

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DriverUnload)

//typedef PCHAR(*GET_PROCESS_IMAGE_NAME) (PEPROCESS Process);


void PloadImageNotifyRoutine(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo)
{
	PEPROCESS pe;
	if (NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &pe)))
	{
		CHAR* processImageFileName = GetProcessImageFileName(pe);
		log::Print("Image base 0x%llx | Process [%s] load [%ws].", (UINT64)ImageInfo->ImageBase, processImageFileName, FullImageName->Buffer);
	}
}

void PcreateProcessNotifyRoutine(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create)
{
	PEPROCESS child;
	if (!NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &child)))
		return;

	PEPROCESS parent;
	if (!NT_SUCCESS(PsLookupProcessByProcessId(ParentId, &parent)))
		return;

	CHAR* parentName = GetProcessImageFileName(parent);
	CHAR* childName = GetProcessImageFileName(child);
	log::Print("%s -> [%s] -> %s", parentName, (Create ? "created" : "closed "), childName);
}

PDEVICE_OBJECT pDeviceObject;
UNICODE_STRING dev, dos;

#define IOCTL_TEST_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0xdeadbeef, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

void DriverUnload(PDRIVER_OBJECT pDriverObject)
{
#ifdef ROUTINES_CAPTURE
	PsRemoveLoadImageNotifyRoutine(PloadImageNotifyRoutine);
	PsSetCreateProcessNotifyRoutine(PcreateProcessNotifyRoutine, TRUE);
#endif
	log::Print("Called \"Driver Unload\"");
	//IoDeleteSymbolicLink(&dos);
	//log::Print("Called \"IoDeleteSymbolicLink\"");
	IoDeleteDevice(pDriverObject->DeviceObject);
	log::Print("Called \"IoDeleteDevice\"");

	log::PrintInfo("Driver unloaded.");
}

NTSTATUS Create(PDEVICE_OBJECT pDriverObject, PIRP Irp)
{
	log::Print("[event] Create");
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS Close(PDEVICE_OBJECT pDriverObject, PIRP Irp)
{
	log::Print("[event] Close");
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS IOCTL(PDEVICE_OBJECT pDriverObject, PIRP Irp)
{
	log::Print("[event] IOCTL");
	NTSTATUS Status;
	ULONG BytesIO = 0;
	PIO_STACK_LOCATION IO;

	IO = IoGetCurrentIrpStackLocation(Irp);

	if (IO->Parameters.DeviceIoControl.IoControlCode = IOCTL_TEST_CODE)
	{
		log::Print("Code received!");
		Status = STATUS_SUCCESS;
	}
	else
	{
		log::PrintError("Unknown code.");
		Status = STATUS_INVALID_PARAMETER;
	}

	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	log::PrintInfo("Driver initializing...");

	/*
	INT errCount = Routine::Initialize();
	if (errCount)
	{
		log::PrintError("Routine initialize errors: %i", errCount);
		return STATUS_FAILED_DRIVER_ENTRY;
	}
	*/

	RtlInitUnicodeString(&dev, L"\\Device\\ioctl");
	RtlInitUnicodeString(&dos, L"\\DosDevice\\ioctl");

	IoCreateDevice(pDriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	IoCreateSymbolicLink(&dos, &dev);

	pDriverObject->MajorFunction[IRP_MJ_CREATE] = Create;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = Close;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IOCTL;
	pDriverObject->DriverUnload = DriverUnload;

	pDeviceObject->Flags |= DO_DIRECT_IO;
	pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

#ifdef ROUTINES_CAPTURE
	if (!NT_SUCCESS(PsSetLoadImageNotifyRoutine(PloadImageNotifyRoutine)))
	{
		log::PrintError("Can't sign \"PsSetLoadImageNotifyRoutine\".");
		return STATUS_FAILED_DRIVER_ENTRY;
	}

	if (!NT_SUCCESS(PsSetCreateProcessNotifyRoutine(PcreateProcessNotifyRoutine, FALSE)))
	{
		log::PrintError("Can't sign \"PsSetCreateProcessNotifyRoutine\".");
		return STATUS_FAILED_DRIVER_ENTRY;
	}
#endif

	log::PrintInfo("Driver loaded successfully.");
	//return STATUS_UNSUCCESSFUL;
	return STATUS_SUCCESS;
}
