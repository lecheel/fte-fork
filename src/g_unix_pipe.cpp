
#define MAX_PIPES 4

#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

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

static GPipe Pipes[MAX_PIPES] =
{
    {0},
    {0},
    {0},
    {0}
};

/* If command pipes are open, wait for input on them or
 * external file descriptors if passed */
int WaitPipeEvent(TEvent *Event, int WaitTime, int *fds, int nfds)
{
    struct pollfd pfds[MAX_PIPES + 8];
    int npfds = 0;
    int pipe_idx[MAX_PIPES + 8];
    int have_pipes = 0;

    for (int p = 0; p < MAX_PIPES; p++) {
        if (Pipes[p].used && Pipes[p].fd != -1) {
            pfds[npfds].fd = Pipes[p].fd;
            pfds[npfds].events = POLLIN;
            pfds[npfds].revents = 0;
            pipe_idx[npfds] = p;
            npfds++;
            have_pipes = 1;
        }
    }
    if (!have_pipes) return 0;

    int ext_start = npfds;
    for (int i = 0; i < nfds && npfds < (int)(sizeof(pfds)/sizeof(pfds[0])); i++) {
        pfds[npfds].fd = fds[i];
        pfds[npfds].events = POLLIN;
        pfds[npfds].revents = 0;
        pipe_idx[npfds] = -1;
        npfds++;
    }

    int rc = poll(pfds, npfds, WaitTime);
    if (rc <= 0) return (rc < 0) ? -1 : 0;

    for (int i = ext_start; i < npfds; i++) {
        if (pfds[i].revents & (POLLIN | POLLERR | POLLHUP)) {
            return 0;
        }
    }

    for (int i = 0; i < ext_start; i++) {
        if (pfds[i].revents & (POLLIN | POLLERR | POLLHUP)) {
            int pp = pipe_idx[i];
            if (pp >= 0 && Pipes[pp].notify) {
                Event->What = evNotify;
                Event->Msg.View = 0;
                Event->Msg.Model = Pipes[pp].notify;
                Event->Msg.Command = cmPipeRead;
                Event->Msg.Param1 = pp;
                Pipes[pp].stopped = 0;
                return 1;
            }
        }
    }
    return 0;
}



int GUI::OpenPipe(char *Command, EModel * notify)
{
    int i;

    for (i = 0; i < MAX_PIPES; i++) {
	if (Pipes[i].used == 0) {
	    int pfd[2];

	    Pipes[i].id = i;
	    Pipes[i].notify = notify;
	    Pipes[i].stopped = 1;

#if defined(__linux__) && defined(O_CLOEXEC)
	    if (pipe2(pfd, O_CLOEXEC | O_NONBLOCK) == -1)
#endif
	    {
		if (pipe(pfd) == -1)
		    return -1;
		fcntl(pfd[0], F_SETFD, FD_CLOEXEC);
		fcntl(pfd[1], F_SETFD, FD_CLOEXEC);
		fcntl(pfd[0], F_SETFL, O_NONBLOCK);
	    }

	    switch (Pipes[i].pid = fork()) {
	    case -1:		/* fail */
		close(pfd[0]);
		close(pfd[1]);
		return -1;
	    case 0:		/* child */
		{
		    struct sigaction sa;
		    memset(&sa, 0, sizeof(sa));
		    sa.sa_handler = SIG_DFL;
		    sigaction(SIGPIPE, &sa, NULL);
		    close(pfd[0]);
		    close(0);
		    int devnull = open("/dev/null", O_RDONLY);
		    (void)devnull;
		    dup2(pfd[1], 1);
		    dup2(pfd[1], 2);
		    close(pfd[1]);
		    exit(system(Command));
		}
	    default:
		close(pfd[1]);
		Pipes[i].fd = pfd[0];
	    }
	    Pipes[i].used = 1;
	    return i;
	}
    }
    return -1;
}

int GUI::SetPipeView(int id, EModel * notify)
{
    if (id < 0 || id > MAX_PIPES)
	return -1;
    if (Pipes[id].used == 0)
	return -1;

    Pipes[id].notify = notify;
    return 0;
}

int GUI::ReadPipe(int id, void *buffer, int len)
{
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

int GUI::ClosePipe(int id)
{
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
