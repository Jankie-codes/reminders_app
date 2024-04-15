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

void error(char* errorStr) {
	printf("Error: %s\n", errorStr);
	exit(1);
}

int wdayStrToInt(char* wdayStr) {
	#define daysSize 14
	char days[daysSize][10] = {"sun", "sunday", "mon", "monday", "tues", "tuesday", "wed", "wednesday", "thurs", "thursday", "fri", "friday", "sat", "saturday"};
	for (int i = 0; i < daysSize; i++) {
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

//0: month 1: day 2: year 3: hours 4: minutes
int parseDateArg(char* dateArg, int dateField) {
	struct tm* pDate;
	time_t now = time(0);
	pDate = localtime(&now);

	if ((strcmp("td", dateArg) == 0) || (strcmp("today", dateArg) == 0)) {
		//no struct tm modifications necessary
	}
	else if ((strcmp("tmr", dateArg) == 0) || (strcmp("tomorrow", dateArg) == 0)) {
	  pDate->tm_mday++;
	} else if (wdayStrToInt(dateArg) != -1) {
		pDate->tm_mday += daysToWday(dateArg, pDate->tm_wday);
	} else {
		error("Invalid date argument");
	}
	printf("%d-%d-%d\n", pDate->tm_mon, pDate->tm_mday, (pDate->tm_year - 100));
	
	switch (dateField) {
		case 0: 
			return pDate->tm_mon;
			break;
		case 1:
			return pDate->tm_mday;
			break;
		case 2:
			return pDate->tm_year;
			break;
		case 3: 
			return pDate->tm_hour;
			break;
		case 4:
			return pDate->tm_min;
			break;
	}
}

bool errorIfFlagSet(char* flag, bool flagSet) {
	if (flagSet) {
		char* errorMessage = strcat(flag, " has been specified twice"); //WARNING: strcat MODIFIES FLAG!!!!! IDK WHY
		error(errorMessage);
	}
	return true;
}

void parseArgsAddReminder(int argc, char** argv) {
	char* message = argv[2];
	int dateFields[5]; //month day years hours minutes
	char* description;
	char* recentFlag = "";
	char flags[3][3] = {"-d", "-t", "-e"};
	bool flagsSet[3] = {false, false, false};

	printf("Message: %s\n", message);
	
	for (int i = 3; i < argc; i++) {
		if (strcmp("", recentFlag) == 0) {
			recentFlag = argv[i];
		} else if (strcmp(flags[0], recentFlag) == 0) {
			printf("-d\n");
			recentFlag = "";
			errorIfFlagSet(flags[0], flagsSet[0]);
			for (int f = 0; f < 5; f++) {
				dateFields[f] = parseDateArg(argv[i], f);
			}
			flagsSet[0] = true;
		} else if (strcmp(flags[1], recentFlag) == 0) {
			printf("-t\n");
			recentFlag = "";
		} else if (strcmp(flags[2], recentFlag) == 0) {
			printf("-e\n");
			recentFlag = "";
		} else {
			printf("flagnotfound: %s\n", recentFlag);
			error("error in arguments");
		}
	}
	if (recentFlag[0] == '-') {
		error("error: flag was specified, but no argument for it entered");
	} else if (!(strcmp("", recentFlag) == 0)) {
		error("error in arguments");
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
						error("Error opening file.");
					}
					rewriteFile(&remindersList, fptr);
					fclose(fptr);
		} else if (strcmp("l", argv[1]) == 0 || strcmp("ls", argv[1]) == 0) {
					printf("task ls or task l\n");
					fptr = fopen("./reminders_save_file.txt","r");
					if (fptr == NULL) {
						error("Error opening file.");
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

