#ifndef NAMESPACE_H
#define NAMESPACE_H

#include <sys/types.h>

int setup_parent_namespaces(void);
int setup_child_namespaces(void);
int mount_procfs(void);
int umount_host_fs(void);
int setup_parent_user_namespace(pid_t child_pid);
int setup_hostname(void);

#endif // NAMESPACE_H