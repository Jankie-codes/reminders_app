#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<stdbool.h>
#include <time.h>

typedef struct OptionalDateTime {
	time_t* value;
	bool* valid;
} OptionalDateTime;

typedef struct Reminder {
	OptionalDateTime* datetime;
	char* message;
	char* description;
	int* id;
} Reminder;

typedef struct ReminderArray {
	Reminder** array;
	size_t used;
	size_t size;
} ReminderArray;

OptionalDateTime* mallocOptionalDateTime(time_t* value, bool valid) {
	OptionalDateTime* newOptionalDateTime = malloc(sizeof(OptionalDateTime));
	bool* pValid = malloc(sizeof(bool));
	*pValid = valid;
	newOptionalDateTime->value = value;
	newOptionalDateTime->valid = pValid;
	return newOptionalDateTime;
}

void freeOptionalDateTime(OptionalDateTime* odt) {
	free(odt->value);
	free(odt->valid);
	free(odt);
}

void initReminderArray(ReminderArray* a, size_t initialSize) {
  a->array = malloc(initialSize * sizeof(Reminder));
  a->used = 0;
  a->size = initialSize;
}

void addToReminderArray(ReminderArray* a, Reminder* element) {
  // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
  // Therefore a->used can go up to a->size 
  if (a->used == a->size) {
    a->size *= 2;
    a->array = realloc(a->array, a->size * sizeof(Reminder));
  }
  a->array[a->used++] = element;
}

