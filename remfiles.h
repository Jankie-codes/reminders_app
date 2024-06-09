#ifndef REMFILES_H
#define REMFILES_H

void intsToTimeString(char*, int, int);

void intsToDateString(char*, int, int, int);

void rewriteFile(ReminderArray*, FILE*); 

ErrStat readFile(BST*, FILE*, int, void**);

#endif
