#include <errno.h>
#include <seccomp.h>
#include <stdio.h>

/* setup_seccomp:
 * Setup seccomp pour bloquer les appels systemes nfsservctl, personality et
 * pivot_root
 */
int setup_seccomp(void)
{
    scmp_filter_ctx ctx;

    ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (!ctx)
    {
        fprintf(stderr, "seccomp_init failed\n");
        return -1;
    }

    if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(nfsservctl), 0)
        < 0)
    {
        fprintf(stderr, "seccomp_rule_add nfsservctl failed\n");
        seccomp_release(ctx);
        return -1;
    }

    if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(personality), 0)
        < 0)
    {
        fprintf(stderr, "seccomp_rule_add personality failed\n");
        seccomp_release(ctx);
        return -1;
    }

    if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(pivot_root), 0)
        < 0)
    {
        fprintf(stderr, "seccomp_rule_add pivot_root failed\n");
        seccomp_release(ctx);
        return -1;
    }

    if (seccomp_load(ctx) < 0)
    {
        fprintf(stderr, "seccomp_load failed\n");
        seccomp_release(ctx);
        return -1;
    }

    seccomp_release(ctx);
    return 0;
}