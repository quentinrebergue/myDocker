#ifndef CHILD_H
#define CHILD_H

#include <sys/types.h>

struct child_args {
    char **argv;
    char *rootfs;
    char *volume;  // Host directory to bind mount to /home/student
    int sync_fd;
};

pid_t create_child_with_user_namespace(char **argv, char *rootfs, char *volume);

#endif // CHILD_H