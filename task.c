#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<stdbool.h>
#include <time.h>
#include<regex.h>

typedef enum ErrStat {
	EOK = 0,
	EBADARGS,
	EDUPFLAG,
	EBADDATE,
	EBADFLAG,
	EBADTIME,
	EEMPFLAG,
	EFILE,
	EOKFINAL,
} ErrStat;

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

bool strContains(char* str, char** strList) {
	int len = sizeof(strList)/sizeof(strList[0]);
	for(int i = 0; i < len; i++) {
 	   if(strcmp(strList[i], str) == 0) {
  	      return true;
    	}
	}
	return false;
}

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
ErrStat setDateField(char* dateArg, int dateField, int* fieldsList) {
	struct tm* pDate;
	time_t now = time(0);
	pDate = localtime(&now);
	regex_t regex;
	regcomp(&regex, "^[0-9]{1,2}-[0-9]{1,2}-[0-9]{1,2}$", REG_EXTENDED);

	if ((strcmp("td", dateArg) == 0) || (strcmp("today", dateArg) == 0)) {
		//no struct tm modifications necessary
	}
	else if ((strcmp("tmr", dateArg) == 0) || (strcmp("tomorrow", dateArg) == 0)) {
	  pDate->tm_mday++;
	} else if (wdayStrToInt(dateArg) != -1) {
		pDate->tm_mday += daysToWday(dateArg, pDate->tm_wday);
	} else if ((strptime(dateArg, "%m-%d-%y", pDate) != 0) && (regexec(&regex, dateArg, 0, NULL, 0) == 0)) {
		//do nothing as no EBADDATE error occurred
	} else {
		regfree(&regex);
		return EBADDATE;
	}
	//printf("%d-%d-%d\n", pDate->tm_mon, pDate->tm_mday, (pDate->tm_year - 100));
	
	switch (dateField) {
		case 0: 
			(*fieldsList) = pDate->tm_mon;
			break;
		case 1:
			*(fieldsList+1) = pDate->tm_mday;
			break;
		case 2:
			*(fieldsList+2) = pDate->tm_year;
			break;
		case 3: 
			*(fieldsList+3) = pDate->tm_hour;
			break;
		case 4:
			*(fieldsList+4) = pDate->tm_min;
			break;
	}
	regfree(&regex);
	return EOK;
}

ErrStat setTimeField(char* timeArg, int* fieldsList) {
	struct tm time;
	struct tm* pTime = &time;
	regex_t regex;
	regcomp(&regex, "^([0-9]{1,2}(am|pm))$|^([0-9]{1,2}:[0-9]{2}(am|pm))$", REG_EXTENDED);

	if ((strptime(timeArg, "%I%p", pTime) != 0) && (regexec(&regex, timeArg, 0, NULL, 0) == 0)) {
		//do nothing, no error
		pTime->tm_min = 0;
	} else if ((strptime(timeArg, "%I:%M%p", pTime) != 0) && (regexec(&regex, timeArg, 0, NULL, 0) == 0)) {
		//do nothing, no error
	} else {
		regfree(&regex);
		return EBADTIME;
	}

	*(fieldsList+3) = pTime->tm_hour;
	*(fieldsList+4) = pTime->tm_min;

	regfree(&regex);
	return EOK;
}

