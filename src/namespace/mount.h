#ifndef MOUNT_H
#define MOUNT_H

int unmount_host_fs(void);
int remount_host_fs(void);
int setup_mount_propagation(void);
int setup_volume_mount(const char *host_path, const char *rootfs);

#endif // MOUNT_H