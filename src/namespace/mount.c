#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#define PROC_SYS_MODE 0555
#define DEV_MODE 0755
#define TMP_MODE 01777
#define DEV_MOUNT_MODE "mode=755"
#define TMP_MOUNT_MODE "mode=1777"
#define HOME_STUDENT_MODE 0755
#define HOME_STUDENT_PATH_SIZE 256

static const char *unmount_fs[] = { "/proc", "/sys", "/dev",   "/tmp",
                                    "/run",  "/var", "/boot",  "/home",
                                    "/root", "/mnt", "/media", NULL };

int mount_procfs(void)
{
    // Ensure /proc exists
    if (access("/proc", F_OK) != 0)
    {
        if (mkdir("/proc", PROC_SYS_MODE) < 0)
        {
            perror("mkdir /proc");
            return -1;
        }
    }

    if (mount("proc", "/proc", "proc", MS_NOSUID | MS_NODEV | MS_NOEXEC, NULL)
        < 0)
    {
        perror("mount proc");
        return -1;
    }
    return 0;
}

int mount_sysfs(void)
{
    // Ensure /sys exists
    if (access("/sys", F_OK) != 0)
    {
        if (mkdir("/sys", PROC_SYS_MODE) < 0)
        {
            perror("mkdir /sys");
            return -1;
        }
    }

    if (mount("sysfs", "/sys", "sysfs", MS_NOSUID | MS_NODEV | MS_NOEXEC, NULL)
        < 0)
    {
        perror("mount sys");
        return -1;
    }
    return 0;
}

int mount_devfs(void)
{
    // Ensure /dev exists
    if (access("/dev", F_OK) != 0)
    {
        if (mkdir("/dev", DEV_MODE) < 0)
        {
            perror("mkdir /dev");
            return -1;
        }
    }

    if (mount("tmpfs", "/dev", "tmpfs", MS_NOSUID | MS_NOEXEC, DEV_MOUNT_MODE)
        < 0)
    {
        perror("mount dev");
        return -1;
    }

    return 0;
}

int mount_tmpfs(void)
{
    // Ensure /tmp exists
    if (access("/tmp", F_OK) != 0)
    {
        if (mkdir("/tmp", 01777) < 0)
        {
            perror("mkdir /tmp");
            return -1;
        }
    }

    if (mount("tmpfs", "/tmp", "tmpfs", MS_NOSUID | MS_NODEV, TMP_MOUNT_MODE)
        < 0)
    {
        perror("mount tmp");
        return -1;
    }
    return 0;
}

// Unmount filesystems in /
int unmount_host_fs(void)
{
    for (int i = 0; unmount_fs[i] != NULL; i++)
    {
        umount2(unmount_fs[i], MNT_DETACH);
    }

    return 0;
}

int remount_host_fs(void)
{
    if (mount_procfs() < 0)
    {
        perror("mount_procfs");
        return -1;
    }
    if (mount_sysfs() < 0)
    {
        perror("mount_sysfs");
        return -1;
    }
    if (mount_devfs() < 0)
    {
        perror("mount_devfs");
        return -1;
    }
    if (mount_tmpfs() < 0)
    {
        perror("mount_tmpfs");
        return -1;
    }
    return 0;
}

// Setup mount propagation to private recursively
int setup_mount_propagation(void)
{
    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) < 0)
    {
        perror("mount");
        return -1;
    }
    return 0;
}

// Setup volume bind mount from host to /home/student
// If rootfs is provided, mount to rootfs/home/student (before chroot)
// If rootfs is NULL, mount to /home/student (after chroot)
int setup_volume_mount(const char *host_path, const char *rootfs)
{
    if (!host_path)
    {
        return 0;
    }

    char home_path[HOME_STUDENT_PATH_SIZE];
    char student_path[HOME_STUDENT_PATH_SIZE];

    // Build paths depending on whether we are before or after chroot
    if (rootfs)
    {
        snprintf(home_path, sizeof(home_path), "%s/home", rootfs);
        snprintf(student_path, sizeof(student_path), "%s/home/student", rootfs);
    }
    else
    {
        snprintf(home_path, sizeof(home_path), "/home");
        snprintf(student_path, sizeof(student_path), "/home/student");
    }

    // Create /home directory if it doesn't exist
    if (mkdir(home_path, HOME_STUDENT_MODE) < 0)
    {
        if (errno != EEXIST)
        {
            fprintf(stderr, "mkdir %s failed: %s\n", home_path,
                    strerror(errno));
            // Continue anyway, /home might exist
        }
    }

    // Create /home/student directory if it doesn't exist
    if (mkdir(student_path, HOME_STUDENT_MODE) < 0)
    {
        if (errno != EEXIST)
        {
            fprintf(stderr, "mkdir %s failed: %s\n", student_path,
                    strerror(errno));
            return -1;
        }
    }

    // Bind mount the host directory to /home/student
    if (mount(host_path, student_path, NULL, MS_BIND | MS_REC, NULL) < 0)
    {
        fprintf(stderr, "bind mount %s to %s failed: %s\n", host_path,
                student_path, strerror(errno));
        return -1;
    }

    return 0;
}