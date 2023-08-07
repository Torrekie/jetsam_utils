//
//  main.c
//  overb0ard
//
//  Created by Conrad Kramer on 3/17/15.
//  Copyright (c) 2015 Kramer Software Productions, LLC. All rights reserved.
//  Revived by Adam Tunnic on 5/17/21.
//  Enhanced by Torrekie Gen on 7/8/23.
//

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <getopt.h>
#include <spawn.h>

#include <sys/sysctl.h>
#include <sys/kern_memorystatus.h>

#define FLAG_PLATFORMIZE (1 << 1)

extern char **environ;

const char *priority_str[] = {
    "Idle",
    "Idle (Deferred)",
    "Background (Opportunistic)",
    "Background",
    "Mail",
    "Phone",
    "6",
    "7",
    "UI Support",
    "Foreground Support",
    "Foreground",
    "11",
    "Audio and Accessory",
    "Conductor",
    "Home",
    "Executive",
    "Important",
    "Critical"
};

bool verbose = false;

int verbose_printf(const char * restrict fmt, ...) {
    int ret;
    if (!verbose)
        return 0;

    va_list args;
    va_start(args, fmt);
    ret = vprintf(fmt, args);
    va_end(args);
    return ret;
}

const char *strprio(int prio) {
    const char *ret;

    if (prio > (sizeof(priority_str) / sizeof(priority_str[0]))) {
        sprintf(ret, "%d", prio);
    } else {
        ret = priority_str[prio];
    }

    return ret;
}

