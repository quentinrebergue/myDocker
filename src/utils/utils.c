#define _POSIX_C_SOURCE 200809L

#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

int parse_image(const char *image, char **repo, char **tag)
{
    const char *colon;

    if (!image || !repo || !tag)
        return -1;

    colon = strrchr(image, ':');
    if (!colon || colon == image || *(colon + 1) == '\0')
        return -1;

    *repo = strndup(image, (size_t)(colon - image));
    *tag = strdup(colon + 1);

    if (!*repo || !*tag)
    {
        free(*repo);
        free(*tag);
        return -1;
    }

    return 0;
}

const char *get_host_arch(void)
{
    static struct utsname uts;

    if (uname(&uts) != 0)
        return NULL;

    if (strcmp(uts.machine, "aarch64") == 0)
        return "arm64";

    if (strcmp(uts.machine, "arm64") == 0)
        return "arm64";

    return "amd64";
}

/* write_file:
 * Open et ecrit data dans le fichier path
 */
int write_file(const char *path, const char *data)
{
    if (!path || !data)
    {
        fprintf(stderr, "write_file: NULL args\n");
        return -1;
    }
    int fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    size_t len = strlen(data);
    size_t w = write(fd, data, len);
    close(fd);
    if (w == len)
    {
        return 0;
    }
    perror("write");
    return -1;
}

/* is_directory:
 * Check if the given path exists and is a directory
 */
int is_directory(const char *path)
{
    if (!path)
    {
        fprintf(stderr, "is_directory: NULL path\n");
        return -1;
    }

    struct stat st;
    if (stat(path, &st) < 0)
    {
        fprintf(stderr, "Error: path '%s' does not exist: %s\n", path,
                strerror(errno));
        return -1;
    }

    if (!S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "Error: path '%s' is not a directory\n", path);
        return -1;
    }

    return 0;
}

int parse_arg(int argc, char ***argv, struct parse_args *args)
{
    if (argc < 2)
    {
        printf("Wrong number arg\n");
        return -1;
    }

    int i = 1;

    if (!strncmp((*argv)[i], "-h", 2))
    {
        if (argc != 2)
        {
            printf("Wrong number arg\n");
            return -1;
        }
        printf("MyMoulette, the students' nightmare, now highly secured\n");
        printf("Usage: %s [-v student_workdir] <-I docker-img|-R rootfs-path> "
               "moulette_prog [moulette_arg [...]]\n",
               (*argv)[0]);
        printf("\n");
        printf("rootfs-path is the path to the directory containing the new "
               "rootfs (exclusive with -I option)\n");
        printf("docker-img is an image available on hub.docker.com (exclusive "
               "with -R option)\n");
        printf("moulette_prog will be the first program to be launched, must "
               "already be in the environment\n");
        exit(0);
    }
    else if (!strncmp((*argv)[i], "-v", 2))
    {
        args->volume = (*argv)[i + 1];
        if (is_directory(args->volume) < 0)
        {
            return -1;
        }
        i += 2;
    }

    if (i >= argc)
    {
        printf("Wrong number arg\n");
        return -1;
    }

    if (!strncmp((*argv)[i], "-R", 2))
    {
        args->rootfs = strndup((*argv)[i + 1], strlen((*argv)[i + 1]));
        i += 2;
    }
    else if (!strncmp((*argv)[i], "-I", 2))
    {
        if (parse_image((*argv)[i + 1], &args->repo, &args->tag) < 0)
        {
            return -1;
        }
        i += 2;
    }
    else
    {
        // If we are after a -v
        if (i != 1)
        {
            printf("Wrong number arg\n");
            return -1;
        }
    }

    if (i >= argc)
    {
        printf("Wrong number arg\n");
        return -1;
    }

    *argv = *argv + i;

    return 0;
}
