#include <ntddk.h>
#include <cr0.h>
#include <Detour.h>
#include <ntos.h>

using WinLibKernel::Mem::cr0;
using WinLibKernel::Mem::Hook::Detour;
using WinLibKernel::NTOS::NTOS;

Detour* detour;

typedef ULONG(__cdecl *dbgprint)(_In_z_ _Printf_format_string_ PCSTR Format, ...);
dbgprint pDbgPrint;

ULONG __cdecl MyDbgPrint(_In_z_ _Printf_format_string_ PCSTR Format, ...) {
	UNREFERENCED_PARAMETER(Format);
	return pDbgPrint("=> hooked from Kernel");
}

VOID KernelMemManipulation() {
	PCHAR addr;
	UNICODE_STRING apiname;

	RtlInitUnicodeString(&apiname, L"DbgPrint");
	addr = (PCHAR)MmGetSystemRoutineAddress(&apiname);

	detour = new (NonPagedPool) Detour((UINT8*)addr, (UINT8*)&MyDbgPrint);
	detour->hook();
	pDbgPrint = (dbgprint)detour->getTrampoline();
}

VOID GetModuleInformation() {
	auto module_info = NTOS::GetSystemModuleInformation("\\SystemRoot\\system32\\ntoskrnl.exe");

	if (module_info) {
		delete module_info;
	}
}

VOID OnUnload(IN PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);

	DbgPrint("=> OnUnload called");
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	DbgPrint("=> DriverEntry called");
	DriverObject->DriverUnload = OnUnload;

	//KernelMemManipulation();
	GetModuleInformation();

	return STATUS_SUCCESS;
}