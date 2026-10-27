/* test_syscalls.c - Minimal seccomp filter test */
#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

int main(void)
{
    int failed = 0;
    
    /* Test blocked syscalls - expect EPERM from seccomp */
    // personnality syscall
    syscall(135, 0xffffffff);
    if (errno != EPERM)
        failed = 1;
    
    // pivot root syscall
    syscall(155, "/tmp", "/tmp");
    if (errno != EPERM)
        failed = 1;
    
    //nfsservctl syscall
    syscall(169, 0, 0, 0);
    if (errno != EPERM && errno != ENOSYS)
        failed = 1;
    
    /* Test normal syscall works */
    if (getpid() <= 0)
        failed = 1;
    
    printf(failed ? "Test NOK\n" : "Test OK\n");
    return failed;
}
