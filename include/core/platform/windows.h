#pragma once

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STR "\\"
#define NEWLINE_CHAR '\n'
#define NEWLINE_STR "\n"
#define DLL_EXTENSION ".dll"
#define DLL_PREFIX ""
#define EXPORT_SYMBOL __declspec(dllexport)
