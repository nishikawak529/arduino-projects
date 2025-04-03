#ifndef TB6612FNG_H
#define TB6612FNG_H

#include <Arduino.h>
#include "RP2040_PWM.h"  // RP2040_PWMライブラリを追加

class TB6612FNG {
public:
    // コンストラクタ: 各ピン番号を指定
    TB6612FNG(int stbyPin, int ain1Pin, int ain2Pin, int pwmAPin, int bin1Pin, int bin2Pin, int pwmBPin);

    // 初期化: 各ピンを出力に設定し、PWM を有効化
    void init();

    // モータ A の制御 (周波数を指定可能)
    void motorAForward(uint16_t frequency, uint8_t dutyCycle);
    void motorABackward(uint16_t frequency, uint8_t dutyCycle);
    void motorAStop();

    // モータ B の制御
    void motorBForward(uint16_t frequency, uint8_t dutyCycle);
    void motorBBackward(uint16_t frequency, uint8_t dutyCycle);
    void motorBStop();

private:
    int _stbyPin;
    int _ain1Pin;
    int _ain2Pin;
    int _pwmAPin;
    int _bin1Pin;
    int _bin2Pin;
    int _pwmBPin;

    // RP2040_PWMのインスタンスを追加
    RP2040_PWM* _pwmA;
    RP2040_PWM* _pwmB;
};

#endif
