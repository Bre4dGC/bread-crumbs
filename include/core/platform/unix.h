#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/wait.h>

#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"
#define NEWLINE_CHAR '\n'
#define NEWLINE_STR "\n"
#define DLL_EXTENSION ".so"
#define DLL_PREFIX "lib"
#define EXPORT_SYMBOL __attribute__((visibility("default")))
