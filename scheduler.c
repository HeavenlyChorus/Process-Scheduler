#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/times.h>

int jnum = 0;

struct job {
	unsigned dur;
	int arr;
	int ard;
	int exec;
	int term;
	char comd[256];
	char arg[256];
	char *argv[128];
	pid_t id;
};

int t = 0;

char* foo(char* a){
	char* p = a;
	while (*p)
		p++;
	if (*(p - 1) == '\n')
		* (p - 1) = '\0';
	return a;
}

void terminated(int s) {
	t = 1;
}
void Relay_SIGTSTP(int signo) {
}

int main(int argc, char* argv[]) {
	int i = 1;
	int j = 0;
	int k = 1;
	int l = 1;
	int e = 0;
	int min = 0;
	int time = 0;
	int time2 = 0;
	int working = 0;
	int termed = 0;
	char* filename;
	char* policy;
	char gantt[1025][10];
	pid_t id;
	signal(SIGTSTP, Relay_SIGTSTP);
	
	signal(SIGCHLD, terminated);
	
	if (argc == 3) {
		filename = argv[1];
		policy = argv[2];
	}
	else {
		printf("Unable to get required argument\n");
		printf("Required format:./scheduler [filename] [policy]\n");
		return 0;
	}

	char* str;
	char* arr;
	char* cmd;
	char* dur;
	int last = 0;
	struct job jobs[10];
	FILE* file = fopen(filename, "r");
	char line[255] = { 0 };
	
	while (fgets(line, sizeof(line), file)) {
		str = strtok(line, "\t");
		while (str) {
			switch (j) {
			case 0:
				arr = foo(str);
				break;
			case 1:
				cmd = str;
				break;
			case 2:
				dur = foo(str);
				break;
			}
			str = strtok(NULL, "\t");
			j++;
			if (j == 3)
				j = 0;
		}
		if (*arr != '\0' && jnum < 9)
		{
			jnum++;
			jobs[jnum].arr = atoi(arr);
			strcpy(jobs[jnum].comd, cmd);
			strcpy(jobs[jnum].arg, cmd);
			jobs[jnum].argv[0] = jobs[jnum].comd;
			str = strtok(jobs[jnum].arg, " ");
			while (str) {
				jobs[jnum].argv[l] = str;
				str = strtok(NULL, " ");
				l++;
			}
			jobs[jnum].argv[l] = NULL;
			l = 1;
			jobs[jnum].dur = atoi(dur);
			jobs[jnum].ard = 0;
			jobs[jnum].term = 0;
			printf("jobs[%d]: Arrive time: %d Cmd: %s Duration: %d\n", jnum, jobs[jnum].arr, jobs[jnum].comd, jobs[jnum].dur);
		}
	}

	printf("We've got %d jobs for %s\n", jnum, policy);

	if (strcmp(policy, "FIFO") == 0) {
		printf("FIFO start\n");
		while (1) {
			//printf("time %d :", time);
			for (k = 1; k <= jnum; k++) {//check if there is job arrived
				if (time == jobs[k].arr && jobs[k].ard == 0) {
					//printf("job %d arrived ", k);
					jobs[k].ard = 1;
				}
			}
			if (working == 0) {//there's no progress running
				min = 0;
				for (k = 1; k <= jnum; k++) {
					if (min == 0 || min > jobs[k].arr) {
						if (jobs[k].ard == 1 && jobs[k].term == 0) {
							working = k;
							min = jobs[k].arr;
						}
					}
				}
				if (working != 0) {
					id = fork();
					if (id > 0) {
						//printf("job %d started ", working);
						jobs[working].id = id;
						jobs[working].exec = time;
					}
					else {
						//printf("executing job %d : %s \n",working, jobs[working].comd);
						execvp("./monitor", jobs[working].argv);
					}
				}
				else {
					if (termed == jnum) {
						//printf("all jobs are terminated \n");
						for (k = 1; k <= jnum; k++) {
							gantt[time][k] = ' ';
							if (jobs[k].term == 0 && jobs[k].ard == 1) {gantt[time][k] = '.';}
							gantt[time][working] = '#';
							//printf("time%d:%d working\n",time,working);
						}
						printf("Gantt Chart\n");
						printf("Time: ");
						for (k = 0; k < time; k++) {if (k % 10 == 0) {printf(" %d", k / 10);} else {printf("  ");};}; printf("\n");
						printf("      "); for (k = 0; k < time; k++) {printf(" %d", k % 10);}; printf("\n");
						for (k = 1; k <= jnum; k++) {
							printf("job%d  ", k);
							for (i = 0; i <= time; i++) {printf(" %c", gantt[i][k]);};
							printf("\n");
						};
						return 0;
					}
				}
			}
			else {//there is a progress working
				if (jobs[working].dur != -1) {
					jobs[working].dur -= 1;
					//printf("job %d is working, %d time remaining pid:%d ",working,jobs[working].dur,jobs[working].id);
				}
				else {
					//printf("job %d is working pid:%d ",working,jobs[working].id);
				}
				if (jobs[working].dur == 0 || (t == 1 && jobs[working].dur == -1)) {
					kill(jobs[working].id, SIGTERM);
					//printf("job %d finished ", working);
					jobs[working].term = 1;
					working = 0;
					t = 0;
					termed = termed + 1;
					continue;
				}
			}
			for (k = 1; k <= jnum; k++) {
				gantt[time][k] = ' ';
				if (jobs[k].term == 0 && jobs[k].ard == 1) {gantt[time][k] = '.';}
				gantt[time][working] = '#';
			}
			time++;
			//printf("\n");
			sleep(1);
		}
	}

	if (strcmp(policy, "SJF") == 0) {
		printf("SJF start\n");
		while (1) {
			//printf("time %d :", time);
			for (k = 1; k <= jnum; k++) {//check if there is job arrived
				if (time == jobs[k].arr && jobs[k].ard == 0) {
					//printf("job %d arrived ", k);
					jobs[k].ard = 1;
				}
			}
			if (working == 0) {//there's no progress running
				min = 0;
				for (k = 1; k <= jnum; k++) {
					if (min == 0 || min > jobs[k].dur) {
						if (jobs[k].ard == 1 && jobs[k].term == 0) {
							working = k;
							min = jobs[k].dur;
						}
					}
				}
				if (working != 0) {
					id = fork();
					if (id > 0) {
						//printf("job %d started ", working);
						jobs[working].id = id;
						jobs[working].exec = time;
					}
					else {
						//printf("executing job %d : %s \n",working, jobs[working].comd);
						execvp("./monitor", jobs[working].argv);
					}
				}
				else {
					if (termed == jnum) {
						//printf("all jobs are terminated \n");
						for (k = 1; k <= jnum; k++) {
							gantt[time][k] = ' ';
							if (jobs[k].term == 0 && jobs[k].ard == 1) {gantt[time][k] = '.';}
							gantt[time][working] = '#';
							printf("time%d:%d working\n", time, working);
						}
						printf("Gantt Chart\n");
						printf("Time: ");
						for (k = 0; k < time; k++) {if (k % 10 == 0) {printf(" %d", k / 10);} else {printf("  ");};}; printf("\n");
						printf("      "); for (k = 0; k < time; k++) {printf(" %d", k % 10);}; printf("\n");
						for (k = 1; k <= jnum; k++) {
							printf("job%d  ", k);
							for (i = 0; i <= time; i++) {printf(" %c", gantt[i][k]);};
							printf("\n");
						};
						return 0;
					}
					//else
					//printf("no job is working ");
				}
			}
			else {//there is a progress working
				if (jobs[working].dur != -1) {
					jobs[working].dur -= 1;
					//printf("job %d is working, %d time remaining pid:%d ",working,jobs[working].dur,jobs[working].id);
				}
				else {
					//printf("job %d is working pid:%d ",working,jobs[working].id);
				}
				if (jobs[working].dur == 0 || (t == 1 && jobs[working].dur == -1)) {
					kill(jobs[working].id, SIGTERM);
					//printf("job %d finished ", working);
					jobs[working].term = 1;
					working = 0;
					t = 0;
					termed = termed + 1;
					continue;
				}
			}
			for (k = 1; k <= jnum; k++) {
				gantt[time][k] = ' ';
				if (jobs[k].term == 0 && jobs[k].ard == 1) {gantt[time][k] = '.';}
				gantt[time][working] = '#';
			}
			time++;
			//printf("\n");
			sleep(1);
		}
	}

	if (strcmp(policy, "RR") == 0) {
		printf("RR start\n");
		time -=1;
		while (1) {
			for (k = 1; k <= jnum; k++) {
				if (gantt[time][k] != '#')
				gantt[time][k] = ' ';
				if (jobs[k].term == 0 && jobs[k].ard == 1) {gantt[time][k] = '.';}
				gantt[time][working] = '#';
			}
			sleep(1);
			time+=1;
			printf("time %d :\n", time);
			if (e == 2 && working != 0) {
				kill(jobs[working].id, SIGTSTP);
				last = working;
				working = 0;
				e = 0;
			}
			for (k = 1; k <= jnum; k++) {//check if there is job arrived
				if (time == jobs[k].arr && jobs[k].ard == 0) {
					printf("job %d arrived \n", k);
					jobs[k].ard = 1;
					id = fork();
					if (id > 0) {
						jobs[k].id = id;
						jobs[k].exec = time;
						kill(jobs[k].id, SIGTSTP);
					}
					else {
						execvp("./monitor", jobs[k].argv);
					}
				}
			}
			if (working == 0) {//there's no progress running
				for (k = jnum; k > last; k--) {
					if (jobs[k].ard == 1 && jobs[k].term == 0) {working = k;}
				}
				if (working == 0) {
					if (last > 1) {
						for (k = last - 1; k > 0; k--) {
							if (jobs[k].ard == 1 && jobs[k].term == 0) {working = k;}
						}
					}
					if (working == 0) { if (jobs[last].term == 0) { working = last; } else {working = 0;} }
				}
				if (working != 0) {
					kill(jobs[working].id, SIGCONT);
					e = 0;
					if (jobs[working].dur != -1) {
						jobs[working].dur -= 1;
						e += 1;
						printf("job %d starts, %d time remaining executed:%d \n",working,jobs[working].dur,e);
					}
					if (jobs[working].dur == 0 || (t == 1 && jobs[working].dur == -1)) {
						gantt[time][working] = '#';
						kill(jobs[working].id, SIGTERM);
						printf("job %d finished ", working);
						jobs[working].term = 1;
						last = working;
						working = 0;
						t = 0;
						termed = termed + 1;
					}
				}
			}
			else {//there is a progress working
				if (jobs[working].dur != -1) {
					jobs[working].dur -= 1;
					e += 1;
					printf("job %d is working, %d time remaining executed:%d \n",working,jobs[working].dur,e);
				}
				if (jobs[working].dur == 0 || (t == 1 && jobs[working].dur == -1)) {
					gantt[time][working] = '#';
					kill(jobs[working].id, SIGTERM);
					printf("job %d finished \n", working);
					jobs[working].term = 1;
					last = working;
					working = 0;
					t = 0;
					termed = termed + 1;
					continue;
				}
			}
			if (termed == jnum) {
				printf("all jobs are terminated in %d \n", time);
				for (k = 1; k <= jnum; k++) {
					if (gantt[time][k] != '#')
						gantt[time][k] = ' ';
					if (jobs[k].term == 0 && jobs[k].ard == 1) {gantt[time][k] = '.';}
					gantt[time][working] = '#';
				}
				printf("Gantt Chart\n");
				printf("Time: ");
				for (k = 0; k <= time; k++) {if (k % 10 == 0) {printf(" %d", k / 10);} else {printf("  ");};}; printf("\n");
				printf("      "); for (k = 0; k <= time; k++) {printf(" %d", k % 10);}; printf("\n");
				for (k = 1; k <= jnum; k++) {
					printf("job%d  ", k);
					for (i = 0; i <= time; i++) {printf(" %c", gantt[i][k]);};
					printf("\n");
				};
				return 0;
			}

		}
	}
	return 0;
}

