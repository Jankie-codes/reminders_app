#include<stdio.h>
#include<time.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include <sys/stat.h>
#include"errstat.h"
#include"reminders.h"

char* saveFilePathAfterHome = ".cache/task";

ErrStat getSaveFilePath(char** pSaveFilePath) {
	const char* home = getenv("HOME");
  if (home == NULL) {
     return ENOHOMEENV;
	}
	char* saveFilePath = malloc(sizeof(char) * (strlen(home) + 1 + strlen(saveFilePathAfterHome) + 1));

	sprintf(saveFilePath, "%s/.cache/task", home);

	*pSaveFilePath = saveFilePath;

	return EOK;
}

int system(const char* command);

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

void initFile(FILE* fptr) {
	fprintf(fptr, "-1\n");
	fprintf(fptr, "NO-READ-PAST-THIS-POINT\n");
}

void rewriteFile(ReminderArray* remindersList, FILE* fptr) {
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
			fprintf(fptr, "%d\n", *(remindersList->array[i]->notified));
		}
		fprintf(fptr, "-1\n");
		fprintf(fptr, "NO-READ-PAST-THIS-POINT\n");
}

ErrStat readFile(BST* remindersBST, FILE* fptr, int mode, void** info) {
	bool idFound = false;
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
	
		int notified;
		fscanf(fptr, "%d\n", &notified);
		
		Reminder* reminderToAdd = makeReminder(message, mallocOptionalDateTime(newDateTime(month, day, year, hours, minutes), valid), desc, notified);
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
					idFound = true;
					freeReminder(reminderToAdd);
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
					idFound = true;
					freeReminder(reminderToAdd);
				} else {
					addToBST(remindersBST, reminderToAdd);
				}
				break;
			}
			case 3: {
				if (!valid || notified) {
					addToBST(remindersBST, reminderToAdd);
					break;
				}
				time_t now;
			  time(&now);

				time_t* reminderTime = newDateTime(month, day, year, hours, minutes);

				double timeSinceReminder = difftime(now, *reminderTime);

				if (timeSinceReminder > 0) {
					char commandString[messageLen + 14];
					sprintf(commandString, "notify-send \"%s\"", message);
					if (timeSinceReminder <= 60) {
						system(commandString);
					} else {
						system(commandString);
					}
					*reminderToAdd->notified = true;
				}

				addToBST(remindersBST, reminderToAdd);
				free(reminderTime);	
				break;
			}
			case 4: {
				int idToEdit = *((int*) info[0]);
				char* newMessage;
				char* newDesc;
				int* newDateArray;
				bool removeDate;

				if (idToEdit != id) {
					addToBST(remindersBST, reminderToAdd);
					break;
				}

				if (info[1]) {
					newMessage = (char*) info[1];
					reminderToAdd->message =  realloc(reminderToAdd->message, sizeof(char)*(1+strlen(newMessage)));
					strcpy(reminderToAdd->message, newMessage);
				}
				if (info[2]) {
					newDesc = (char*) info[2];
					reminderToAdd->description = realloc(reminderToAdd->description, sizeof(char)*(1+strlen(newDesc)));
					strcpy(reminderToAdd->description, newDesc);
				} 
				if (info[3]) {
					newDateArray = (int*) info[3];
					if ((newDateArray[0] != -1 || newDateArray[3] != -1) && !(*reminderToAdd->datetime->valid)) {
						struct tm* pDate;
						time_t now = time(0);
						pDate = localtime(&now);	
						month = pDate->tm_mon;
						day = pDate->tm_mday;
						year = pDate->tm_year;
						hours = 0;
						minutes = 0;
						*reminderToAdd->datetime->valid = true;
					}
					if (newDateArray[0] != -1) {
						month = newDateArray[0];
						day = newDateArray[1];
						year = newDateArray[2];
					}
					if (newDateArray[3] != -1) {
						hours = newDateArray[3];
						minutes = newDateArray[4];
					}
					free(reminderToAdd->datetime->value);
					reminderToAdd->datetime->value = newDateTime(month, day, year, hours, minutes);
				}
				if (info[4]) {
					removeDate = *((bool*) info[4]);
					if (removeDate) {
						*reminderToAdd->datetime->valid = false;
					}
				}

				*reminderToAdd->notified = false;
				addToBST(remindersBST, reminderToAdd);
				idFound = true;
				break;
			}
		}
	}
	if (mode == 1 || mode == 2 || mode == 4) {
		if (!idFound) {
			fclose(fptr);
			return EIDNOTFOUND;
		}
	}

	return EOK;
}
