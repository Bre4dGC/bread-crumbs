#pragma once

#include "core/strings.h"

string_t fs_read_file(string_pool_t* sp, const char* filepath);
int is_file_exists(const char* filepath);