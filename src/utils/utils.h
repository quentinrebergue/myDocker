#ifndef UTILS_H
#define UTILS_H

// Structure to hold parsed command-line arguments
struct parse_args {
    char *rootfs;
    char *repo;
    char *tag;
    char *volume;
};

int write_file(const char *path, const char *data);
int parse_arg(int argc, char ***argv, struct parse_args *args);
const char *get_host_arch(void);


#endif //UTILS_H