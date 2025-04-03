#ifndef DATASAVE_H
#define DATASAVE_H

#include <string>

void datasave(bool b1[], int b3[], int b2[], int counter[]);
void thresholdSave(int threshold);
int thresholdLoad();
void saveBeta(double beta);
double loadBeta();

#endif
