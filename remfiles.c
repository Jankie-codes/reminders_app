#include<stdio.h>
#include<time.h>
#include<stdbool.h>
#include<string.h>
#include"errstat.h"
#include"reminders.h"

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
					//printf("Date: %d %d %d %d %d\n", month, day, year, hours, minutes);
					idFound = true;
					freeReminder(reminderToAdd);
				} else {
					addToBST(remindersBST, reminderToAdd);
				}
				break;
			}
		}
	}
	if (mode == 1 || mode == 2) {
		if (!idFound) {
			fclose(fptr);
			return EIDNOTFOUND;
		}
	}

	return EOK;
}
