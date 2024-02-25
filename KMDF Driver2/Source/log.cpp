#include "log.h"
#include "mem/memorymanager.h"

ULONG log::PrintFormatted(const char* prefix, const char* format, const va_list &args)
{
	DbgPrintEx(0, 0, "[%s] ", prefix);

	ULONG result = vDbgPrintEx(0, 0, format, args);

	DbgPrintEx(0, 0, "\n");
	return result;
}

ULONG log::Print(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	ULONG result = PrintFormatted("driver_log", format, args);
	va_end(args);

	return result;
}

ULONG log::PrintInfo(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	ULONG result = PrintFormatted("INFO", format, args);
	va_end(args);

	return result;
}

ULONG log::PrintWarning(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	ULONG result = PrintFormatted("WARNING", format, args);
	va_end(args);

	return result;
}

ULONG log::PrintError(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	ULONG result = PrintFormatted("ERROR", format, args);
	va_end(args);

	return result;
}
