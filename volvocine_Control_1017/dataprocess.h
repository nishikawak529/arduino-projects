#ifndef DATAPROCESS_H
#define DATAPROCESS_H

short calculateDynamicThreshold(short *data, int length);
float calculateAmplitude(float f, float Fz);
float sigmoid(float x);  // sigmoid関数のプロトタイプ

#endif // DATAPROCESS_H
