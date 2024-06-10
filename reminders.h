#ifndef REMINDERS_H
#define REMINDERS_H

typedef struct OptionalDateTime {
	time_t* value;
	bool* valid;
} OptionalDateTime;

typedef struct Reminder {
	OptionalDateTime* datetime;
	char* message;
	char* description;
	int* id;
	bool* notified;
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

time_t* newDateTime(int, int, int, int, int);

OptionalDateTime* mallocOptionalDateTime(time_t*, bool);

void freeOptionalDateTime(OptionalDateTime*);

int rmdCmp(Reminder*, Reminder*);

Reminder* makeReminder(char*, OptionalDateTime*, char*, bool);

void freeReminder(Reminder*);

void initReminderArray(ReminderArray*, size_t); 

void addToReminderArray(ReminderArray*, Reminder*);

void addReminder(char*, OptionalDateTime*, char*, bool, ReminderArray*);

void freeReminderArray(ReminderArray*);

void freeReminderArrayNotItems(ReminderArray*);

BST* initBST();

void addToBST(BST*, Reminder*); 

void bstToArray(BST*, ReminderArray*);

void printBST(BST*);

void freeBST(BST*); 

#endif
