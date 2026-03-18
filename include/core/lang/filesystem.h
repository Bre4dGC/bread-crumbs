#pragma once

#include <stdbool.h>    // bool
#include <stddef.h>     // size_t

#include "core/ds/strings.h"    // string_pool_t

#define FS_MAX_PATH 4096
#define FS_MAX_FILENAME 256

enum fs_file_type {
    FS_TYPE_UNKNOWN,
    FS_TYPE_FILE,
    FS_TYPE_DIRECTORY,
    FS_TYPE_SYMLINK,
    FS_TYPE_NOT_FOUND
};

enum fs_error {
    FS_OK,
    FS_ERR_NOT_FOUND,
    FS_ERR_PERMISSION,
    FS_ERR_IS_DIRECTORY,
    FS_ERR_NOT_DIRECTORY,
    FS_ERR_PATH_TOO_LONG,
    FS_ERR_INVALID_PATH,
    FS_ERR_IO,
    FS_ERR_OUT_OF_MEMORY
};

typedef struct {
    char path[FS_MAX_PATH];
    char name[FS_MAX_FILENAME];
    char extension[32];
    size_t size;
    enum fs_file_type type;
    bool readable;
    bool writable;
    bool executable;
} fs_file_info_t;

typedef struct {
    char name[FS_MAX_FILENAME];
    enum fs_file_type type;
} fs_dir_entry_t;

typedef struct {
    fs_dir_entry_t* entries;
    size_t count;
    size_t capacity;
} fs_dir_list_t;

string_t fs_read_file(string_pool_t* sp, const char* filepath);
bool fs_write_file(const char* filepath, const char* content, size_t length);
bool fs_append_file(const char* filepath, const char* content, size_t length);
bool fs_delete_file(const char* filepath);
bool fs_copy_file(const char* src, const char* dst);
bool fs_move_file(const char* src, const char* dst);

bool fs_file_exists(const char* filepath);
bool fs_dir_exists(const char* dirpath);
enum fs_file_type fs_get_type(const char* path);
fs_file_info_t fs_get_info(const char* path);
size_t fs_get_size(const char* filepath);

bool fs_create_dir(const char* dirpath);
bool fs_create_dir_recursive(const char* dirpath);
bool fs_delete_dir(const char* dirpath);
bool fs_delete_dir_recursive(const char* dirpath);
fs_dir_list_t* fs_list_dir(const char* dirpath);
void fs_free_dir_list(fs_dir_list_t* list);

void fs_path_normalize(char* path);
void fs_path_join(char* result, const char* base, const char* relative);
void fs_path_dirname(char* result, const char* path);
void fs_path_basename(char* result, const char* path);
void fs_path_extension(char* result, const char* path);
void fs_path_without_extension(char* result, const char* path);
bool fs_path_is_absolute(const char* path);
bool fs_path_is_relative(const char* path);

bool fs_get_cwd(char* buffer, size_t size);
bool fs_set_cwd(const char* path);

bool fs_path_resolve(char* result, const char* path);
bool fs_path_absolute(char* result, const char* path);

enum fs_error fs_get_last_error(void);
const char* fs_error_string(enum fs_error err);
