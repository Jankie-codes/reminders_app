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
	EBADID,
	ENOID,
	ETOOMANYARGS,
	EIDNOTFOUND,
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
	int nextId;
} ReminderArray;

typedef struct BST_ {
	Reminder* value;
	struct BST_* left;
	struct BST_* right;
	int* size;
} BST;

int min(int a, int b) {
	if (b < a) {
		return b;
	} else {
		return a;
	}
}

void intsToTimeString(char* stringToModify, int hours, int minutes) {
	int realHours;
	char minutesString[3] = "00";
	char* amPm = "AM";

	if (hours >= 12) {
		amPm = "PM";
	}
	if (hours == 0) {
		realHours = 12;
	} else {
		realHours = (hours % 12);
		if (hours == 12) {
			realHours = 12;
		}
	}
	if (minutes < 10) {
		sprintf(minutesString, "0%d", minutes);
	} else {
		sprintf(minutesString, "%d", minutes);
	}
	sprintf(stringToModify, "%d:%s%s", realHours, minutesString, amPm);
}

void intsToDateString(char* stringToModify, int month, int day, int year) {
	int realYear = year + 1900;
	char monthStrings[12][10] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
	sprintf(stringToModify, "%s %d, %d", monthStrings[month], day, realYear);
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
   
	return time_epoch;
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

int rmdCmp(Reminder* a, Reminder* b) {
	if (*a->datetime->valid && *b->datetime->valid) {
			if (*a->datetime->value < *b->datetime->value) {
				return 1;
			} else if (*a->datetime->value > *b->datetime->value) {
				return -1;
			}
	}
	if ((*a->datetime->valid && *b->datetime->valid) || (!(*a->datetime->valid) && !(*b->datetime->valid))) {
			if (strcmp(b->message, a->message) != 0) {
				if (strcmp(b->message, a->message) > 0) {
					return 1;
				} else if (strcmp(b->message, a->message) < 0) {
					return -1;
				}
			}	else {
				if (strcmp(b->description, a->description) > 0) {
					return 1;
				} else if (strcmp(b->description, a->description) < 0) {
					return -1;
				} else {
					return 0;
				}
			}
	} else {
			if (*a->datetime->valid) {
				return 1;
			} else {
				return -1;
			}
	}
}

Reminder* makeReminder(char* message, OptionalDateTime* datetime, char* desc) {
	Reminder* reminderToMake = malloc(sizeof(Reminder));
	reminderToMake->message = malloc(sizeof(char)*(1+strlen(message)));
	reminderToMake->description = malloc(sizeof(char)*(1+strlen(desc)));
	strcpy(reminderToMake->message, message);
	strcpy(reminderToMake->description, desc);
	reminderToMake->datetime = datetime;
	reminderToMake->id = malloc(sizeof(int));
	*(reminderToMake->id) = 1;
	
	return reminderToMake;
}

void freeReminder(Reminder* r) {
		free(r->message);
		free(r->description);
		freeOptionalDateTime(r->datetime);
		free(r->id);
		free(r);
}

void initReminderArray(ReminderArray* a, size_t initialSize) {
  a->array = malloc(initialSize * sizeof(Reminder));
  a->used = 0;
  a->size = initialSize;
	a->nextId = 1;
}

void addToReminderArray(ReminderArray* a, Reminder* element) {
  // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
  // Therefore a->used can go up to a->size 
  if (a->used == a->size) {
    a->size *= 2;
    a->array = realloc(a->array, a->size * sizeof(Reminder));
  }
  a->array[a->used++] = element;
	(*element->id) = a->nextId;
	a->nextId++;
}

void addReminder(char* message, OptionalDateTime* datetime, char* desc, ReminderArray* reminders) {
	Reminder* reminderToAdd = makeReminder(message, datetime, desc);
	addToReminderArray(reminders, reminderToAdd);
}

void freeReminderArray(ReminderArray* a) {
	for (int i = 0; i < a->used; i++) {
		if (a->array[i] != NULL) {
			freeReminder(a->array[i]);
		}
	}

  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

void freeReminderArrayNotItems(ReminderArray* a) {
	free(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}

BST* initBST() {
	BST* bst = malloc(sizeof(BST));
	bst->size = malloc(sizeof(int));
	*(bst->size) = 0;
	bst->left = NULL;
	bst->right = NULL;
	bst->value = NULL;
	return bst;
}

void addToBST(BST* bst, Reminder* reminderToAdd) {
	(*bst->size)++;

	if (!bst->value) {
		bst->value = reminderToAdd;
		return;
	}

	if (rmdCmp(reminderToAdd, bst->value) <= 0) {
		if (!bst->left) {
			BST* left = initBST();
			left->value = reminderToAdd;
			(*left->size)++;
			bst->left = left;
			return;
		} else {
			addToBST(bst->left, reminderToAdd);
		}
	} else {
		if (!bst->right) {
			BST* right = initBST();
			right->value = reminderToAdd;
			(*right->size)++;
			bst->right = right;
			return;
		} else {
			addToBST(bst->right, reminderToAdd);
		}
	}
}

void bstToArray(BST* bst, ReminderArray* ra) {
	if ((!bst) || (!bst->value)) {
		return;
	}
	if (bst->right) {
		bstToArray(bst->right, ra);
		addToReminderArray(ra, bst->value);
		bstToArray(bst->left, ra);
	} else {
		addToReminderArray(ra, bst->value);
		bstToArray(bst->left, ra);
	}
}

void printBST(BST* bst) {
	if (!bst) {
		return;
	}
	if (bst->right) {
		printBST(bst->right);
		printf("%s\n", bst->value->description);
		printBST(bst->left);
	} else {
		printf("%s\n", bst->value->description);
		printBST(bst->left);
	}
}

void freeBST(BST* bst) {
	if (bst->value) {
		freeReminder(bst->value);
	}
	free(bst->size);
	if (bst->left != NULL) {
		freeBST(bst->left);
	}
	if (bst->right != NULL) {
		freeBST(bst->right);
	}
	free(bst);
}

bool strContains(char* str, char** strList) {
	int len = sizeof(strList)/sizeof(strList[0]);
	for(int i = 0; i < len; i++) {
 	   if(strcmp(strList[i], str) == 0) {
  	      return true;
    	}
	}
	return false;
}

void rewriteFile(ReminderArray* remindersList, FILE* fptr) {
		//fprintf(fptr, "BEGIN-SAVE-FILE:\n");
		for (int i = 0; i < remindersList->used; i++) {
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
		fprintf(fptr, "-1\n");
		fprintf(fptr, "NO-READ-PAST-THIS-POINT\n");
}

ErrStat readFile(BST* remindersBST, FILE* fptr, int mode, void** info) {
	bool idFound = false;
	//fscanf(fptr, "%s\n", NULL);
	while (feof(fptr) == 0) {
		int id;
		fscanf(fptr, "%d\n", &id);

		if (id == -1) {
			break;
		}
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
		
		
		Reminder* reminderToAdd = makeReminder(message, mallocOptionalDateTime(newDateTime(month, day, year, hours, minutes), valid), desc);

		switch (mode) {
			case 0:
				//add everything to BST
				addToBST(remindersBST, reminderToAdd);
				break;
			case 1: {
				int** ppIdToRemove= (int**) info;
				int idToRemove = **ppIdToRemove;
				char dateString[19];
				char timeString[8];
				if (valid) {
					intsToDateString(dateString, month, day, year);
					intsToTimeString(timeString, hours, minutes);
				}
				if (idToRemove == id) {
					if (!valid) {
						printf("Removed reminder \"%s\" with no date\n", message);
					} else {
						printf("Removed reminder \"%s\" set to %s %s\n", message, dateString, timeString);
					}
					//printf("Date: %d %d %d %d %d\n", month, day, year, hours, minutes);
					idFound = true;
				} else {
					addToBST(remindersBST, reminderToAdd);
				}
				break;
			}
			case 2: {
				int** ppIdToRemove= (int**) info;
				int idToRemove = **ppIdToRemove;
				char dateString[19];
				char timeString[8];
				if (valid) {
					intsToDateString(dateString, month, day, year);
					intsToTimeString(timeString, hours, minutes);
				}
				if (idToRemove == id) {
					if (!valid) {
						printf("COMPLETED reminder \"%s\" with no date\n", message);
					} else {
						printf("COMPLETED reminder \"%s\" set to %s %s\n", message, dateString, timeString);
					}
					//printf("Date: %d %d %d %d %d\n", month, day, year, hours, minutes);
					idFound = true;
				} else {
					addToBST(remindersBST, reminderToAdd);
				}
				break;
			}
		}
	
		/*
		printf("id: %d\n", id);
		printf("messagelen: %d\n", messageLen);
		printf("message: %s\n", message);
		printf("descLen: %d\n", descLen);
		printf("desc: %s\n", desc);

		printf("month: %d\n", month);
		printf("day: %d\n", day);
		printf("year: %d\n", year);
		printf("hours: %d\n", hours);
		printf("minutes: %d\n", minutes);*/
	}
	if (mode == 1) {
		if (!idFound) {
			return EIDNOTFOUND;
		}
	}

	return EOK;
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

ErrStat parseArgsAddReminder(int argc, char** argv, BST* bst, void** status) {
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

	Reminder* reminderToAdd = makeReminder(message, mallocOptionalDateTime(newDateTime(dateFields[0], dateFields[1], dateFields[2], dateFields[3], dateFields[4]), (flagsSet[0] || flagsSet[1])), description);

	addToBST(bst, reminderToAdd);

	return EOK;
}

void errHandle(ErrStat errStat, ReminderArray* ra, BST* bst, void** status) {
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
		case 9:
			printf("Invalid ID specified. Please ensure that the reminder ID argument is a number\n");
			break;
		case 10: 
			printf("No ID specified. Please specify a reminder ID for which reminder to remove\n");
			break;
		case 11:
			printf("Too many arguments\n");
			break;
		case 12:
			printf("Could not find a reminder with the given ID\n");
			break;
	}
	//bstToArray(bst, ra);
	freeBST(bst);
	freeReminderArrayNotItems(ra);
	free(status);
	exit(1);
}

bool checkIfAllMlfPrinted(char* mlf[], int numMlf, int indices[]) {
	for (int i = 0; i < numMlf; i++) {
			if (indices[i] <= strlen(mlf[i])) {
				return true;
			}
	}
	return false;
}

ErrStat printReminders(ReminderArray* ra) {
	#define maxMessage 50
	#define maxDescription 20
	#define numMultilineFields 2
	printf("--------------------------------------------------------------------------------------------------------------------------------------------\n"
						"\t%s\t%-8s\t%-7s\t\t%-*s\t\t%-*s\t\n"
						"--------------------------------------------------------------------------------------------------------------------------------------------\n", "id", "date", "time", maxMessage, "description", maxDescription, "note");

	for (int i = 0; i < ra->used; i++) {
		int id;
		bool datetimeValid;
		struct tm* datetime;
		char* message;
		char* description;
		char dateString[9];
		char timeString[9];

		char* multilineFields[numMultilineFields] = {ra->array[i]->message, ra->array[i]->description};
		char mlfLinesToPrint[numMultilineFields][50]; //message and description
		int mlfCharLimits[numMultilineFields] = {maxMessage, maxDescription};
		int mlfIndices[numMultilineFields] = {0, 0};

		id = *(ra->array[i]->id);
		datetimeValid = *ra->array[i]->datetime->valid;
		if (datetimeValid) {
			datetime = localtime(ra->array[i]->datetime->value);
			//memcpy(&datetime, localtime(ra->array[i]->datetime->value), sizeof(struct tm));
			strftime(dateString, 9, "%m/%d/%y", datetime); //watch out maxSize character limit could cause bugs
			strftime(timeString, 9, "%I:%M%p", datetime);
		} else {
			strcpy(dateString, "N/A");
			strcpy(timeString, "N/A");
		}
		
		message = ra->array[i]->message;
		description = ra->array[i]->description;

		for (int i = 0; i < numMultilineFields; i++) {
			if (strlen(multilineFields[i]) > mlfCharLimits[i]) {
				int lineLength = min((strlen(multilineFields[i]) - mlfIndices[i]), mlfCharLimits[i]);
				strncpy(mlfLinesToPrint[i], (multilineFields[i])+(mlfIndices[i]), lineLength);
				(mlfLinesToPrint[i])[lineLength] = '\0';
				//printf("%d\n", min((strlen(message) - messageIndex), maxMessage));
				//printf("message: %s\n", messageLine);
				//messageIndex += maxMessage;
				mlfIndices[i] += mlfCharLimits[i];
			} else {
				strcpy(mlfLinesToPrint[i], multilineFields[i]);
				mlfIndices[i] = mlfCharLimits[i] + 1;
			}
		}

		printf("\t%d\t%-8s\t%-7s\t\t%-*s\t\t%-*s\t\n", id, dateString, timeString, mlfCharLimits[0], mlfLinesToPrint[0], mlfCharLimits[1], mlfLinesToPrint[1]);

		bool mlfIncomplete = checkIfAllMlfPrinted(multilineFields, numMultilineFields, mlfIndices);

		while (mlfIncomplete) {
			for (int i = 0; i < numMultilineFields; i++) {
				if (mlfIndices[i] <= strlen(multilineFields[i])) {
					int lineLength = min((strlen(multilineFields[i]) - mlfIndices[i]), mlfCharLimits[i]);
					strncpy(mlfLinesToPrint[i], (multilineFields[i])+(mlfIndices[i]), lineLength);
					(mlfLinesToPrint[i])[lineLength] = '\0';
					mlfIndices[i] += mlfCharLimits[i];
				} else {
					strcpy(mlfLinesToPrint[i], "");
				}
			}
			mlfIncomplete = checkIfAllMlfPrinted(multilineFields, numMultilineFields, mlfIndices);
		
			printf("\t%-1s\t%-8s\t%-7s\t\t%-*s\t\t%-*s\t\n", "", " ", " ", mlfCharLimits[0], mlfLinesToPrint[0], mlfCharLimits[1], mlfLinesToPrint[1]);
		}
	}
	return EOK;
}

ErrStat parseArgsRemoveReminder(int argc, char** argv) {
	regex_t regex;
	regcomp(&regex, "^[0-9]+$", REG_EXTENDED);
	
	if (argc < 3) {
		return ENOID;
	} else if (argc > 3) {
		return ETOOMANYARGS;
	}
	if (regexec(&regex, argv[2], 0, NULL, 0) != 0) {
		return EBADID;
	}
	return EOK;
}

int main(int argc, char** argv) {
		FILE* fptr;
		ReminderArray remindersList;
		BST* remindersBST = initBST(remindersBST);
		initReminderArray(&remindersList, 1);

		void** status = malloc(sizeof(void*));
		ErrStat errStat;
		
		if (strcmp("add", argv[1]) == 0) {
					//errHandle(parseArgsAddReminder(argc, argv, remindersBST, status), &remindersList, remindersBST, status);
					//bstToArray(remindersBST, &remindersList);


					//printf("errstat: %d\n", errStat);
					//printf("status: %s\n", status);
					//printf("task add\n");
					//addReminder("samplmessage with spaces", mallocOptionalDateTime(newDateTime(5, 25, 123, 12, 0), false), "sample description.", &remindersList);
					//addReminder("second reminder", mallocOptionalDateTime(newDateTime(3,27,124,12,0), true), "second desc", &remindersList);
					//addReminder("third reminder", mallocOptionalDateTime(newDateTime(3,27,124,12,0), false), "third sec", &remindersList);
					fptr = fopen("./reminders_save_file.txt","r");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, status);
						exit(1);
					}

					readFile(remindersBST, fptr, 0, NULL); //comment and uncomment this one
					fclose(fptr);

					//printf("here\n");
					errHandle(parseArgsAddReminder(argc, argv, remindersBST, status), &remindersList, remindersBST, status);
					//printf("here\n");
					bstToArray(remindersBST, &remindersList);

					fptr = fopen("./reminders_save_file.txt","w");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, status);
						exit(1);
					}

					rewriteFile(&remindersList, fptr);
					fclose(fptr);
					errHandle(EOKFINAL, &remindersList, remindersBST, status);
		} else if (strcmp("l", argv[1]) == 0 || strcmp("ls", argv[1]) == 0) {
					//printf("task ls or task l\n");
					fptr = fopen("./reminders_save_file.txt","r");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, status);
						exit(1);
					}
					readFile(remindersBST, fptr, 0, NULL);
					fclose(fptr);
					/*for (int i = 0; i < remindersList.used; i++) {
							printf("%s\n", remindersList.array[i]->message);
							printf("%s\n", remindersList.array[i]->description);
							printf("datetime valid or not: %d\n", *(remindersList.array[i]->datetime->valid));
							printf("%s", ctime(remindersList.array[i]->datetime->value));
							//printf("%d\n", remindersList.array[i]->id);
							printf("%d\n", *(remindersList.array[i]->id));
							printf("END OF REMINDER\n");
					}*/
					
					
					/*Reminder* reminder1 = makeReminder("Aamessage", mallocOptionalDateTime(newDateTime(5, 25, 123, 12, 0), true), "description1");
					Reminder* reminder2 = makeReminder("Aamessage", mallocOptionalDateTime(newDateTime(5, 25, 123, 12, 0), true), "description2");
					Reminder* reminder3 = makeReminder("hi", mallocOptionalDateTime(newDateTime(5, 25, 122, 12, 0), true), "urgentmore");
					Reminder* reminder4 = makeReminder("hi", mallocOptionalDateTime(newDateTime(5, 25, 122, 12, 0), false), "nodate");
					Reminder* reminder5 = makeReminder("hi2", mallocOptionalDateTime(newDateTime(5, 25, 122, 12, 0), false), "nodate2");

					
					BST* bst = initBST();

					addToBST(bst, reminder1);
					addToBST(bst, reminder2);
					addToBST(bst, reminder3);
					addToBST(bst, reminder4);
					addToBST(bst, reminder5);
					*/
					bstToArray(remindersBST, &remindersList);

					printReminders(&remindersList);
					
					/*
					for (int i = 0; i < remindersList.used; i++) {
							printf("%s\n", remindersList.array[i]->message);
							printf("%s\n", remindersList.array[i]->description);
							printf("datetime valid or not: %d\n", *(remindersList.array[i]->datetime->valid));
							printf("%s", ctime(remindersList.array[i]->datetime->value));
							//printf("%d\n", remindersList.array[i]->id);
							printf("%d\n", *(remindersList.array[i]->id));
							printf("END OF REMINDER\n");
					}*/

					//printf("reminder1: %s\n", ctime(reminder1->datetime->value));

					//printf("rmdCmp: %d\n", rmdCmp(reminder1, reminder2));
					errHandle(EOKFINAL, &remindersList, remindersBST, status);
		} else if (strcmp("edit", argv[1]) == 0) {
					printf("task edit\n");
		} else if (strcmp("remove", argv[1]) == 0) {
					errHandle(parseArgsRemoveReminder(argc, argv), &remindersList, remindersBST, status);
					int idToRemove = atoi(argv[2]);
					int* pIdToRemove = &idToRemove;
					int** ppIdToRemove = &pIdToRemove;

					fptr = fopen("./reminders_save_file.txt","r");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, status);
						exit(1);
					}
					errHandle(readFile(remindersBST, fptr, 1, (void**)ppIdToRemove), &remindersList, remindersBST, status);
					fclose(fptr);

					bstToArray(remindersBST, &remindersList);

					fptr = fopen("./reminders_save_file.txt", "w");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, status);
						exit(1);
					}
					rewriteFile(&remindersList, fptr);
					fclose(fptr);
		} else if (strcmp("complete", argv[1]) == 0) {
					errHandle(parseArgsRemoveReminder(argc, argv), &remindersList, remindersBST, status);
					int idToRemove = atoi(argv[2]);
					int* pIdToRemove = &idToRemove;
					int** ppIdToRemove = &pIdToRemove;

					fptr = fopen("./reminders_save_file.txt","r");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, status);
						exit(1);
					}
					errHandle(readFile(remindersBST, fptr, 2, (void**)ppIdToRemove), &remindersList, remindersBST, status);
					fclose(fptr);

					bstToArray(remindersBST, &remindersList);

					fptr = fopen("./reminders_save_file.txt", "w");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, status);
						exit(1);
					}
					rewriteFile(&remindersList, fptr);
					fclose(fptr);
		} else if (strcmp("undo", argv[1]) == 0) {
					printf("task undo\n");
		} else if (strcmp("redo", argv[1])== 0) {
					printf("task redo\n");
		}
		freeReminderArray(&remindersList);
}

