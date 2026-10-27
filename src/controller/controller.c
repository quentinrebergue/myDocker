#include "controller.h"

#include <stdio.h>
#include <sys/types.h>

#include "utils/utils.h"

#define BUFF 100
#define MEMORY_LIMIT "1000000000\n"
#define PIDS_LIMIT "100\n"
#define CPU_LIMIT "100000 100000\n"
#define PATH_SIZE 256
#define PID_BUFF_SIZE 20

/* setup_controller:
 * Setup a cgroup controller with a limit
 */
int setup_controller(char *controller, char *limit)
{
    if (!controller || !limit)
    {
        fprintf(stderr, "setup_controller: empty arguments\n");
        return -1;
    }

    char path[BUFF] = { 0 };

    // Create the path to the controller to modify
    snprintf(path, BUFF, "%s/%s", CGROUP_CHILD_PATH, controller);

    if (write_file(path, limit) < 0)
    {
        fprintf(stderr, "setup_controller: error writing on %s\n", controller);
        return -1;
    }
    return 0;
}

/* setup_controllers:
 * Setup les controllers cpu, memory et pids avec des limites
 */
int setup_controllers(void)
{
    // Enable controllers that our child will be able to use
    write_file("/sys/fs/cgroup/cgroup.subtree_control", "+cpu\n");
    write_file("/sys/fs/cgroup/cgroup.subtree_control", "+memory\n");
    write_file("/sys/fs/cgroup/cgroup.subtree_control", "+pids\n");

    if (setup_controller("memory.max", MEMORY_LIMIT) < 0)
    {
        return -1;
    }
    if (setup_controller("pids.max", PIDS_LIMIT) < 0)
    {
        return -1;
    }
    if (setup_controller("cpu.max", CPU_LIMIT) < 0)
    {
        return -1;
    }
    return 0;
}

/* add_pid_to_cgroup:
 * Ajoute le pid du child dans cgroup.procs de la cgroup cible
 */
int add_pid_to_cgroup(pid_t pid)
{
    char path[PATH_SIZE];
    char pid_buff[PID_BUFF_SIZE];

    snprintf(path, sizeof(path), "%s/cgroup.procs", CGROUP_CHILD_PATH);
    snprintf(pid_buff, sizeof(pid_buff), "%d\n", (int)pid);

    if (write_file(path, pid_buff) < 0)
    {
        fprintf(stderr, "add_pid_to_cgroup: failed to write pid to %s\n", path);
        return -1;
    }
    return 0;
}