#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>

#include "core/lang/filesystem.h"

static __thread enum fs_error last_error = FS_OK;

static inline void set_error(enum fs_error err)
{
    last_error = err;
}

static int get_stat(const char* path, struct stat* st)
{
    // TODO: implement
}

string_t fs_read_file(string_pool_t* sp, const char* filepath)
{
    // TODO: implement
}

bool fs_write_file(const char* filepath, const char* content, size_t length)
{
    // TODO: implement
}

bool fs_append_file(const char* filepath, const char* content, size_t length)
{
    // TODO: implement
}

bool fs_delete_file(const char* filepath)
{
    // TODO: implement
}

bool fs_copy_file(const char* src, const char* dst)
{
    // TODO: implement
}

bool fs_move_file(const char* src, const char* dst)
{
    // TODO: implement
}

bool fs_file_exists(const char* filepath)
{
    // TODO: implement
}

bool fs_dir_exists(const char* dirpath)
{
    // TODO: implement
}

enum fs_file_type fs_get_type(const char* path)
{
    // TODO: implement
}

fs_file_info_t fs_get_info(const char* path)
{
    // TODO: implement
}

size_t fs_get_size(const char* filepath)
{
    // TODO: implement
}

bool fs_create_dir(const char* dirpath)
{
    // TODO: implement
}

bool fs_create_dir_recursive(const char* dirpath)
{
    // TODO: implement
}

bool fs_delete_dir(const char* dirpath)
{
    // TODO: implement
}

bool fs_delete_dir_recursive(const char* dirpath)
{
    // TODO: implement
}

fs_dir_list_t* fs_list_dir(const char* dirpath)
{
    // TODO: implement
}

void fs_free_dir_list(fs_dir_list_t* list)
{
    // TODO: implement
}

void fs_path_normalize(char* path)
{
    // TODO: implement
}

void fs_path_join(char* result, const char* base, const char* relative)
{
    // TODO: implement
}

void fs_path_dirname(char* result, const char* path)
{
    // TODO: implement
}

void fs_path_basename(char* result, const char* path)
{
    // TODO: implement
}

void fs_path_extension(char* result, const char* path)
{
    // TODO: implement
}

void fs_path_without_extension(char* result, const char* path)
{
    // TODO: implement
}

bool fs_path_is_absolute(const char* path)
{
    // TODO: implement
}

bool fs_path_is_relative(const char* path)
{
    // TODO: implement
}

bool fs_get_cwd(char* buffer, size_t size)
{
    // TODO: implement
}

bool fs_set_cwd(const char* path)
{
    // TODO: implement
}

bool fs_path_resolve(char* result, const char* path)
{
    // TODO: implement
}

bool fs_path_absolute(char* result, const char* path)
{
    // TODO: implement
}

enum fs_error fs_get_last_error(void)
{
    return last_error;
}

const char* fs_error_string(enum fs_error err)
{
    switch(err){
        case FS_OK:                 return "Success";
        case FS_ERR_NOT_FOUND:      return "File or directory not found";
        case FS_ERR_PERMISSION:     return "Permission denied";
        case FS_ERR_IS_DIRECTORY:   return "Path is a directory";
        case FS_ERR_NOT_DIRECTORY:  return "Path is not a directory";
        case FS_ERR_PATH_TOO_LONG:  return "Path too long";
        case FS_ERR_INVALID_PATH:   return "Invalid path";
        case FS_ERR_IO:             return "I/O error";
        case FS_ERR_OUT_OF_MEMORY:  return "Out of memory";
        default:                    return "Unknown error";
    }
}
