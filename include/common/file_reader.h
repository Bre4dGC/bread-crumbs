#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

char* read_file(const char* filepath, size_t* out_size)
{
    if(!filepath){
        fprintf(stderr, "Error: NULL filepath\n");
        return NULL;
    }

    FILE* file = fopen(filepath, "rb");
    if(!file){
        fprintf(stderr, "Error: Cannot open file '%s': %s\n", filepath, strerror(errno));
        return NULL;
    }

    // get size
    if(fseek(file, 0, SEEK_END) != 0){
        fprintf(stderr, "Error: Cannot seek file '%s': %s\n", filepath, strerror(errno));
        fclose(file);
        return NULL;
    }

    long file_size = ftell(file);
    if(file_size < 0){
        fprintf(stderr, "Error: Cannot get file size '%s': %s\n", filepath, strerror(errno));
        fclose(file);
        return NULL;
    }

    if(fseek(file, 0, SEEK_SET) != 0){
        fprintf(stderr, "Error: Cannot rewind file '%s': %s\n", filepath, strerror(errno));
        fclose(file);
        return NULL;
    }

    char* buffer = (char*)malloc(file_size + 1);
    if(!buffer){
        fprintf(stderr, "Error: Cannot allocate %ld bytes for file '%s'\n", file_size + 1, filepath);
        fclose(file);
        return NULL;
    }

    // entire file into buffer
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if(bytes_read != (size_t)file_size){
        if(feof(file)){
            fprintf(stderr, "Error: Unexpected EOF reading '%s'\n", filepath);
        }
        else {
            fprintf(stderr, "Error: Cannot read file '%s': %s\n", filepath, strerror(errno));
        }
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0';
    fclose(file);

    if(out_size) *out_size = file_size;

    return buffer;
}

int is_correct_extension(const char* filepath)
{
    if(!filepath) return 0;

    size_t len = strlen(filepath);
    if(len < 4) return 0;

    return strcmp(filepath + len - 4, ".brc") == 0;
}

int is_file_exists(const char* filepath)
{
    if(!filepath) return 0;

    struct stat st;
    if(stat(filepath, &st) != 0) return 0;

    return S_ISREG(st.st_mode);
}