void freeReminderArray(ReminderArray* a) {
	for (int i = 0; i < a->used; i++) {
		free(a->array[i]->message);
		free(a->array[i]->description);
		freeOptionalDateTime(a->array[i]->datetime);
		free(a->array[i]->id);
		free(a->array[i]);
	}

  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

Reminder* makeReminder(char* message, OptionalDateTime* datetime, char* desc, ReminderArray* reminders) {
	Reminder* reminderToMake = malloc(sizeof(Reminder));
	reminderToMake->message = malloc(sizeof(char)*(1+strlen(message)));
	reminderToMake->description = malloc(sizeof(char)*(1+strlen(desc)));
	//reminderToMake->message = message;
	//reminderToMake->description = desc;
	strcpy(reminderToMake->message, message);
	strcpy(reminderToMake->description, desc);
	reminderToMake->datetime = datetime;
	reminderToMake->id = malloc(sizeof(int));
	*(reminderToMake->id) = 1;
	
	return reminderToMake;
}

void addReminder(char* message, OptionalDateTime* datetime, char* desc, ReminderArray* reminders) {
	//printf("%d\n",reminderToAdd->id);
	Reminder* reminderToAdd = makeReminder(message, datetime, desc, reminders);
	addToReminderArray(reminders, reminderToAdd);
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
	//printf("TIMEDATE POINTER: %p\n", time_epoch);

	return time_epoch;
}

void rewriteFile(ReminderArray* remindersList, FILE* fptr) {
		for (int i = 0; i < remindersList->used; i++) {
			//printf("%d\n",i);
			//fwrite(remindersList[i]->message, sizeof(char) * strlen(remindersList[i]->message), 1, fptr);
			fprintf(fptr, "%d\n", *(remindersList->array[i]->id));
			fprintf(fptr, "%d\n", strlen(remindersList->array[i]->message));
			fprintf(fptr, "%s\n", remindersList->array[i]->message);
			fprintf(fptr, "%d\n", strlen(remindersList->array[i]->description));
			fprintf(fptr, "%s\n", remindersList->array[i]->description);
			fprintf(fptr, "%d\n", *(remindersList->array[i]->datetime->valid));
			if (*(remindersList->array[i]->datetime->valid)) {
				struct tm* dateToSave;

				dateToSave = localtime(remindersList->array[i]->datetime->value);

				fprintf(fptr, "%d %d %d %d %d\n", dateToSave->tm_mon, dateToSave->tm_mday, dateToSave->tm_year, dateToSave->tm_hour, dateToSave->tm_min);

			} else {
				fprintf(fptr, "0 0 0 0 0\n");
			}
		}
}

void readFile(ReminderArray* remindersList, FILE* fptr) {
	//int acc = 0;
	while (feof(fptr) == 0) {
		//printf("ACC: %d\n", acc);
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
		
		int valid;

		fscanf(fptr, "%d\n", &valid);

		fscanf(fptr, "%d\n", &month);
		fscanf(fptr, "%d ", &day);
		fscanf(fptr, "%d ", &year);
		fscanf(fptr, "%d ", &hours);
		fscanf(fptr, "%d\n", &minutes);
	
		addReminder(message, mallocOptionalDateTime(newDateTime(month, day, year, hours, minutes), valid), desc, remindersList);
		//acc++;
	}
/*
	printf("%d\n", id);
	printf("%d\n", messageLen);
	printf("%s\n", message);
	printf("%d\n", descLen);
	printf("%s\n", desc);

	printf("%d\n", month);
	printf("%d\n", day);
	printf("%d\n", year);
	printf("%d\n", hours);
	printf("%d\n", minutes);*/
}

int wdayStrToInt(char* wdayStr) {
	char days[14][10] = {"sun", "sunday", "mon", "monday", "tues", "tuesday", "wed", "wednesday", "thurs", "thursday", "fri", "friday", "sat", "saturday"};
	for (int i = 0; i < 14; i++) {
		if (strcmp(days[i], wdayStr) == 0) {
			return i/2;
		}
	}
	return -1;
}

//wdayStr is the destination weekday. we go from today to the next week's weekday
//requires: 0 <= int <= 7
int daysToWday(char* wdayStr, int today) {
	int wday = wdayStrToInt(wdayStr);
	if ((((wday - today) + 7) % 7) == 0) {
		return 7;
	} else {
		return ((wday - today) + 7) % 7;
	}
}

struct tm* parseDateArg(char* dateArg) {
	struct tm date;
	struct tm* pDate;
	time_t now = time(0);
	pDate = localtime(&now);

	if ((strcmp("td", dateArg) == 0) || (strcmp("today", dateArg) == 0)) {
		//today
	}
	else if ((strcmp("tmr", dateArg) == 0) || (strcmp("tomorrow", dateArg) == 0)) {
	  pDate->tm_mday++;
	} else if (wdayStrToInt(dateArg) != -1) {
		pDate->tm_mday += daysToWday(dateArg, pDate->tm_wday);
	}
	printf("%d-%d-%d\n", pDate->tm_mon, pDate->tm_mday, (pDate->tm_year - 100));
	return pDate;
}

void parseArgsAddReminder(int argc, char** argv) {
	char* message = argv[2];
	int month, day, year, hours, minutes;
	char* description;
	char* recentFlag = "";

	printf("Message: %s\n", message);
	
	for (int i = 3; i < argc; i++) {
		if (strcmp("", recentFlag) == 0) {
			recentFlag = argv[i];
		} else if (strcmp("-d", recentFlag) == 0) {
			printf("-d\n");
			recentFlag = "";
			parseDateArg(argv[i]);
		} else if (strcmp("-t", recentFlag) == 0) {
			printf("-t\n");
			recentFlag = "";
		} else if (strcmp("-e", recentFlag) == 0) {
			printf("-e\n");
			recentFlag = "";
		} else {
			printf("%s\n", recentFlag);
			printf("error in arguments\n");
			exit(1);
		}
	}
	if (recentFlag[0] == '-') {
		printf("error: flag was specified, but no argument for it entered\n");
		exit(1);
	} else if (!(strcmp("", recentFlag) == 0)) {
		printf("error in arguments\n");
		exit(1);
	}
	printf("arguments are proper\n");
	//printf("recentFlag: %s\n", recentFlag);
}

int main(int argc, char** argv) {
		FILE* fptr;
		ReminderArray remindersList;
		initReminderArray(&remindersList, 1);

		if (strcmp("add", argv[1]) == 0) {
					parseArgsAddReminder(argc, argv);
					//printf("task add\n");
					addReminder("samplmessage with spaces", mallocOptionalDateTime(newDateTime(5, 25, 123, 12, 0), false), "sample description.", &remindersList);
					addReminder("second reminder", mallocOptionalDateTime(newDateTime(3,27,124,12,0), true), "second desc", &remindersList);
					addReminder("third reminder", mallocOptionalDateTime(newDateTime(3,27,124,12,0), false), "third sec", &remindersList);
					fptr = fopen("./reminders_save_file.txt","w");
					if (fptr == NULL) {
						printf("Error opening file.");
						exit(1);
					}
					rewriteFile(&remindersList, fptr);
					fclose(fptr);
		} else if (strcmp("l", argv[1]) == 0 || strcmp("ls", argv[1]) == 0) {
					printf("task ls or task l\n");
					fptr = fopen("./reminders_save_file.txt","r");
					if (fptr == NULL) {
						printf("Error opening file.");
						exit(1);
					}
					readFile(&remindersList, fptr);
					for (int i = 0; i < remindersList.used; i++) {
							printf("%s\n", remindersList.array[i]->message);
							printf("%s\n", remindersList.array[i]->description);
							printf("datetime valid or not: %d\n", *(remindersList.array[i]->datetime->valid));
							printf("%s", ctime(remindersList.array[i]->datetime->value));
							//printf("%d\n", remindersList.array[i]->id);
							printf("%d\n", *(remindersList.array[i]->id));
							printf("END OF REMINDER\n");
					}

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
		freeReminderArray(&remindersList);
}

