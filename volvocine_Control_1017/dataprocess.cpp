#include "dataprocess.h"
#include <algorithm> // std::sort
#include <cmath>     // std::floor, std::ceil, std::abs sigmoid計算のために必要

// 閾値を動的に計算する関数
short calculateDynamicThreshold(short *data, int length) {
    const int windowsize = 200;
    short sortedData[windowsize];
    short threshold = 0;

    if (length < windowsize) {
        // データが不足している場合は、全データを使用
        short* tempData = new short[length];
        std::copy(data, data + length, tempData);
        std::sort(tempData, tempData + length);
        
        int lower10Index = std::floor(0.1 * length);
        int upper10Index = std::ceil(0.9 * length);
        
        float lower10Value = tempData[lower10Index];
        float upper10Value = tempData[upper10Index];

        threshold = (lower10Value + upper10Value) / 2;
        delete[] tempData;
    } else {
        // 最新のwindowsize個のデータを使用
        std::copy(data + length - windowsize, data + length, sortedData);
        std::sort(sortedData, sortedData + windowsize);

        int lower10Index = std::floor(0.1 * windowsize);
        int upper10Index = std::ceil(0.9 * windowsize);
        
        float lower10Value = sortedData[lower10Index];
        float upper10Value = sortedData[upper10Index];

        threshold = (lower10Value + upper10Value) / 2;
    }
    
    return threshold;
}



// 振幅の解を計算する関数
float calculateAmplitude(float f, float Fz) {
    // 係数を定義
    const float C1 = -0.0434;
    const float C2 = 2.1260;
    const float C3 = 0.00078;
    const float C4 = 0.0919;
    const float C5 = -0.7468;
    const float C6 = 5.9372;

    // 解を求める範囲
    const float minA = 0;
    const float maxA = 60.0;
    const float step = 1;

    // 最良の解を探す
    float bestA = minA;
    float smallestError = std::abs(sigmoid(C1 * minA + C2 * f + C3 * minA * minA + C4 * f * minA + C5 * f * f - C6) - Fz);

    for (float A = minA; A <= maxA; A += step) {
        // Fz_eqnの計算
        float Fz_eqn = sigmoid(C1 * A + C2 * f + C3 * A * A + C4 * f * A + C5 * f * f - C6);
        float error = std::abs(Fz_eqn - Fz);

        if (error < smallestError) {
            bestA = A;
            smallestError = error;
        }
    }

    // 解が範囲外であれば調整
    if (bestA < minA) {
        return minA;
    } else if (bestA > maxA) {
        return maxA;
    } else {
        return bestA;
    }
}

// Sigmoid関数の定義
float sigmoid(float x) {
    return 1.0 / (1.0 + std::exp(-x));
}
