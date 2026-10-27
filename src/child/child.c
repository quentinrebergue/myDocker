#define _GNU_SOURCE

#include "child.h"

#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <unistd.h>

#include "controller/controller.h"
#include "isolation/capabilities.h"
#include "isolation/seccomp.h"
#include "namespace/namespace.h"
#include "namespace/setup_rootfs.h"

// Stack size for cloned child
#define STACK_SIZE (1024 * 1024)
#define SYNC_TOKEN "\x01"
#define SYNC_TOKEN_SIZE 1

/* child_program:
 * Entry point for the child process created with clone().
 */
int child_program(void *arg)
{
    struct child_args *a = arg;
    char token;
    // Wait for parent to finish UID/GID mappings
    if (read(a->sync_fd, &token, SYNC_TOKEN_SIZE) != SYNC_TOKEN_SIZE)
    {
        fprintf(stderr, "child_program: sync read failed\n");
        return -1;
    }
    close(a->sync_fd);

    // Setup other namespaces in child
    if (setup_child_namespaces() < 0)
    {
        fprintf(stderr, "setup_child_namespaces failed\n");
        return -1;
    }

    // Setup random hostname
    if (setup_hostname() < 0)
    {
        fprintf(stderr, "setup_hostname failed\n");
        return -1;
    }

    // Setup rootfs, if no rootfs continue
    if (setup_rootfs(a->rootfs, a->volume) < 0)
    {
        fprintf(stderr, "setup_rootfs failed\n");
        return -1;
    }

    if (setup_capabilities() < 0)
    {
        fprintf(stderr, "setup_capabilities failed\n");
        return -1;
    }

    if (setup_seccomp() < 0)
    {
        perror("seccomp");
        return -1;
    }

    execvp(a->argv[0], a->argv);

    perror("execvp");
    return -1;
}

/* create_child_with_user_namespace:
 * Create the child with clone and setup user namespace.
 * Child is synchronized with parent with a pipe and waits for UID/GID mappings.
 */
pid_t create_child_with_user_namespace(char **argv, char *rootfs, char *volume)
{
    char *child_stack = NULL;
    pid_t return_value = -1;
    int sync_pipe[2];

    // We create a pipe to synchronize parent and child
    // Because child should wait for parent to setup UID/GID mappings before
    // continuing
    if (pipe(sync_pipe) < 0)
    {
        perror("pipe");
        return -1;
    }

    struct child_args args = { .argv = argv,
                               .rootfs = rootfs,
                               .volume = volume,
                               .sync_fd = sync_pipe[0] };

    child_stack = malloc(STACK_SIZE * sizeof(char));
    if (!child_stack)
    {
        perror("malloc child_stack");
        return_value = -1;
        goto cleanup;
    }

    // Create child in new user, PID and cgroup namespaces
    pid_t pid =
        clone(child_program, child_stack + STACK_SIZE,
              CLONE_NEWUSER | CLONE_NEWPID | CLONE_NEWCGROUP | SIGCHLD, &args);
    if (pid < 0)
    {
        perror("clone");
        return_value = -1;
        goto cleanup;
    }

    // We are in the parent

    // Setup UID/GID mappings for child
    if (setup_parent_user_namespace(pid) < 0)
    {
        fprintf(stderr, "setup_parent_user_namespace failed\n");
        kill(pid, SIGKILL);
        return_value = -1;
        goto cleanup;
    }

    // Add child to cgroup before signaling it to continue
    if (add_pid_to_cgroup(pid) < 0)
    {
        fprintf(stderr, "add_pid_to_cgroup failed\n");
        kill(pid, SIGKILL);
        return_value = -1;
        goto cleanup;
    }

    // Signal child to proceed
    if (write(sync_pipe[1], SYNC_TOKEN, SYNC_TOKEN_SIZE) != SYNC_TOKEN_SIZE)
    {
        fprintf(stderr, "sync signal write failed\n");
        kill(pid, SIGKILL);
        return_value = -1;
        goto cleanup;
    }

    return_value = pid;

cleanup:
    close(sync_pipe[0]);
    close(sync_pipe[1]);
    if (child_stack)
    {
        free(child_stack);
    }
    return return_value;
}