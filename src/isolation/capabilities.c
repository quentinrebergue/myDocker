#include <stdio.h>
#include <sys/capability.h>
#include <sys/prctl.h>

/* setup_bounding_ambiant_caps:
 * Removes all bounding and ambient capabilities except CAP_NET_RAW.
 */
int setup_bounding_ambiant_caps()
{
    // Drop all capabilities from the bounding set except CAP_NET_RAW
    for (int cap = 0; cap <= CAP_LAST_CAP; cap++)
    {
        if (cap == CAP_NET_RAW)
        {
            continue;
        }
        // Drop the capability from the bounding set
        if (prctl(PR_CAPBSET_DROP, cap, 0, 0, 0) < 0)
        {
            perror("prctl PR_CAPBSET_DROP");
            return -1;
        }
    }
    // Clear all ambient capabilities
    if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0) < 0)
    {
        perror("prctl PR_CAP_AMBIENT_CLEAR_ALL");
        return -1;
    }
    return 0;
}

/* setup_permitted_effective_caps:
 * Configure the permitted and effective capabilities so that only CAP_NET_RAW
 * is allowed.
 */
int setup_permitted_effective_caps(void)
{
    cap_t caps = cap_init();
    if (!caps)
    {
        perror("cap_init");
        return -1;
    }
    cap_value_t raw_sock_cap = CAP_NET_RAW;

    // Set effective capabilities to CAP_NET_RAW
    if (cap_set_flag(caps, CAP_EFFECTIVE, 1, &raw_sock_cap, CAP_SET) < 0)
    {
        perror("cap_set_flag EFFECTIVE");
        cap_free(caps);
        return -1;
    }
    // Set permitted capabilities to CAP_NET_RAW
    if (cap_set_flag(caps, CAP_PERMITTED, 1, &raw_sock_cap, CAP_SET) < 0)
    {
        perror("cap_set_flag PERMITTED");
        cap_free(caps);
        return -1;
    }

    // Apply the capabilities to the process
    if (cap_set_proc(caps) < 0)
    {
        perror("cap_set_proc");
        cap_free(caps);
        return -1;
    }

    cap_free(caps);
    return 0;
}

/* setup_caps:
 * Setup capabilities for the process:
 * Bounding and ambiant capabilities: only CAP_NET_RAW allowed
 * Permitted and effective capabilities: only CAP_NET_RAW allowed
 */
int setup_capabilities(void)
{
    // First set the bounding/ambient capabilities
    // Then set the permitted/effective capabilities
    // The order is important because if we drop all effective capabilities
    // first, we can no longer set the bounding/ambient capabilities since we no
    // longer have the rights!
    if (setup_bounding_ambiant_caps() < 0)
    {
        fprintf(stderr, "setup_bounding_ambiant_caps failed\n");
        return -1;
    }
    if (setup_permitted_effective_caps() < 0)
    {
        fprintf(stderr, "setup_permitted_effective_caps failed\n");
        return -1;
    }
    return 0;
}