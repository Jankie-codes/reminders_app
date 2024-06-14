#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<stdbool.h>
#include <time.h>
#include<regex.h>
#include <sys/stat.h>
#include"errstat.h"
#include"reminders.h"
#include"remfiles.h"

int min(int a, int b) {
	if (b < a) {
		return b;
	} else {
		return a;
	}
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

ErrStat parseArgs(int argc, char** argv, void** status, int* pDateFields, int dateFieldsSize, bool* pChangeValid, char** pDescription, char** pMessage, int mode) {
	#define maxNumFlags 4
	char flags[maxNumFlags][3] = {"-d", "-t", "-e", "-m"};
	bool flagsSet[maxNumFlags] = {false, false, false, false};
	char* recentFlag = "";

	int numFlags = 3;
	if (mode == 2) {
		numFlags = 4;
	}

	if (mode == 1) {
		for (int i = 0; i < (dateFieldsSize - 2); i++) { //datetime by default
			setDateField("today", i, pDateFields);
		}
		setTimeField("12am", pDateFields);
	}

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
			if ((mode == 2) && (strcmp(argv[i], "none") == 0)) {
				*pChangeValid = true;
				flagsSet[0] = true;
				continue;
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
			*pDescription = argv[i];
			flagsSet[2] = true;
		} else if ((strcmp(flags[3], recentFlag) == 0) && (mode == 2)) {
			recentFlag = "";
			if (flagsSet[3]) {
				*status = malloc(sizeof(char)*3);
				*status = strcpy(*status, flags[3]);
				return EDUPFLAG;
			}
			*pMessage = argv[i];
			flagsSet[3] = true;
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
	
	switch (mode) {
		case 1:
			*pChangeValid = (flagsSet[0] || flagsSet[1]);
			break;
		case 2:
			if ((*pChangeValid == true) && flagsSet[1])  {
				return ETIMEOVERRIDDEN;
			} else if (argc == 3) {
				return ENOEDITFLAGS;
			}
			break;
	} 

	return EOK;
}

ErrStat parseArgsForEditing(int argc, char** argv, void** status, int* pDateFields, int dateFieldsSize, bool* pValid, char** pDescription, char** pMessage, int* pId) {
	regex_t regex;
	regcomp(&regex, "^[0-9]+$", REG_EXTENDED);

	if (argc == 2) {
		regfree(&regex);
		return ENOIDEDIT;
	}
	if (regexec(&regex, argv[2], 0, NULL, 0) != 0) {
		regfree(&regex);
		return EBADID;
	}

	*pId = atoi(argv[2]);

	ErrStat result = parseArgs(argc, argv, status, pDateFields, dateFieldsSize, pValid, pDescription, pMessage, 2);
	regfree(&regex);
	return result;
}

ErrStat parseArgsAddReminder(int argc, char** argv, BST* bst, void** status) {
	if (argc == 2) {
		return ENOMESSAGE;
	}
	char* message = argv[2];
	#define dateFieldsSize 5
	int dateFields[dateFieldsSize]; //month day years hours minutes
	int* pDateFields = dateFields;
	char* description = "";
	char** pDescription = &description;
	bool valid;
	bool* pValid = &valid;

	ErrStat result = parseArgs(argc, argv, status, pDateFields, dateFieldsSize, pValid, pDescription, NULL, 1);
	if (result != 0) {
		return result;
	}

	Reminder* reminderToAdd = makeReminder(message, mallocOptionalDateTime(newDateTime(dateFields[0], dateFields[1], dateFields[2], dateFields[3], dateFields[4]), *pValid), description, false);

	addToBST(bst, reminderToAdd);

	return EOK;
}

void errHandle(ErrStat errStat, ReminderArray* ra, BST* bst, char* saveFilePath, void** status) {
	switch (errStat) {
		case 0: 
			//ok
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
			perror("Error opening file");
			break;
		case 8:
			freeBST(bst);
			freeReminderArrayNotItems(ra);
			free(saveFilePath);
			free(status);
			exit(1);
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
		case 13:
			printf("Invalid command\n");
			break;
		case 14:
			printf("A reminders app written in C. Create and complete reminders with ease.\n");
			break;
		case 15:
			printf("No message specified. Please specify a message for the reminder to add\n");
			break;
		case 16:
			printf("No ID specified. Please specify a reminder ID for which reminder to edit\n");
			break;
		case 17:
			printf("WARNING: -d flag set to none, but -t flag was also specified. Overriding -t flag and simply removing both date and time\n");
			return;
		case 18:
			printf("No edit flags specified\n");
			break;
		case 19:
			printf("$HOME environment variable is not set. Cannot create/access save file\n");
			break;
	}
	freeBST(bst);
	freeReminderArrayNotItems(ra);
	free(saveFilePath);
	free(status);
	printf("\nAvailable commands:\n\n\
task add <description> \n\
\tcreates a new reminder. \n\
Available flags for TASK ADD: \n\
\t[-d] specify a date to notify about this reminder, in the format mm-dd-yy. If -t flag is not set, the time defaults to 12am. \n\
\t\tCan also use keywords \'mon\', \'tues\', \'wed\', \'thurs\', \'fri\', \'sat\', \'sun\', or any weekday's full name to schedule the reminder within the next 8 days.\n\
\t\tNote that if today's weekday is typed, then the date defaults to next week, NOT today.\n\
\t\tAlternatively keywords \'today\', \'td\', \'tomorrow\', and \'tmr\' also work.\n\
\t[-t] specify a time to notify about this reminder, in the format hh:mm(am/pm) OR hh(am/pm) for a specific hour. If -d flag is not set, the date defaults to today. \n\
\t[-e] specify an extra note for this reminder, to describe it in more detail.\n\n\
task (l | ls) \n\
\tlists all current reminders. Also reveals each reminder's id.\n\
No available flags for task l or task ls.\n\n\
task (remove | complete) <id> \n\
\tremoves a reminder with the given id. To view reminder ids, use task l or task ls. NOTE THAT individual reminder ids change as new reminders are added and deleted.\n\
No available flags for task remove or task complete.\n\n\
task edit <id> \n\
\tedits a reminder with the given id. To view reminder ids, use task l or task ls. NOTE THAT individual reminder ids change as new reminders are added and deleted.\n\
Available flags for TASK EDIT: \n\
\t[-m] specify a new description for this reminder.\n\
\t[-e] specify a new extra note for this reminder.\n\
\t[-d] specify a new date for this reminder, in the format mm-dd-yy. To remove BOTH DATE AND TIME from this reminder, use the keyword \'none\'.\n\
\t\tCan also use keywords \'mon\', \'tues\', \'wed\', \'thurs\', \'fri\', \'sat\', \'sun\', or any weekday's full name to schedule the reminder within the next 8 days.\n\
\t\tNote that if today's weekday is typed, then the date defaults to next week, NOT today.\n\
\t\tAlternatively keywords \'today\', \'td\', \'tomorrow\', and \'tmr\' also work.\n\
\t[-t] specify a new time for this reminder, in the format hh:mm(am/pm) OR hh(am/pm) for a specific hour.\n");
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
	printf("\t\t\t\t\t\t\t\tMy Reminders\n");
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
	printf("\n");
	return EOK;
}

ErrStat parseArgsRemoveReminder(int argc, char** argv) {
	regex_t regex;
	regcomp(&regex, "^[0-9]+$", REG_EXTENDED);
	
	if (argc < 3) {
		regfree(&regex);
		return ENOIDREMOVE;
	} else if (argc > 3) {
		regfree(&regex);
		return ETOOMANYARGS;
	}
	if (regexec(&regex, argv[2], 0, NULL, 0) != 0) {
		regfree(&regex);
		return EBADID;
	}
	regfree(&regex);
	return EOK;
}

int main(int argc, char** argv) {
		FILE* fptr;
		ReminderArray remindersList;
		BST* remindersBST = initBST(remindersBST);
		initReminderArray(&remindersList, 1);

		void** status = malloc(sizeof(void*));
		ErrStat errStat;

		char* saveFilePath = "";
		char** pSaveFilePath = &saveFilePath;
		errHandle(getSaveFilePath(pSaveFilePath), &remindersList, remindersBST, NULL, status);

		saveFilePath = *pSaveFilePath;
		
		if (argc < 2) {
			errHandle(ENOCOMMAND, &remindersList, remindersBST, saveFilePath, status);
		}

		struct stat buffer;

		if (stat(saveFilePath, &buffer) == -1) {
			fptr = fopen(saveFilePath, "w");
			if (fptr == NULL) {
				errHandle(EFILE, &remindersList, remindersBST, saveFilePath, status);
				exit(1);
			}
			initFile(fptr);
			fclose(fptr);
		}		
		if (strcmp("add", argv[1]) == 0) {
					fptr = fopen(saveFilePath,"r");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, saveFilePath, status);
						exit(1);
					}

					readFile(remindersBST, fptr, 0, NULL);
					fclose(fptr);

					errHandle(parseArgsAddReminder(argc, argv, remindersBST, status), &remindersList, remindersBST, saveFilePath, status);
					bstToArray(remindersBST, &remindersList);

					fptr = fopen(saveFilePath,"w");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, saveFilePath, status);
						exit(1);
					}

					rewriteFile(&remindersList, fptr);
					fclose(fptr);
					errHandle(EOKFINAL, &remindersList, remindersBST, saveFilePath, status);
		} else if (strcmp("l", argv[1]) == 0 || strcmp("ls", argv[1]) == 0) {
					fptr = fopen(saveFilePath,"r");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, saveFilePath, status);
						exit(1);
					}
					readFile(remindersBST, fptr, 0, NULL);
					fclose(fptr);
				
					bstToArray(remindersBST, &remindersList);

					printReminders(&remindersList);
					
					errHandle(EOKFINAL, &remindersList, remindersBST, saveFilePath, status);
		} else if (strcmp("edit", argv[1]) == 0) {
					int id;
					int* pId = &id;
				
					char* message = NULL;
					char** pMessage = &message;

					#define dateFieldsSize 5
					int dateFields[dateFieldsSize] = {-1, -1, -1, -1, -1}; //month day years hours minutes
					int* pDateFields = dateFields;

					char* description = NULL;
					char** pDescription = &description;

					bool removeDate = false;
					bool* pRemoveDate = &removeDate;

					errHandle(parseArgsForEditing(argc, argv, status, pDateFields, dateFieldsSize, pRemoveDate, pDescription, pMessage, pId), &remindersList, remindersBST, saveFilePath, status);

					void* fieldsToEditArray[5];

					void** fieldsToEdit = fieldsToEditArray;
					fieldsToEdit[0] = (void*) pId;
					fieldsToEdit[1] = (void*) message;
					fieldsToEdit[2] = (void*) description;
					fieldsToEdit[3] = (void*) pDateFields;
					fieldsToEdit[4] = (void*) pRemoveDate;
					
					fptr = fopen(saveFilePath,"r");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, saveFilePath, status);
						exit(1);
					}
					errHandle(readFile(remindersBST, fptr, 4, fieldsToEdit), &remindersList, remindersBST, saveFilePath, status);
					fclose(fptr);

					bstToArray(remindersBST, &remindersList);

					fptr = fopen(saveFilePath,"w");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, saveFilePath, status);
						exit(1);
					}

					rewriteFile(&remindersList, fptr);
					fclose(fptr);

					errHandle(EOKFINAL, &remindersList, remindersBST, saveFilePath, status);
		} else if (strcmp("remove", argv[1]) == 0) {
					errHandle(parseArgsRemoveReminder(argc, argv), &remindersList, remindersBST, saveFilePath, status);
					int idToRemove = atoi(argv[2]);
					int* pIdToRemove = &idToRemove;
					int** ppIdToRemove = &pIdToRemove;

					fptr = fopen(saveFilePath,"r");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, saveFilePath, status);
						exit(1);
					}
					errHandle(readFile(remindersBST, fptr, 1, (void**)ppIdToRemove), &remindersList, remindersBST, saveFilePath, status);
					fclose(fptr);

					bstToArray(remindersBST, &remindersList);

					fptr = fopen(saveFilePath, "w");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, saveFilePath, status);
						exit(1);
					}
					rewriteFile(&remindersList, fptr);
					fclose(fptr);
					errHandle(EOKFINAL, &remindersList, remindersBST, saveFilePath, status);
		} else if (strcmp("complete", argv[1]) == 0) {
					errHandle(parseArgsRemoveReminder(argc, argv), &remindersList, remindersBST, saveFilePath, status);
					int idToRemove = atoi(argv[2]);
					int* pIdToRemove = &idToRemove;
					int** ppIdToRemove = &pIdToRemove;

					fptr = fopen(saveFilePath,"r");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, saveFilePath, status);
						exit(1);
					}
					errHandle(readFile(remindersBST, fptr, 2, (void**)ppIdToRemove), &remindersList, remindersBST, saveFilePath, status);
					fclose(fptr);

					bstToArray(remindersBST, &remindersList);

					fptr = fopen(saveFilePath, "w");
					if (fptr == NULL) {
						errHandle(EFILE, &remindersList, remindersBST, saveFilePath, status);
						exit(1);
					}
					rewriteFile(&remindersList, fptr);
					fclose(fptr);
					errHandle(EOKFINAL, &remindersList, remindersBST, saveFilePath, status);
		}
		errHandle(EBADCOMMAND, &remindersList, remindersBST, saveFilePath, status);
}

