#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/times.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

pid_t child = 0;

void Relay_SIGTERM(int signo) {
	kill(child, SIGTERM);
}
void Relay_SIGTSTP(int signo) {
	kill(child, SIGTSTP);
}
void Relay_SIGCONT(int signo) {
	kill(child, SIGCONT);
}

int main(int argc, char *argv[]) {
	signal(SIGTERM, Relay_SIGTERM);
	signal(SIGTSTP, Relay_SIGTSTP);
	signal(SIGCONT, Relay_SIGCONT);

	struct tms start, end;
	clock_t utime, stime, etime, estart, eend;

	if (times(&start) == -1) {
		printf("times error\n");
	}

	estart = times(NULL);
	child = fork();

	if (child > 0) //parent
	{
		wait(0);
		eend = times(NULL);

		if (times(&end) == -1) {
			printf("times error\n");
		}

		utime = end.tms_cutime - start.tms_cutime;
		stime = end.tms_cstime - start.tms_cstime;
		etime = eend - estart;
		printf(ANSI_COLOR_CYAN "Process %d\n" ANSI_COLOR_RESET, child);
		printf(ANSI_COLOR_CYAN "Time elapsed: %f\n" ANSI_COLOR_RESET, (double)etime/sysconf(_SC_CLK_TCK));
		printf(ANSI_COLOR_CYAN "User time: %f\n" ANSI_COLOR_RESET, (double)utime/sysconf(_SC_CLK_TCK));
		printf(ANSI_COLOR_CYAN "System time: %f\n" ANSI_COLOR_RESET, (double)stime/sysconf(_SC_CLK_TCK));
		kill(getpid(), SIGTERM);
	}

	else//child
	{
		execvp(argv[1], argv+1);
	}

	return 0;
}
