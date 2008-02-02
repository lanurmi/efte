/*    e_unix.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1997, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifdef WINNT
#include "e_win32.cpp"
#else

// UNIX specific routines

#include "fte.h"

#include <sys/wait.h>
#include <signal.h>

int EView::SysShowHelp(ExState &State, const char *word) {
    char options[128] = "";
    char command[1024];
    char file[MAXPATH];

    if (State.GetStrParam(this, options, sizeof(options) - 1) == 0)
        options[0] = 0;

    char wordAsk[64] = "";
    if (word == 0) {
        if (State.GetStrParam(this, wordAsk, sizeof(wordAsk) - 1) == 0)
            if (MView->Win->GetStr("Keyword",
                                   sizeof(wordAsk) - 1, wordAsk, HIST_DEFAULT) == 0)
                return 0;
        word = wordAsk;
    }

    snprintf(file, sizeof(file) - 1, "/tmp/efte%d-man-%s", getpid(), word);
    snprintf(command, sizeof(command) - 1, "%s %s %s >'%s' 2>&1", HelpCommand, options, word, file);

    /// !!! why is this needed ???
#define SYSCALL(call) while(((call) == -1) && (errno == EINTR))
    pid_t pid;
    int err, status;

    Msg(S_INFO, "Retrieving man page for %s, please wait", word);

    if ((pid = fork()) == 0) {
        close(1);
        SYSCALL(err = open(file, O_CREAT | O_WRONLY | O_APPEND, S_IRWXU));
        if (err != -1) {
            close(2);
            //dup(1); // ignore error output
            close(0);
            assert(open("/dev/null", O_RDONLY) == 0);
            execlp("man", "man",
#ifndef AIX // current AIX's don't like the -a.
                   "-a",
#endif
                   word, NULL);
            // execlp("/bin/sh", "sh", "-c", command, NULL);
        }
        perror("Can't Exec Command\n");
        exit(-1);
    } else if (pid < 0) {
        perror("Can't fork");
        return 0;
    }
    SYSCALL(err = waitpid(pid, &status, 0));
    if (err == -1) {
        perror("Waitpid failed\n");
        return 0;
    }

    // int rc = system(command);

    err = FileLoad(0, file, "CATBS", this);
    unlink(file);

    if (err == 0) {
        Msg(S_ERROR, "Error code %d retrieving manpage for %s", err, word);
        return 0;
    }
    return 1;
}

#endif
