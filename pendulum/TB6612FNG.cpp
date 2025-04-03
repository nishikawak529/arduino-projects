#include "TB6612FNG.h"

// コンストラクタ
TB6612FNG::TB6612FNG(int stbyPin, int ain1Pin, int ain2Pin, int pwmAPin, int bin1Pin, int bin2Pin, int pwmBPin)
: _stbyPin(stbyPin), _ain1Pin(ain1Pin), _ain2Pin(ain2Pin), _pwmAPin(pwmAPin),
  _bin1Pin(bin1Pin), _bin2Pin(bin2Pin), _pwmBPin(pwmBPin), _pwmA(nullptr), _pwmB(nullptr)
{
}

// ピン設定と STBY ピンの有効化
void TB6612FNG::init() {
    pinMode(_stbyPin, OUTPUT);
    pinMode(_ain1Pin, OUTPUT);
    pinMode(_ain2Pin, OUTPUT);
    pinMode(_pwmAPin, OUTPUT);
    pinMode(_bin1Pin, OUTPUT);
    pinMode(_bin2Pin, OUTPUT);
    pinMode(_pwmBPin, OUTPUT);

    // STBY を HIGH にしてドライバを有効化
    digitalWrite(_stbyPin, HIGH);

    // PWMインスタンスを作成（デフォルトの周波数は1kHz, デューティ比0%）
    _pwmA = new RP2040_PWM(_pwmAPin, 1000, 0);
    _pwmB = new RP2040_PWM(_pwmBPin, 1000, 0);
}

// モータ A を正転（周波数とデューティ比を指定可能）
void TB6612FNG::motorAForward(uint16_t frequency, uint8_t dutyCycle) {
    digitalWrite(_ain1Pin, HIGH);
    digitalWrite(_ain2Pin, LOW);
    _pwmA->setPWM(_pwmAPin, frequency, dutyCycle);
}

// モータ A を逆転
void TB6612FNG::motorABackward(uint16_t frequency, uint8_t dutyCycle) {
    digitalWrite(_ain1Pin, LOW);
    digitalWrite(_ain2Pin, HIGH);
    _pwmA->setPWM(_pwmAPin, frequency, dutyCycle);
}

// モータ A を停止 (ブレーキ)
void TB6612FNG::motorAStop() {
    digitalWrite(_ain1Pin, LOW);
    digitalWrite(_ain2Pin, LOW);
    _pwmA->setPWM(_pwmAPin, 1000, 0);  // デューティ比0%にして停止
}

// モータ B を正転
void TB6612FNG::motorBForward(uint16_t frequency, uint8_t dutyCycle) {
    digitalWrite(_bin1Pin, HIGH);
    digitalWrite(_bin2Pin, LOW);
    _pwmB->setPWM(_pwmBPin, frequency, dutyCycle);
}

// モータ B を逆転
void TB6612FNG::motorBBackward(uint16_t frequency, uint8_t dutyCycle) {
    digitalWrite(_bin1Pin, LOW);
    digitalWrite(_bin2Pin, HIGH);
    _pwmB->setPWM(_pwmBPin, frequency, dutyCycle);
}

// モータ B を停止 (ブレーキ)
void TB6612FNG::motorBStop() {
    digitalWrite(_bin1Pin, LOW);
    digitalWrite(_bin2Pin, LOW);
    _pwmB->setPWM(_pwmBPin, 1000, 0);  // デューティ比0%にして停止
}
