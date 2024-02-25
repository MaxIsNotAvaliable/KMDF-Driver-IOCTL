#pragma once

#include <ntddk.h>
#include <cstdarg>

class log
{
private:
	static ULONG PrintFormatted(const char* prefix, const char* format, const va_list& args);
	
public:
	static ULONG Print(const char* format, ...);
	static ULONG PrintInfo(const char* format, ...);
	static ULONG PrintWarning(const char* format, ...);
	static ULONG PrintError(const char* format, ...);
};

