all:monitor scheduler
monitor:monitor.c
	gcc -o monitor monitor.c
scheduler:scheduler.c
	gcc -o scheduler scheduler.c