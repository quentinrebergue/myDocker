#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/personality.h>

int main(void)
{
    int failed = 0;
    
    errno = 0;
    personality(0xffffffff);
    if (errno != EPERM) {
        printf("personality(): expected EPERM, got errno=%d\n", errno);
        failed = 1;
    } else {
        printf("personality() got EPERM as expected\n", errno);
    }
    
    errno = 0;
    syscall(SYS_pivot_root, "/tmp", "/tmp");
    if (errno != EPERM) {
        printf("pivot_root(): expected EPERM, got errno=%d\n", errno);
        failed = 1;
    } else {
        printf("pivot_root() got EPERM as expected\n", errno);
    }
    
    pid_t pid = getpid();
    if (pid <= 0) {
        printf("getpid() returned %d (wrong pid)\n", pid);
        failed = 1;
    } else {
        printf("getpid() returned %d (right pid)\n", pid);
    }
    
    printf(failed ? "Test NOK\n" : "Test OK\n");
    return failed;
}
