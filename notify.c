#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<time.h>
#include"errstat.h"
#include"reminders.h"
#include"remfiles.h"


int system(const char* command);

int main() {
	FILE* fptr;
	ReminderArray remindersList;
	BST* remindersBST = initBST(remindersBST);
	initReminderArray(&remindersList, 1);

	char* saveFilePath = "";
	char** pSaveFilePath = &saveFilePath;
	getSaveFilePath(pSaveFilePath);
	saveFilePath = *pSaveFilePath;
	
  fptr = fopen(saveFilePath,"r");
	if (fptr == NULL) {
		printf("Error opening file\n");
		freeReminderArrayNotItems(&remindersList);
		freeBST(remindersBST);
		free(saveFilePath);
		exit(1);
	}

	readFile(remindersBST, fptr, 3, NULL);
	fclose(fptr);

	bstToArray(remindersBST, &remindersList);

	fptr = fopen(saveFilePath,"w");
	if (fptr == NULL) {
		printf("Error opening file\n");
		freeReminderArrayNotItems(&remindersList);
		freeBST(remindersBST);
		free(saveFilePath);
		exit(1);
	}

	rewriteFile(&remindersList, fptr);
	fclose(fptr);
	freeReminderArrayNotItems(&remindersList);
	freeBST(remindersBST);
	free(saveFilePath);
}
