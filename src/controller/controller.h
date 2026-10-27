#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <sys/types.h>

#define CGROUP_CHILD_PATH "/sys/fs/cgroup/mymoulette"

int setup_controllers(void);
int add_pid_to_cgroup(pid_t pid);

#endif //CONTROLLER_H