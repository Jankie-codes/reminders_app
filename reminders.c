#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<time.h>
#include"reminders.h"

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

