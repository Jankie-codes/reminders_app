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
	THISUSER=`logname`; (crontab -l -u $$THISUSER 2>/dev/null; echo "* * * * * XDG_RUNTIME_DIR=\"/run/user/1000\" DISPLAY=:0 /usr/local/bin/notify")| crontab -u $$THISUSER -

clean:
	rm notify task *.o