int main(int argc, const char * argv[]) {
    // Make it easier for devs to use overb0ard in their code.
    setuid(0);

    char *endptr;

    static const char *usage = "Usage: %s [-l limit] [-s set] [-p priority] [-v] processes\nSee https://github.com/Torrekie/overb0ard for more information.\n";

    static struct option opts[] = {
        {"limit", optional_argument, NULL, 'l'},
        {"priority", optional_argument, NULL, 'p'},
        {"set", optional_argument, NULL, 's'},
        {"verbose", no_argument, NULL, 'v'},
        {NULL, 0, NULL, 0}
    };

//    char** proc_list;
    pid_t *pid_list;
    int proc_list_len = 0;

    int ch;
    const char *prioritystr = NULL, *limitstr = NULL, *setstr = NULL;
    while ((ch = getopt_long(argc, (char * const *)argv, "l:p:s:v", opts, NULL)) != -1) {
        switch (ch) {
            case 'l':
                limitstr = optarg;
                break;
            case 'p':
                prioritystr = optarg;
                break;
            case 's':
                setstr = optarg;
                break;
            case 'v':
                verbose = true;
                break;
        }
    }

    const char *overb0ard = argv[0];

    if (argc == optind || (prioritystr == NULL && limitstr == NULL && setstr == NULL)) {
        fprintf(stderr, usage, overb0ard);
        return 1;
    }

    // Don't directly operate on argc/argv, for position-free opts
    proc_list_len = argc - optind;
    pid_t pid;
    int pid_list_size = 1;
    unsigned long pid_list_len = 0;
    pid_list = malloc(pid_list_size * sizeof(pid_t));
    if (pid_list == NULL) {
        fprintf(stderr, "%s: malloc failed with error: %s\n", overb0ard, strerror(errno));
        return 1;
    }
    // Store and process non-option args
    for (int i = optind; i < argc; i++) {
//        proc_list[i - optind] = argv[i];
        char *process = (char *)argv[i];
        pid = strtoimax(process, &endptr, 0);
        // If recieved procname, try to get all PID with this name
        if (process == endptr || *endptr != '\0') {
            int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};

            size_t all_proc_size;
            // Get length of kinfo_proc list
            if (sysctl(mib, 4, NULL, &all_proc_size, NULL, 0) == -1) {
                fprintf(stderr, "%s: sysctl failed with error: %s\n", overb0ard, strerror(errno));
                goto free;
            }

            // malloc list
            struct kinfo_proc *processes = malloc(all_proc_size);
            if (processes == NULL) {
                fprintf(stderr, "%s: malloc failed with error: %s\n", overb0ard, strerror(errno));
                goto free;
            }

            // Recieve kinfo_proc list
            if (sysctl(mib, 4, processes, &all_proc_size, NULL, 0) == -1) {
                fprintf(stderr, "%s: sysctl failed with error: %s\n", overb0ard, strerror(errno));
                goto free;
            }

            // Find all pids with procname
            for (int j; j < all_proc_size / sizeof(struct kinfo_proc); j++) {
                if (strcmp(processes[j].kp_proc.p_comm, process) == 0) {
                    pid = processes[j].kp_proc.p_pid;
                    if (pid == 0) {
                        fprintf(stderr, "%s: warning: cannot get pid of process `%s' (%s)\n", overb0ard, process, strerror(ESRCH));
                        continue;
                    }
                    if (pid_list_len > pid_list_size) {
                        pid_list_size *= 2;
                        pid_t *alloc = malloc(pid_list_size * sizeof(pid_t));
                        if (alloc == NULL) {
                            fprintf(stderr, "%s: malloc failed with error: %s\n", overb0ard, strerror(errno));
                            goto free;
                        }
                        pid_list = alloc;
                    }
                    pid_list[pid_list_len] = pid;
                    pid_list_len++;
                }
            }
        } else {
            // Otherwise user is providing PID
            if (pid_list_len > pid_list_size) {
                pid_list_size *= 2;
                pid_t *alloc = malloc(pid_list_size * sizeof(pid_t));
                if (alloc == NULL) {
                    fprintf(stderr, "%s: malloc failed with error: %s\n", overb0ard, strerror(errno));
                    goto free;
                }
                pid_list = alloc;
            }
            pid_list[pid_list_len] = pid;
            pid_list_len++;
        }
    }

    // Remember clear errno
    errno = 0;
    // Process all recieved PID
    for (unsigned long k = 0; k < pid_list_len; k++) {
        pid = pid_list[k];
        if (limitstr) {
            uint32_t limit = strtoimax(limitstr, &endptr, 0);
            if (limitstr == endptr || *endptr != '\0') {
                fprintf(stderr, "%s: `%s' is not a valid number\n", overb0ard, limitstr);
                goto free;
            }

            if (memorystatus_control(MEMORYSTATUS_CMD_SET_JETSAM_TASK_LIMIT, pid, limit, NULL, 0) != 0) {
                fprintf(stderr, "%s: Failed to update %d with error %d: %s\n", overb0ard, pid, errno, strerror(errno));
            } else {
                verbose_printf("%s: Updated limit of process %d to %s MB\n", overb0ard, pid, limitstr);
            }
        }

        if (prioritystr) {
            uint32_t priority = strtoimax(prioritystr, &endptr, 0);
            if (prioritystr == endptr || *endptr != '\0') {
                fprintf(stderr, "%s: `%s' is not a valid number\n", overb0ard, prioritystr);
                goto free;
            }

            memorystatus_priority_entry_t properties;
            memset(&properties, 0, sizeof(memorystatus_priority_entry_t));
            properties.pid = pid;
            properties.priority = priority;

            if (memorystatus_control(MEMORYSTATUS_CMD_GRP_SET_PROPERTIES, 0, 0, &properties, sizeof(memorystatus_priority_entry_t)) != 0) {
                fprintf(stderr, "%s: Failed to update %d with error %d: %s\n", overb0ard, pid, errno, strerror(errno));
            } else {
                verbose_printf("%s: Updated priority of process %d to %s (%d)\n", overb0ard, pid, strprio(priority), priority);
            }
        }

        if (setstr) {
            // This is where the persistent stuff will go.
            fprintf(stderr, "%s: \"set\" option is not yet implemented, ignored.\n", overb0ard);
        }
    }

free:
    free(pid_list);

    return 0;
}
