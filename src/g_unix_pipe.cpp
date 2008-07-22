/*
 * g_unix_pipe.cpp
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#define MAX_PIPES 4

#include <signal.h>
#include <sys/wait.h>

#include "sysdep.h"
#include "c_config.h"
#include "console.h"
#include "gui.h"


typedef struct {
    int used;
    int id;
    int fd;
    int pid;
    int stopped;
    EModel *notify;
} GPipe;

static GPipe Pipes[MAX_PIPES] = {
    {0},
    {0},
    {0},
    {0}
};

/* If command pipes are open, wait for input on them or
 * external file descriptors if  passed */
int WaitPipeEvent(TEvent *Event, int WaitTime, int *fds, int nfds) {
    fd_set readfds;
    struct timeval timeout;
    int have_pipes;
    int i;


    FD_ZERO(&readfds);

    have_pipes = 0;
    for (int p = 0; p < MAX_PIPES; p++)
        if (Pipes[p].used && Pipes[p].fd != -1) {
            FD_SET(Pipes[p].fd, &readfds);
            have_pipes = 1;
        }
    if (!have_pipes) return 0;

    for (i = 0; i < nfds; i++) {
        FD_SET(fds[i], &readfds);
    }

    if (WaitTime == -1) {
        if (select(sizeof(fd_set) * 8, &readfds, NULL, NULL, NULL) < 0)
            return -1;
    } else {
        timeout.tv_sec = WaitTime / 1000;
        timeout.tv_usec = (WaitTime % 1000) * 1000;
        if (select(sizeof(fd_set) * 8, &readfds, NULL, NULL, &timeout) < 0)
            return -1;
    }

    for (i = 0; i < nfds; i++) {
        if (FD_ISSET(fds[i], &readfds)) {
            return 0;
        }
    }

    for (int pp = 0; pp < MAX_PIPES; pp++) {
        if (Pipes[pp].used && Pipes[pp].fd != -1 &&
                FD_ISSET(Pipes[pp].fd, &readfds) &&
                Pipes[pp].notify) {
            Event->What = evNotify;
            Event->Msg.View = 0;
            Event->Msg.Model = Pipes[pp].notify;
            Event->Msg.Command = cmPipeRead;
            Event->Msg.Param1 = pp;
            Pipes[pp].stopped = 0;
            return 1;
        }
        //fprintf(stderr, "Pipe %d\n", Pipes[pp].fd);
    }
    return 0;
}



int GUI::OpenPipe(char *Command, EModel * notify) {
    int i;

    for (i = 0; i < MAX_PIPES; i++) {
        if (Pipes[i].used == 0) {
            int pfd[2];

            Pipes[i].id = i;
            Pipes[i].notify = notify;
            Pipes[i].stopped = 1;

            if (pipe((int *) pfd) == -1)
                return -1;

            switch (Pipes[i].pid = fork()) {
            case -1: /* fail */
                return -1;
            case 0:  /* child */
                signal(SIGPIPE, SIG_DFL);
                close(pfd[0]);
                close(0);
                dup2(pfd[1], 1);
                dup2(pfd[1], 2);
                close(pfd[1]);
                exit(system(Command));
            default:
                close(pfd[1]);
                fcntl(pfd[0], F_SETFL, O_NONBLOCK);
                Pipes[i].fd = pfd[0];
            }
            Pipes[i].used = 1;
            return i;
        }
    }
    return -1;
}

int GUI::SetPipeView(int id, EModel * notify) {
    if (id < 0 || id > MAX_PIPES)
        return -1;
    if (Pipes[id].used == 0)
        return -1;

    Pipes[id].notify = notify;
    return 0;
}

int GUI::ReadPipe(int id, void *buffer, int len) {
    int rc;

    if (id < 0 || id > MAX_PIPES)
        return -1;
    if (Pipes[id].used == 0)
        return -1;

    rc = read(Pipes[id].fd, buffer, len);
    if (rc == 0) {
        close(Pipes[id].fd);
        Pipes[id].fd = -1;
        return -1;
    }
    if (rc == -1) {
        Pipes[id].stopped = 1;
        return 0;
    }
    return rc;
}

int GUI::ClosePipe(int id) {
    int status = -1;

    if (id < 0 || id > MAX_PIPES)
        return -1;
    if (Pipes[id].used == 0)
        return -1;
    if (Pipes[id].fd != -1)
        close(Pipes[id].fd);

    kill(Pipes[id].pid, SIGHUP);
    alarm(1);
    waitpid(Pipes[id].pid, &status, 0);
    alarm(0);
    Pipes[id].used = 0;
    return WEXITSTATUS(status);
}
