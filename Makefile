.PHONY: install, build, clean

CC=gcc
CFLAGS=

%.o: %.c
	$(CC) -c $< $(CFLAGS)

build: notify task

notify: remfiles.o reminders.o notify.o
	$(CC) notify.o remfiles.o reminders.o -o notify 

task: remfiles.o reminders.o task.o
	$(CC) task.o remfiles.o reminders.o -o task

install: notify task
	which crontab
	cp notify /usr/local/bin
	cp task /usr/local/bin
	(crontab -l ; echo "* * * * * notify")| crontab -

clean:
	rm notify task *.o
