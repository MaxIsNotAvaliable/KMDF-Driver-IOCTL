#pragma once
#include <ntddk.h>
#include <wdm.h>
#include <initguid.h>
#include "routines.h"

extern "C"

DEFINE_GUID(MyGuid, 0x12345678, 0x9ABC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC);

template<typename T=void>
T* myAlloc(size_t size)
{
	return reinterpret_cast<T*>(ExAllocatePoolWithTag(NonPagedPool, size, (ULONG)&MyGuid));
}

void myFree(void* ptr)
{
	ExFreePoolWithTag(ptr, (ULONG)&MyGuid);
}

extern "C" NTSTATUS NTAPI MmCopyVirtualMemory(
	IN PEPROCESS FromProcess,
	IN CONST VOID * FromAddress,
	IN PEPROCESS ToProcess,
	OUT PVOID ToAddress,
	IN SIZE_T BufferSize,
	IN KPROCESSOR_MODE PreviousMode,
	OUT PSIZE_T NumberOfBytesCopied
);

template<typename T>
BOOLEAN ReadMemory(const PVOID pSrc, HANDLE srcProcId, T* outResult)
{
	if (!pSrc)
		return FALSE;

	if (!srcProcId)
		return FALSE;

	if (!outResult)
		return FALSE;

	PEPROCESS pSrcProcess;

	if (!NT_SUCCESS(LookupProcessByProcessId(srcProcId, &pSrcProcess)))
	{
		log::PrintError("--- Unable to locate process!");
		return FALSE;
	}

	SIZE_T size = 0;
	PEPROCESS pTargetProcess = PsGetCurrentProcess();
	if (!NT_SUCCESS(MmCopyVirtualMemory(pSrcProcess, pSrc, pTargetProcess, outResult, sizeof(T), KernelMode, &size)))
	{
		log::PrintError("--- Unable to copy virtual memory!");
		return FALSE;
	}

	log::Print(">>> Copy memory: size[%ll]", size);
	return TRUE;
}

template<typename T>
BOOLEAN WriteMemory(PVOID pTarget, HANDLE targetProcId, const T& srcData)
{
	if (!pTarget)
		return FALSE;

	if (!targetProcId)
		return FALSE;

	PEPROCESS pTargetProcess;

	if (!NT_SUCCESS(LookupProcessByProcessId(targetProcId, &pTargetProcess)))
	{
		log::PrintError("--- Unable to locate process!");
		return FALSE;
	}

	SIZE_T size = 0;
	PEPROCESS pSrcProcess = PsGetCurrentProcess();
	if (!NT_SUCCESS(MmCopyVirtualMemory(pSrcProcess, &srcData, pTargetProcess, pTarget, sizeof(T), KernelMode, &size)))
	{
		log::PrintError("--- Unable to copy virtual memory!");
		return FALSE;
	}

	log::Print(">>> Memory write succeed: size[%ll]", size);
	return TRUE;
}

BOOLEAN CopyMemory(const PVOID pSrc, HANDLE srcProcId, PVOID pTarget, HANDLE targetProcId, SIZE_T bufferSize)
{
	if (!pSrc)
		return FALSE;

	if (!srcProcId)
		return FALSE;

	if (!pTarget)
		return FALSE;

	if (!targetProcId)
		return FALSE;

	PEPROCESS pTargetProcess, pSrcProcess;

	if (!NT_SUCCESS(LookupProcessByProcessId(srcProcId, &pSrcProcess)) || !NT_SUCCESS(LookupProcessByProcessId(targetProcId, &pTargetProcess)))
	{
		log::PrintError("--- Unable to locate process!");
		return FALSE;
	}

	SIZE_T size = 0;
	if (!NT_SUCCESS(MmCopyVirtualMemory(pSrcProcess, pSrc, pTargetProcess, pTarget, bufferSize, KernelMode, &size)))
	{
		log::PrintError("--- Unable to copy virtual memory!");
		return FALSE;
	}

	log::Print(">>> Copy memory succeed: size[%ll]", size);
	return TRUE;
}