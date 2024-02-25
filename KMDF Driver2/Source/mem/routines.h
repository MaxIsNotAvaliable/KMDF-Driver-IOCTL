#include <ntddk.h>

extern "C"

struct Routine
{
	typedef PCHAR(*GET_PROCESS_IMAGE_NAME) (PEPROCESS Process);
	static inline GET_PROCESS_IMAGE_NAME gGetProcessImageFileName = nullptr;
	
	typedef HANDLE(*GET_PROCESS_ID) (PEPROCESS Process);
	static inline GET_PROCESS_ID gGetProcessId = nullptr;

	typedef NTSTATUS(*LOOKUP_PROCESS_BY_PROCESS_ID) (HANDLE ProcessId, PEPROCESS* Process);
	static inline LOOKUP_PROCESS_BY_PROCESS_ID gLookupProcessByProcessId = nullptr;

	static inline INT Initialize()
	{
		INT result = 0;
		
		UNICODE_STRING sPsGetProcessImageFileName = RTL_CONSTANT_STRING(L"PsGetProcessImageFileName");
		gGetProcessImageFileName = (GET_PROCESS_IMAGE_NAME)MmGetSystemRoutineAddress(&sPsGetProcessImageFileName);
		if (!gGetProcessImageFileName) result++;

		UNICODE_STRING sPsGetProcessId = RTL_CONSTANT_STRING(L"PsGetProcessId");
		gGetProcessId = (GET_PROCESS_ID)MmGetSystemRoutineAddress(&sPsGetProcessId);
		if (!gGetProcessId) result++;

		UNICODE_STRING sPsLookupProcessByProcessId = RTL_CONSTANT_STRING(L"PsLookupProcessByProcessId");
		gLookupProcessByProcessId = (LOOKUP_PROCESS_BY_PROCESS_ID)MmGetSystemRoutineAddress(&sPsLookupProcessByProcessId);
		if (!gLookupProcessByProcessId) result++;

		return result;
	}
};

static inline PCHAR GetProcessImageFileName(PEPROCESS Process)
{
	return !Routine::gGetProcessImageFileName ? nullptr : Routine::gGetProcessImageFileName(Process);
}

static inline HANDLE GetProcessId(PEPROCESS Process)
{
	return !Routine::gGetProcessId ? nullptr : Routine::gGetProcessId(Process);
}

static inline NTSTATUS LookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process)
{
	return !Routine::gLookupProcessByProcessId ? NULL : Routine::gLookupProcessByProcessId(ProcessId, Process);
}

static inline NTSTATUS GetProcessFromAddress(PVOID address, PEPROCESS* pProcess) {
	HANDLE processId = PsGetProcessId((PEPROCESS)address); // ERROR:
	NTSTATUS status = LookupProcessByProcessId(processId, pProcess);
	return status;
}


//FAULTING_SOURCE_LINE:  C:\dx\Visual Studio Projects\KMDF Driver2\KMDF Driver2\Source\mem\routines.h
//
//FAULTING_SOURCE_FILE:  C:\dx\Visual Studio Projects\KMDF Driver2\KMDF Driver2\Source\mem\routines.h
//
//FAULTING_SOURCE_LINE_NUMBER:  52
//
//FAULTING_SOURCE_CODE:
//	48: 	return !Routine::gLookupProcessByProcessId ? NULL : Routine::gLookupProcessByProcessId(ProcessId, Process);
//	49: }
//	50:
//	51: static inline NTSTATUS GetProcessFromAddress(PVOID address, PEPROCESS* pProcess) {
//> 52: 	HANDLE processId = PsGetProcessId((PEPROCESS)address);
//	53: 	NTSTATUS status = LookupProcessByProcessId(processId, pProcess);
//	54: 	return status;
//	55: }

