#ifndef REMFILES_H
#define REMFILES_H

extern char* saveFilePath;

int system(const char*);

void intsToTimeString(char*, int, int);

void intsToDateString(char*, int, int, int);

void initFile(FILE*);

void rewriteFile(ReminderArray*, FILE*); 

ErrStat readFile(BST*, FILE*, int, void**);

#endif