ErrStat parseArgsAddReminder(int argc, char** argv, ReminderArray* ra, void** status) {
	char* message = argv[2];
	#define dateFieldsSize 5
	int dateFields[dateFieldsSize]; //month day years hours minutes
	int* pDateFields = dateFields;
	char* description = "";
	char* recentFlag = "";
	#define numFlags 3
	char flags[numFlags][3] = {"-d", "-t", "-e"};
	bool flagsSet[3] = {false, false, false};

	for (int i = 0; i < (dateFieldsSize - 2); i++) { //datetime by default
		setDateField("today", i, pDateFields);
	}
	setTimeField("12am", pDateFields);
	
	for (int i = 3; i < argc; i++) {
		if (strcmp("", recentFlag) == 0) {
			recentFlag = argv[i];
		} else if (strcmp(flags[0], recentFlag) == 0) {
			recentFlag = "";
			if (flagsSet[0]) {
				*status = malloc(sizeof(char)*3);
				*status = strcpy(*status, flags[0]);
				return EDUPFLAG;
			}
			for (int f = 0; f < (dateFieldsSize - 2); f++) {
				ErrStat errStat = setDateField(argv[i], f, pDateFields);
				if (errStat != 0) {
					return errStat;
				}
			}
			flagsSet[0] = true;
		} else if (strcmp(flags[1], recentFlag) == 0) {
			recentFlag = "";
			if (flagsSet[1]) {
				*status = malloc(sizeof(char)*3);
				*status = strcpy(*status, flags[1]);
				return EDUPFLAG;
			}
			ErrStat errStat = setTimeField(argv[i], pDateFields);
			if (errStat != 0) {
				return errStat;
			}
			flagsSet[1] = true;
		} else if (strcmp(flags[2], recentFlag) == 0) {
			recentFlag = "";
			if (flagsSet[2]) {
				*status = malloc(sizeof(char)*3);
				*status = strcpy(*status, flags[2]);
				return EDUPFLAG;
			}
			description = argv[i];
			flagsSet[2] = true;
		} else {
			printf("flagnotfound: %s\n", recentFlag);
			return EBADARGS;
		}
	}
	if (recentFlag[0] == '-') {
		int flagLength = strlen(recentFlag);
		flagLength++; //to account for null character \0
		*status = malloc(sizeof(char) * flagLength);
		*status = strcpy(*status, recentFlag);
		
		for (int i = 0; i < numFlags; i++) {
			if (strcmp(recentFlag, flags[i]) == 0) {
				if (flagsSet[i]) {
					return EDUPFLAG;
				} else {
					return EEMPFLAG;
				}
			}
		}
		return EBADFLAG;
		
	} else if (!(strcmp("", recentFlag) == 0)) {
		return EBADARGS;
	}

	addReminder(message, mallocOptionalDateTime(newDateTime(dateFields[0], dateFields[1], dateFields[2], dateFields[3], dateFields[4]), (flagsSet[0] || flagsSet[1])), description, ra);

	return EOK;
}

void errHandle(ErrStat errStat, ReminderArray* ra, void** status) {
	switch (errStat) {
		case 0: 
			//ok
			//printf("ok\n");
			return;
		case 1:
			printf("arguments formatted badly\n");
			break;
		case 2:
			printf("%s was specified twice\n", *status);
			free(*status);
			break;
		case 3:
			printf("invalid date\n");
			break;
		case 4:
			printf("%s flag does not exist\n", *status);
			free(*status);
			break;
		case 5:
			printf("invalid time (do not forget to add am/pm indicator)\n");
			break;
		case 6:
			printf("%s flag entered, but no subsequent argument specified\n", *status);
			free(*status);
			break;
		case 7:
			printf("Error opening file\n");
			break;
		case 8:
			break;
	}
	freeReminderArray(ra);
	free(status);
	exit(1);
}

int main(int argc, char** argv) {
		FILE* fptr;
		ReminderArray remindersList;
		initReminderArray(&remindersList, 1);
		void** status = malloc(sizeof(void*));
		ErrStat errStat;

		if (strcmp("add", argv[1]) == 0) {
					errHandle(parseArgsAddReminder(argc, argv, &remindersList, status), &remindersList, status);
					//printf("errstat: %d\n", errStat);
					//printf("status: %s\n", status);
					//printf("task add\n");
					//addReminder("samplmessage with spaces", mallocOptionalDateTime(newDateTime(5, 25, 123, 12, 0), false), "sample description.", &remindersList);
					//addReminder("second reminder", mallocOptionalDateTime(newDateTime(3,27,124,12,0), true), "second desc", &remindersList);
					//addReminder("third reminder", mallocOptionalDateTime(newDateTime(3,27,124,12,0), false), "third sec", &remindersList);
					fptr = fopen("./reminders_save_file.txt","w");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, status);
						exit(1);
					}
					rewriteFile(&remindersList, fptr);
					fclose(fptr);
					errHandle(EOKFINAL, &remindersList, status);
		} else if (strcmp("l", argv[1]) == 0 || strcmp("ls", argv[1]) == 0) {
					printf("task ls or task l\n");
					fptr = fopen("./reminders_save_file.txt","r");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, status);
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
					errHandle(EOKFINAL, &remindersList, status);
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

