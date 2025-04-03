#include "TB6612FNG.h"

// コンストラクタ
TB6612FNG::TB6612FNG(int stbyPin, int ain1Pin, int ain2Pin, int pwmAPin, int bin1Pin, int bin2Pin, int pwmBPin)
: _stbyPin(stbyPin), _ain1Pin(ain1Pin), _ain2Pin(ain2Pin), _pwmAPin(pwmAPin),
  _bin1Pin(bin1Pin), _bin2Pin(bin2Pin), _pwmBPin(pwmBPin)
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
}

// モータ A を正転
void TB6612FNG::motorAForward(uint8_t speed) {
  digitalWrite(_ain1Pin, HIGH);
  digitalWrite(_ain2Pin, LOW);
  analogWrite(_pwmAPin, speed);
}

// モータ A を逆転
void TB6612FNG::motorABackward(uint8_t speed) {
  digitalWrite(_ain1Pin, LOW);
  digitalWrite(_ain2Pin, HIGH);
  analogWrite(_pwmAPin, speed);
}

// モータ A を停止 (ブレーキ)
void TB6612FNG::motorAStop() {
  digitalWrite(_ain1Pin, LOW);
  digitalWrite(_ain2Pin, LOW);
  analogWrite(_pwmAPin, 0);
}

// モータ B を正転
void TB6612FNG::motorBForward(uint8_t speed) {
  digitalWrite(_bin1Pin, HIGH);
  digitalWrite(_bin2Pin, LOW);
  analogWrite(_pwmBPin, speed);
}

// モータ B を逆転
void TB6612FNG::motorBBackward(uint8_t speed) {
  digitalWrite(_bin1Pin, LOW);
  digitalWrite(_bin2Pin, HIGH);
  analogWrite(_pwmBPin, speed);
}

// モータ B を停止 (ブレーキ)
void TB6612FNG::motorBStop() {
  digitalWrite(_bin1Pin, LOW);
  digitalWrite(_bin2Pin, LOW);
  analogWrite(_pwmBPin, 0);
}
