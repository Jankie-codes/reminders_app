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
	
	//system("notify-send \"Dinner Is Ready\"");

  fptr = fopen("./reminders_save_file.txt","r");
	if (fptr == NULL) {
		printf("Error opening file\n");
		freeReminderArrayNotItems(&remindersList);
		freeBST(remindersBST);
		exit(1);
	}

	readFile(remindersBST, fptr, 3, NULL); //comment and uncomment this one
	fclose(fptr);

	//errHandle(parseArgsAddReminder(argc, argv, remindersBST, status), &remindersList, remindersBST, status);
	bstToArray(remindersBST, &remindersList);

	fptr = fopen("./reminders_save_file.txt","w");
	if (fptr == NULL) {
		printf("Error opening file\n");
		freeReminderArrayNotItems(&remindersList);
		freeBST(remindersBST);
		exit(1);
	}

	rewriteFile(&remindersList, fptr);
	fclose(fptr);
	freeReminderArrayNotItems(&remindersList);
	freeBST(remindersBST);

	//errHandle(EOKFINAL, &remindersList, remindersBST, status);
}
