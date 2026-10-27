#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "namespace/mount.h"

// Toggle between chroot and pivot_root implementations
// Set to 1 for pivot_root (Palier 9), 0 for chroot (Palier 3)
#define USE_PIVOT_ROOT 1
#define OLD_ROOT_SIZE 256
#define OLD_ROOT_MODE 0755

// pivot_root syscall wrapper
static int pivot_root(const char *new_root, const char *put_old)
{
    return syscall(SYS_pivot_root, new_root, put_old);
}

// Setup root filesystem with chroot (Palier 3)
int setup_rootfs_with_chroot(const char *rootfs)
{
    // Change root filesystem using chroot
    if (chroot(rootfs) < 0)
    {
        fprintf(stderr, "chroot(%s) failed: %s\n", rootfs, strerror(errno));
        return -1;
    }

    // Change to the new root directory
    if (chdir("/") < 0)
    {
        fprintf(stderr, "chdir(/) failed: %s\n", strerror(errno));
        return -1;
    }

    // Unmount host filesystems (Palier 7)
    if (unmount_host_fs() < 0)
    {
        fprintf(stderr, "unmount_host_fs failed\n");
        return -1;
    }

    // Remount isolated filesystems (Palier 7)
    if (remount_host_fs() < 0)
    {
        fprintf(stderr, "remount_host_fs failed\n");
        return -1;
    }

    return 0;
}

// Setup root filesystem with pivot_root (Palier 9)
int setup_rootfs_with_pivot_root(const char *rootfs)
{
    // Change root filesystem using pivot_root
    if (mount(rootfs, rootfs, NULL, MS_BIND | MS_REC, NULL) < 0)
    {
        perror("mount");
        return -1;
    }

    // Create a directory for the old root
    char old_root[OLD_ROOT_SIZE];
    snprintf(old_root, sizeof(old_root), "%s/.old_root", rootfs);
    if (mkdir(old_root, OLD_ROOT_MODE) < 0 && errno != EEXIST)
    {
        perror("mkdir old_root");
        return -1;
    }

    // Change to new root directory
    if (chdir(rootfs) < 0)
    {
        perror("chdir new root");
        return -1;
    }

    // Perform pivot_root
    if (pivot_root(".", ".old_root") < 0)
    {
        fprintf(stderr, "pivot_root failed: %s\n", strerror(errno));
        return -1;
    }

    // Change to the new root
    if (chdir("/") < 0)
    {
        fprintf(stderr, "chdir(/) after pivot_root failed: %s\n",
                strerror(errno));
        return -1;
    }
    // Remount isolated filesystems (Palier 7)
    if (remount_host_fs() < 0)
    {
        fprintf(stderr, "remount_host_fs failed\n");
        return -1;
    }

    // Unmount the old root to completely remove access to host filesystem
    if (umount2("/.old_root", MNT_DETACH) < 0)
    {
        fprintf(stderr, "umount /.old_root failed: %s\n", strerror(errno));
    }

    // Remove the old_root directory
    if (rmdir("/.old_root") < 0)
    {
        fprintf(stderr, "rmdir /.old_root failed: %s\n", strerror(errno));
    }

    return 0;
}

// Setup root filesystemw
int setup_rootfs(const char *rootfs, const char *volume)
{
    if (!rootfs)
    {
        return 0;
    }
    // Setup volume bind mount before change rootfs
    if (setup_volume_mount(volume, rootfs) < 0)
    {
        fprintf(stderr, "setup_volume_mount failed\n");
        return -1;
    }

    int return_value = 0;
    if (USE_PIVOT_ROOT)
    {
        return_value = setup_rootfs_with_pivot_root(rootfs);
    }
    else
    {
        return_value = setup_rootfs_with_chroot(rootfs);
    }
    if (return_value < 0)
    {
        return -1;
    }

    return 0;
}