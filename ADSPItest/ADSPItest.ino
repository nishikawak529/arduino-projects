#include <Arduino.h>
#include <SPI.h>

#define CS_PIN 17  // MCP3008 の CS ピン

// SPI 設定
SPISettings spisettings(1000000, MSBFIRST, SPI_MODE0);

void setup() {
    Serial.begin(115200);
    SPI.begin();
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
}

// MCP3008 からアナログ値を取得する関数
int readMCP3008(int channel) {
    if (channel < 0 || channel > 7) return -1;

    SPI.beginTransaction(spisettings);
    digitalWrite(CS_PIN, LOW);

    SPI.transfer(0b00000001);
    byte highByte = SPI.transfer(0b10000000 | (channel << 4));
    byte lowByte = SPI.transfer(0x00);

    digitalWrite(CS_PIN, HIGH);
    SPI.endTransaction();

    return ((highByte & 0x03) << 8) | lowByte;
}

void loop() {
    int ch0Value = readMCP3008(0);
    int ch7Value = readMCP3008(7);

    Serial.print("CH0: ");
    Serial.print(ch0Value* 3.3f / (1 << 10));
    Serial.print("\tCH7: ");
    Serial.println(ch7Value* 3.3f / (1 << 10));

    delay(10);
}
