#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "cli/core.h"
#include "cli/register.h"

static int count_matching(const char* output, const char* pattern)
{
    return strstr(output, pattern) != NULL ? 1 : 0;
}

int main(void)
{
    /* Test find_command returns NULL for no match */
    {
        cli_t cli = new_cli(0, NULL);
        register_commands(&cli);

        const char* expected[] = {
            "run", "build", "check", "repl", "doc", "new",
            "init", "debug", "test", "fmt", "install", "add",
            "remove", "help", "version"
        };
        int n = sizeof(expected) / sizeof(expected[0]);
        for (int i = 0; i < n; i++){
            command_t* cmd = find_command(&cli, expected[i]);
            assert(cmd != NULL);
            assert(strcmp(cmd->name.data, expected[i]) == 0);
        }

        command_t* cmd = find_command(&cli, "nonexistent");
        assert(cmd == NULL);

        free_cli(&cli);
    }

    /* Test --version flag */
    {
        char* argv[] = {"crum", "--version"};
        cli_t cli = new_cli(2, argv);
        register_commands(&cli);
        assert(cli_run(&cli) == 0);
        free_cli(&cli);
    }

    /* Test -V short flag */
    {
        char* argv[] = {"crum", "-V"};
        cli_t cli = new_cli(2, argv);
        register_commands(&cli);
        assert(cli_run(&cli) == 0);
        free_cli(&cli);
    }

    /* Test --help flag */
    {
        char* argv[] = {"crum", "--help"};
        cli_t cli = new_cli(2, argv);
        register_commands(&cli);
        assert(cli_run(&cli) == 0);
        free_cli(&cli);
    }

    /* Test -h short flag */
    {
        char* argv[] = {"crum", "-h"};
        cli_t cli = new_cli(2, argv);
        register_commands(&cli);
        assert(cli_run(&cli) == 0);
        free_cli(&cli);
    }

    /* Test no args shows help */
    {
        char* argv[] = {"crum"};
        cli_t cli = new_cli(1, argv);
        register_commands(&cli);
        assert(cli_run(&cli) == 0);
        free_cli(&cli);
    }

    /* Test help for specific command */
    {
        char* argv[] = {"crum", "--help", "run"};
        cli_t cli = new_cli(3, argv);
        register_commands(&cli);
        assert(cli_run(&cli) == 0);
        free_cli(&cli);
    }

    /* Test unknown command returns error */
    {
        char* argv[] = {"crum", "nonexistent"};
        cli_t cli = new_cli(2, argv);
        register_commands(&cli);
        assert(cli_run(&cli) == 1);
        free_cli(&cli);
    }

    /* Test global options -q, -v, -o */
    {
        char* argv[] = {"crum", "-v", "-q", "-o", "out.txt", "run", "test.brc"};
        cli_t cli = new_cli(7, argv);
        register_commands(&cli);
        assert(cli_run(&cli) == 0);
        free_cli(&cli);
    }

    /* Test all 15 commands dispatch correctly */
    struct { const char* name; char** argv; int argc; int expected; } tests[] = {
        {"run",    (char*[]){"crum", "run", "test.brc"}, 3, 0},
        {"run",    (char*[]){"crum", "run"}, 2, 1},
        {"build",  (char*[]){"crum", "build"}, 2, 0},
        {"check",  (char*[]){"crum", "check", "src.brc"}, 3, 0},
        {"repl",   (char*[]){"crum", "repl"}, 2, 0},
        {"doc",    (char*[]){"crum", "doc", "run"}, 3, 0},
        {"doc",    (char*[]){"crum", "doc"}, 2, 1},
        {"new",    (char*[]){"crum", "new", "myapp"}, 3, 0},
        {"new",    (char*[]){"crum", "new"}, 2, 1},
        {"init",   (char*[]){"crum", "init"}, 2, 0},
        {"debug",  (char*[]){"crum", "debug", "x.brc"}, 3, 0},
        {"test",   (char*[]){"crum", "test"}, 2, 0},
        {"fmt",    (char*[]){"crum", "fmt", "x.brc"}, 3, 0},
        {"fmt",    (char*[]){"crum", "fmt"}, 2, 1},
        {"install",(char*[]){"crum", "install", "pkg"}, 3, 0},
        {"install",(char*[]){"crum", "install"}, 2, 1},
        {"add",    (char*[]){"crum", "add", "dep"}, 3, 0},
        {"add",    (char*[]){"crum", "add"}, 2, 1},
        {"remove", (char*[]){"crum", "remove", "dep"}, 3, 0},
        {"remove", (char*[]){"crum", "remove"}, 2, 1},
    };

    int n_tests = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n_tests; i++){
        cli_t cli = new_cli(tests[i].argc, tests[i].argv);
        register_commands(&cli);
        int result = cli_run(&cli);
        assert(result == tests[i].expected);
        free_cli(&cli);
    }

    printf("All CLI tests passed!\n");
    return 0;
}
