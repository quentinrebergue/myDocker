#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "namespace/mount.h"
#include "utils/utils.h"

#define HOSTNAME_LENGTH 13
#define PATH_BUFFER_SIZE 256
#define UID_MAP_SIZE 128
#define PID_SHIFT_BITS 16

static const char charset[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// We separe the setup of PID namespace because it requires a fork after unshare
// So it will be executed by the parent process, the child will have the PID 1
int setup_parent_namespaces(void)
{
    if (unshare(CLONE_NEWPID) < 0)
    {
        perror("unshare");
        return -1;
    }
    return 0;
}

// The other namespaces can be setup together in the child process
int setup_child_namespaces(void)
{
    int flags = CLONE_NEWUTS | CLONE_NEWNS | CLONE_NEWIPC | CLONE_NEWNET;

    if (unshare(flags) < 0)
    {
        perror("unshare");
        return -1;
    }
    return setup_mount_propagation();
}

int setup_parent_user_namespace(pid_t child_pid)
{
    char path[PATH_BUFFER_SIZE];
    uid_t uid = getuid();
    gid_t gid = getgid();
    char map[UID_MAP_SIZE];

    // Deny in setgroups
    snprintf(path, sizeof(path), "/proc/%d/setgroups", child_pid);
    if (write_file(path, "deny\n") < 0)
    {
        fprintf(stderr, "write_file setgroups failed\n");
        return -1;
    }

    // UID user mapping
    snprintf(path, sizeof(path), "/proc/%d/uid_map", child_pid);
    snprintf(map, sizeof(map), "0 %d 1\n", uid);
    if (write_file(path, map) < 0)
    {
        fprintf(stderr, "write_file uid_map failed\n");
        return -1;
    }

    // GID group mapping
    snprintf(path, sizeof(path), "/proc/%d/gid_map", child_pid);
    snprintf(map, sizeof(map), "0 %d 1\n", gid);
    if (write_file(path, map) < 0)
    {
        fprintf(stderr, "write_file gid_map failed\n");
        return -1;
    }
    return 0;
}

// Generate a random hostname
int setup_hostname(void)
{
    char hostname[HOSTNAME_LENGTH];

    // Seed random number generator
    srand(time(NULL) ^ (getpid() << PID_SHIFT_BITS));

    // Generate random hostname
    for (int i = 0; i < HOSTNAME_LENGTH - 1; i++)
    {
        hostname[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    hostname[HOSTNAME_LENGTH - 1] = '\0';

    // Set the hostname
    if (sethostname(hostname, HOSTNAME_LENGTH) < 0)
    {
        perror("sethostname");
        return -1;
    }

    return 0;
}