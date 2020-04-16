#include "load_kernel.h"
#include "memory_map.h"
#include "templeos-loader.h"
#include "vfs.h"

#include <argtable3.h>

#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
	//printf("pid: %d\n", getpid());
    //printf("pc: %p\n", get_pc());

    struct arg_lit *help;
    struct arg_str* arg_drive;
    struct arg_file* arg_kernel;
    struct arg_end *end;

	/* init memory map -- do this before anything else, particularly any mallocs */
	if (init_memory_map(0, 0, 0) < 0)
        exit(-1);

    void* argtable[] = {
        help = arg_lit0(NULL, "help", "display this help and exit"),
        arg_drive = arg_strn(NULL, "drive", "letter,dir[,writedir]", 0, 26, "expose host directories as virtual drives"),
        //arg_drive = arg_rexn(NULL, "drive", "^([A-Z]),([^,]+)(,[^,]+)?$", "letter,dir,writedir", 0, 26, 0, "expose host directories as virtual drives"),
        end = arg_end(20),
    };

    int exitcode = 0;
    char progname[] = "templeos-loader";

    int nerrors;
    nerrors = arg_parse(argc, argv, argtable);

    /* special case: '--help' takes precedence over error reporting */
    if (help->count > 0)
    {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("Demonstrate command-line parsing in argtable3.\n\n");
        arg_print_glossary(stdout, argtable, "  %-25s %s\n");
        exitcode = 0;
        goto exit;
    }

    /* If the parser returned any errors then display them and exit */
    if (nerrors > 0)
    {
        /* Display the error details contained in the arg_end struct.*/
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        exitcode = 1;
        goto exit;
    }

    /* Init virtual drives. Warning: crappy parsing ahead! refactoring needed. */
    bool have_C = false, have_D = false;

    for (int i = 0; i < arg_drive->count; i++) {
        char* comma1 = strchr(arg_drive->sval[i], ',');

        if (!comma1 || comma1 != &arg_drive->sval[i][1]) {
            fprintf(stderr, "drive spec syntax error: %s\n", arg_drive->sval[i]);
            exitcode = -1;
            goto exit;
        }

        *comma1 = 0;

        char let = arg_drive->sval[i][0];
        char* dir = comma1 + 1;
        char* writedir = NULL;

        char* comma2 = strchr(dir, ',');
        if (comma2) {
            *comma2 = 0;
            writedir = comma2 + 1;
        }

        // TODO: when we allow >2 drives, idx should be just dynamically assigned
        // (kernel will call HostGetOptionDriveList to discover)
        int idx;
        if (let == 'C') { idx = 0; have_C = true; }
        else if (let == 'D') { idx = 1; have_D = true; }
        else {
            fprintf(stderr, "only C and D drives can be mounted for the time being\n");
            exitcode = -1;
            goto exit;
        }

        // TODO: sanity check for re-definition of the same idx
        vfs_init(argv[0], dir, writedir, idx);
    }

    if (!have_C) { vfs_init(argv[0], NULL, NULL, 0); }
    if (!have_D) { vfs_init(argv[0], NULL, NULL, 1); }

    install_trap_handlers();

    __asm__(
        "movq $vsyscall_dispatcher, _VSYSCALL_DISPATCHER$Temple(%rip)\r\n"
        "call InitRuntime$Temple\r\n"
    );

exit:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return exitcode;
}
