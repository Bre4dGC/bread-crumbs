#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compiler/context.h"

static bool write_source(const char* path)
{
    FILE* file = fopen(path, "w");
    if(!file) return false;

    fputs("func main() {\n"
          "    print(\"hello\")\n"
          "    print(42)\n"
          "    print(true)\n"
          "    write(\"a\")\n"
          "    write(\"b\")\n"
          "    print(\"\")\n"
          "}\n", file);

    fclose(file);
    return true;
}

int main(void)
{
    char source_path[] = "/tmp/brc_io_src_XXXXXX";
    int source_fd = mkstemp(source_path);
    if(source_fd < 0){
        fprintf(stderr, "io: failed to create temporary source\n");
        return 1;
    }
    close(source_fd);

    if(!write_source(source_path)){
        fprintf(stderr, "io: failed to write temporary source\n");
        unlink(source_path);
        return 1;
    }

    char executable_path[] = "/tmp/brc_io_exe_XXXXXX";
    int executable_fd = mkstemp(executable_path);
    if(executable_fd < 0){
        fprintf(stderr, "io: failed to create temporary executable path\n");
        unlink(source_path);
        return 1;
    }
    close(executable_fd);
    unlink(executable_path);

    compiler_context_t* ctx = new_compiler_context(source_path);
    if(!ctx){
        fprintf(stderr, "io: failed to create compiler context\n");
        unlink(source_path);
        return 1;
    }

    bool built = compile_to_executable(ctx, executable_path);
    free_compiler_context(ctx);

    if(!built){
        fprintf(stderr, "io: failed to build test program\n");
        unlink(source_path);
        return 1;
    }

    FILE* process = popen(executable_path, "r");
    if(!process){
        fprintf(stderr, "io: failed to run test program\n");
        unlink(source_path);
        unlink(executable_path);
        return 1;
    }

    char output[256] = {0};
    size_t length = fread(output, 1, sizeof(output) - 1, process);
    output[length] = '\0';
    int status = pclose(process);

    unlink(source_path);
    unlink(executable_path);

    /* print appends one newline, write appends none (see docs/runtime.md). */
    const char* expected = "hello\n42\ntrue\nab\n";
    if(status != 0 || strcmp(output, expected) != 0){
        fprintf(stderr, "io: unexpected output\n  expected: %s\n  actual:   %s\n",
                expected, output);
        return 1;
    }

    printf("All runtime I/O tests passed!\n");
    return 0;
}
