#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct Reminder {
	time_t* datetime;
	char* message;
	char* description;
	int* id;
} Reminder;

void addReminder(char* message, time_t* datetime, char* desc, Reminder** reminders, int* capacity, int* numItems) {
	if (*numItems == *capacity) {
			*capacity *= 2;
			reminders = realloc(reminders, (sizeof(Reminder) * *capacity));
	}
	*numItems += 1;
	Reminder* reminderToAdd = malloc(sizeof(Reminder));
	reminderToAdd->message = malloc(sizeof(char)*2048);
	reminderToAdd->description = malloc(sizeof(char)*2048);
	reminderToAdd->message = message;
	reminderToAdd->description = desc;
	reminderToAdd->datetime = datetime;
	reminderToAdd->id = malloc(sizeof(int));
	*(reminderToAdd->id) = 1;
	printf("%d\n",reminderToAdd->id);
	
	reminders[*numItems - 1] = reminderToAdd;
}

time_t* newDateTime(int month, int day, int year, int hours, int minutes) {
	struct tm value;
  
  value.tm_sec = 0;
  value.tm_min = minutes;
  value.tm_hour = hours;
  value.tm_mday = day;
  value.tm_mon = month;
  value.tm_year = year;
	value.tm_isdst = -1;

	time_t* time_epoch = malloc(sizeof(time_t));
	*time_epoch = mktime(&value);
   
  printf("Time and date: %s", ctime(time_epoch));
	printf("%p\n", time_epoch);
	return time_epoch;
}


int main(int argc, char** argv) {
		int* capacity = malloc(sizeof(int));
		int* numItems = malloc(sizeof(int));
		*capacity = 1;
		*numItems = 0;
		Reminder** reminders = malloc(sizeof(Reminder));

		if (strcmp("add", argv[1]) == 0) {
					addReminder("samplmessage", newDateTime(5, 25, 123, 12, 0), "desc", reminders, capacity, numItems);
					printf("task add\n");
					//printf("%s\n", reminders[0]->message);
					//printf("%p\n", reminders[0]->datetime);
					printf("%d\n", reminders[0]->id);
					printf("%d\n", *(reminders[0]->id));
		} else if (strcmp("l", argv[1]) == 0 || strcmp("ls", argv[1]) == 0) {
					printf("task ls or task l\n");
		} else if (strcmp("edit", argv[1]) == 0) {
					printf("task edit\n");
		} else if (strcmp("remove", argv[1]) == 0) {
					printf("task remove\n");
		} else if (strcmp("undo", argv[1]) == 0) {
					printf("task undo\n");
		} else if (strcmp("redo", argv[1])== 0) {
					printf("task redo\n");
		}
}

