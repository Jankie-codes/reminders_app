# C Reminders App, Terminal Client
### Latest Version: V1.00

README Last updated: June 13, 2024
## Overview

This is a simple reminders app terminal client written in C and designed for Linux.

## App Dependencies
This app requires the **Cronie daemon** for reminder notifications to properly work. Without it, reminders can still be made, but notifications will not occur.

Information and download help can be found here: https://github.com/cronie-crond/cronie/

Relevant Archwiki forum (which helped me personally): https://wiki.archlinux.org/title/cron

The app also requires the **gcc C compiler** which the makefile uses.

Finally, the app requires that `notify-send` is installed into your path.

## Installation (Read Dependecies Section First!)

FIRST: make sure that cron daemon and gcc is installed. For more information, please see the **App Dependencies** section above.

Once all dependencies are installed, begin by cloning the repository.

```
$ git clone https://github.com/Jankie-codes/reminders_app.git
```

Enter the local repository folder.

```
$ cd reminders_app
```

Compile all the C code using the given makefile.

```
$ make
```
Next, install both the task app and notify app (the latter will be run regularly to check for notifications) into /usr/local/bin by running make install.

```
$ sudo make install
```

IMPORTANT: Note that running `sudo make install` also edits configurations in the crontab (cronie daemon's config file), setting the notify app to be run every minute. 

**Installation is now complete. Note that to remove all executable and .o files from the local repository folder, you can run** `make clean`**within it.**
## Usage/Examples

**Note that in all commands, any flag starting with a dash (-) is optional. Everything else without a dash is mandatory.**

### Add a reminder:

```
$ task add "description" -d 06-13-24 -t 3:04pm -e "extra note here"
```

Note that for the -d flag, keywords such as (but not limited by) "mon", "wed", "wednesday", "today"
 or "td", and "tomorrow" or "tmr" all work. Otherwise, use the format mm-dd-yy.

The -t flag accepts times in HH:MM(am/pm) OR HH(am/pm) format.

Other examples:

```
$ task add description2 -d today -t 9:30am
$ task add description3
$ task add "multiple word description" -t 12pm -e "multi-word note"
```

### List reminders:

The outputted list is always sorted by chronological order. Reminders with no date are placed at the bottom and organized alphabetically.

```
$ task l
                                                                My Reminders
--------------------------------------------------------------------------------------------------------------------------------------------
        id      date            time            description                                                     note                
--------------------------------------------------------------------------------------------------------------------------------------------
        1       06/13/24        03:00PM         take out the trash                                              extra note here.    
        2       N/A             N/A             message     
```
OR
```
$ task ls
                                                                My Reminders
--------------------------------------------------------------------------------------------------------------------------------------------
        id      date            time            description                                                     note                
--------------------------------------------------------------------------------------------------------------------------------------------
        1       06/13/24        03:00PM         take out the trash                                              extra note here.    
        2       N/A             N/A             message     
```

### Complete/remove a reminder:

This removes the reminder from the todo list. **The first argument is the reminder ID**, which can be found using `task l`.

Note that reminder IDs change as new reminders are added and deleted.
```
$ task complete 1
```
OR
```
$ task remove 1
```

### Edit a reminder:
**The first argument is the reminder ID**, which can be found using `task l`.

At least one flag (-m, -d, -t, or -e) is mandatory.
```
$ task edit 1 -m "new description" -d tmr -e "new extra note" -t 7pm
```

Note that for the -d flag, keywords such as (but not limited by) "mon", "wed", "wednesday", "today"
or "td", and "tomorrow" or "tmr" all work. Otherwise, use the format mm-dd-yy.

The -t flag accepts times in HH:MM(am/pm) OR HH(am/pm) format.

### View man page and more information on each command:

```
$ task
```

Typing invalid commands will also invoke a man page.

## Uninstallation

Currently, uninstallation can only be done manually.

Start by removing both `task` and `notify` apps from `/usr/local/bin` where it was installed.
```
$ rm /usr/local/bin/task
$ rm /usr/local/bin/notify
```

Then, edit the crontab using:

```
$ crontab -e
```
This opens a text file containing all crontab commands. Delete the line `* * * * * XDG_RUNTIME_DIR="/run/user/1000" DISPLAY=:0 /usr/local/bin/notify` which tells the system to check for notifications every minute.

Finally, delete the repository folder on your machine using `rm -rf`.

## Special Thanks
Special thanks go to my AWESUM brother the GOAT for supporting me throughout this app's development.
