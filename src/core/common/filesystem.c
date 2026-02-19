#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "core/strings.h"

string_t fs_read_file(string_pool_t* sp, const char* filepath)
{
    if(!filepath) return (string_t){0};

    FILE* file = fopen(filepath, "rb");
    if(!file) return (string_t){0};

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(filesize < 0){
        fclose(file);
        return (string_t){0};
    }

    char* buffer = malloc(filesize + 1);
    if(!buffer){
        fclose(file);
        return (string_t){0};
    }

    size_t read_size = fread(buffer, 1, filesize, file);
    if(read_size != (size_t)filesize){
        free(buffer);
        fclose(file);
        return (string_t){0};
    }
    buffer[filesize] = '\0'; // null-terminate

    fclose(file);

    string_t result = new_string(sp, buffer);
    free(buffer);

    return result;
}

string_t read_file(string_pool_t* sp, const char* filepath)
{
    return fs_read_file(sp, filepath);
}

int is_file_exists(const char* filepath)
{
    if(!filepath) return 0;

    struct stat st;
    if(stat(filepath, &st) != 0) return 0;

    return S_ISREG(st.st_mode);
}