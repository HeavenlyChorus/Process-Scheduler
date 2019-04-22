#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/times.h>

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
		printf("Process %d\n", child);
		printf("Time elapsed: %f\n", (double)etime/sysconf(_SC_CLK_TCK));
		printf("User time: %f\n", (double)utime/sysconf(_SC_CLK_TCK));
		printf("System time: %f\n", (double)stime/sysconf(_SC_CLK_TCK));
	}

	else//child
	{
		execvp(argv[1], argv+1);
	}

	return 0;
}
