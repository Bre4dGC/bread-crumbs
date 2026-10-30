#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli/commands.h"
#include "compiler/context.h"

static int build_handler(int argc, char** argv, void* userdata)
{
    (void)userdata;

    const char* source = NULL;
    const char* output = "a.out";
    bool object_only = false;

    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "-o") == 0 && i + 1 < argc){
            output = argv[++i];
        }
        else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--object") == 0){
            object_only = true;
        }
        else if(argv[i][0] != '-' && !source){
            source = argv[i];
        }
    }

    if(!source){
        fprintf(stderr, "crum: build: missing source file\n");
        fprintf(stderr, "Usage: crum build <source> [-o <output>] [-c]\n");
        return 0;
    }

    compiler_context_t* ctx = new_compiler_context(source);
    if(!ctx){
        fprintf(stderr, "crum: build: failed to create compiler context\n");
        return 1;
    }

    bool ok = object_only ? compile_to_object(ctx, output)
                          : compile_to_executable(ctx, output);
    if(!ok){
        fprintf(stderr, "crum: build: failed to compile '%s'\n", source);
        print_report_table(ctx->reports);
        free_compiler_context(ctx);
        return 1;
    }

    printf("crum: built '%s' -> '%s'\n", source, output);
    free_compiler_context(ctx);
    return 0;
}

void register_build_command(command_list_t* list)
{
    cli_option_set_t opts = new_option_list();
    option_add(&opts, "output", "Output file", CLI_OPTION_VALUE, "FILE", "a.out", false);
    cmd_add(list, "build", "Build a source file into an executable or object file",
            "<source> [-o <output>] [-c]", opts, build_handler, NULL);
}
