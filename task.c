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
	//printf("%d\n",reminderToAdd->id);
	
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

	//printf("%d\n", value.tm_sec);
	
	time_t* time_epoch = malloc(sizeof(time_t));
	*time_epoch = mktime(&value);
   
  //printf("Time and date: %s", ctime(time_epoch));
	//printf("%p\n", time_epoch);

	return time_epoch;
}

void rewriteFile(Reminder** remindersList, FILE* fptr) {
		for (int i = 0; i < (sizeof(remindersList) / sizeof(remindersList[0])); i++) {
			//fwrite(remindersList[i]->message, sizeof(char) * strlen(remindersList[i]->message), 1, fptr);
			fprintf(fptr, "%d\n", *(remindersList[i]->id));
			fprintf(fptr, "%d\n", strlen(remindersList[i]->message));
			fprintf(fptr, "%s\n", remindersList[i]->message);
			fprintf(fptr, "%d\n", strlen(remindersList[i]->description));
			fprintf(fptr, "%s\n", remindersList[i]->description);

			struct tm* dateToSave;

			dateToSave = localtime(remindersList[i]->datetime);

			fprintf(fptr, "%d %d %d %d %d\n", dateToSave->tm_mon, dateToSave->tm_mday, dateToSave->tm_year, dateToSave->tm_hour, dateToSave->tm_min);
		}
}

void readFile(FILE* fptr) {
	int id;
	fscanf(fptr, "%d\n", &id);
	int messageLen;
	fscanf(fptr, "%d\n", &messageLen);
	messageLen++; //increments the string length to account for the null character at the end when reading
	char message[messageLen];
	fgets(message, messageLen, fptr);

	int descLen;
	fscanf(fptr, "%d\n", &descLen);
	descLen++; //increments the string length to account for the null character at the end when reading
	char desc[descLen];
	fgets(desc, descLen, fptr);

	int month;
	int day;
	int year;
	int hours;
	int minutes;

	fscanf(fptr, "%d ", &month);
	fscanf(fptr, "%d ", &day);
	fscanf(fptr, "%d ", &year);
	fscanf(fptr, "%d ", &hours);
	fscanf(fptr, "%d ", &minutes);

	printf("%d\n", id);
	printf("%d\n", messageLen);
	printf("%s\n", message);
	printf("%d\n", descLen);
	printf("%s\n", desc);

	printf("%d\n", month);
	printf("%d\n", day);
	printf("%d\n", year);
	printf("%d\n", hours);
	printf("%d\n", minutes);
}


int main(int argc, char** argv) {
		FILE* fptr;
		int* capacity = malloc(sizeof(int));
		int* numItems = malloc(sizeof(int));
		*capacity = 1;
		*numItems = 0;
		Reminder** reminders = malloc(sizeof(Reminder));
		if (strcmp("add", argv[1]) == 0) {
					addReminder("samplmessage with spaces", newDateTime(5, 25, 123, 12, 0), "desc", reminders, capacity, numItems);
					//printf("task add\n");
					//printf("%s\n", reminders[0]->message);
					//printf("%p\n", reminders[0]->datetime);
					//printf("%d\n", reminders[0]->id);
					//printf("%d\n", *(reminders[0]->id));
					//printf("%d\n", sizeof(reminders) / sizeof(reminders[0]));
					fptr = fopen("./reminders_save_file.txt","w");
					if (fptr == NULL) {
						printf("Error opening file.");
						exit(1);
					}
					rewriteFile(reminders, fptr);
					fclose(fptr);
		} else if (strcmp("l", argv[1]) == 0 || strcmp("ls", argv[1]) == 0) {
					printf("task ls or task l\n");
					fptr = fopen("./reminders_save_file.txt","r");
					if (fptr == NULL) {
						printf("Error opening file.");
						exit(1);
					}
					readFile(fptr);
					fclose(fptr);
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

