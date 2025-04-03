#ifndef DATASAVE_H
#define DATASAVE_H

#include <string>

void datasave(short b5[], bool b1[], int b3[], short b2[], int counter[]);
void thresholdSave(int threshold);
int thresholdLoad();
void saveBeta(double beta);
double loadBeta();

#endif
