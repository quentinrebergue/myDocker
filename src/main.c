
#define _POSIX_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "child/child.h"
#include "controller/controller.h"
#include "isolation/capabilities.h"
#include "isolation/seccomp.h"
#include "namespace/namespace.h"
#include "request/buildenv.h"
#include "utils/utils.h"

#define PID_BUFF_SIZE 20
#define BUFF 100
#define CHILD_STACK_SIZE (1024 * 1024)
#define CGROUP_MODE 0755

int main(int argc, char **argv)
{
    struct parse_args *args = calloc(1, sizeof(struct parse_args));
    if (!args)
    {
        fprintf(stderr, "malloc parse_args failed\n");
        return 1;
    }
    int return_value = 0;

    // Parsing arguments (argv modified to remove parsed args)
    if (parse_arg(argc, &argv, args) < 0)
    {
        return_value = 1;
        fprintf(stderr, "parse_arg failed\n");
        goto cleanup;
    }

    // Build the environment (rootfs) if repo and tag are provided
    if (args->repo && args->tag
        && (build_env(args->repo, args->tag, &args->rootfs) < 0))
    {
        return_value = 1;
        fprintf(stderr, "build_env failed\n");
        goto cleanup;
    }
    // Check if cgroup child path exists, if not create it
    struct stat st = { 0 };
    if (stat(CGROUP_CHILD_PATH, &st) == -1)
    {
        if (mkdir(CGROUP_CHILD_PATH, CGROUP_MODE) < 0)
        {
            perror("mkdir cgroup");
            return_value = 1;
            goto cleanup;
        }
    }

    // Setup cgroup controllers
    if (setup_controllers() < 0)
    {
        return_value = 1;
        fprintf(stderr, "setup_controllers failed\n");
        goto cleanup;
    }

    // Create child process in new user namespace
    pid_t pid =
        create_child_with_user_namespace(argv, args->rootfs, args->volume);

    if (pid < 0)
    {
        return_value = 1;
        fprintf(stderr, "create_child_with_user_namespace failed\n");
        goto cleanup;
    }

    int status;

    // Wait child process to avoid zombies
    if (waitpid(pid, &status, 0) < 0)
    {
        return_value = 1;
        perror("waitpid");
        goto cleanup;
    }

cleanup:
    free(args->repo);
    free(args->tag);
    free(args->rootfs);
    free(args);
    return return_value;
}