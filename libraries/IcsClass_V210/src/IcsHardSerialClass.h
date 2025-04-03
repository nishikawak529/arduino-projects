#ifndef ICS_HARD_SERIAL_CLASS_H
#define ICS_HARD_SERIAL_CLASS_H

#include <Arduino.h>
#include "hardware/uart.h"

class IcsHardSerialClass {
public:
    IcsHardSerialClass();
    IcsHardSerialClass(uart_inst_t* uart, byte enpin);
    IcsHardSerialClass(uart_inst_t* uart, byte enpin, long baudrate, int timeout);
    ~IcsHardSerialClass();

    bool begin();
    bool begin(long baudrate, int timeout);
    bool begin(uart_inst_t* uart, int enpin, long baudrate, int timeout);
    bool synchronize(byte* txBuf, byte txLen, byte* rxBuf, byte rxLen);

    // サーボのID設定／取得用の関数（プロトコルに合わせて実装してください）
    bool setID(byte id);
    int getID();

    inline void enHigh() { digitalWrite(enPin, HIGH); }
    inline void enLow() { digitalWrite(enPin, LOW); }

private:
    uart_inst_t* icsUART;  // Pico のハードウェアUART (uart0 または uart1)
    byte enPin;
    long baudRate;
    int timeOut;
};

#endif // ICS_HARD_SERIAL_CLASS_H
