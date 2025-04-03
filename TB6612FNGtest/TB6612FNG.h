#ifndef TB6612FNG_H
#define TB6612FNG_H

#include <Arduino.h>

class TB6612FNG {
public:
    // コンストラクタ: 各ピン番号を指定
    TB6612FNG(int stbyPin, int ain1Pin, int ain2Pin, int pwmAPin, int bin1Pin, int bin2Pin, int pwmBPin);

    // 初期化: 各ピンを出力に設定し、STBY ピンを HIGH にしてドライバを有効化
    void init();

    // モータ A の制御
    // speed: 0～255 (analogWrite の値)
    void motorAForward(uint8_t speed);
    void motorABackward(uint8_t speed);
    void motorAStop();

    // モータ B の制御
    void motorBForward(uint8_t speed);
    void motorBBackward(uint8_t speed);
    void motorBStop();

private:
    int _stbyPin;
    int _ain1Pin;
    int _ain2Pin;
    int _pwmAPin;
    int _bin1Pin;
    int _bin2Pin;
    int _pwmBPin;
};

#endif
