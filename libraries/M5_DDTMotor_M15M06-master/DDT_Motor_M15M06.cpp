#include <Arduino.h>
#include "DDT_Motor_M15M06.h"

// Pico W用：RS485通信用のUARTとしてSerial1を使用し、方向制御ピン（GP11）を初期化
MotorHandler::MotorHandler(int RX, int TX)
{
    // RXピンとTXピンを設定（Picoの場合、RXはGP9、TXはGP8）
    Serial1.setRX(RX);  
    Serial1.setTX(TX);
    // 115200bps、8N1で初期化
    Serial1.begin(115200, SERIAL_8N1);
    
    // RS485の送受信切替用ピン（GP11）を出力に設定。初期は受信モード（LOW）にする。
    pinMode(11, OUTPUT);
    digitalWrite(11, LOW);
}

void MotorHandler::Control_Motor(uint16_t Speed, uint8_t ID, uint8_t Acce, uint8_t Brake_P, Receiver *Receiver)
{
    this->Tx[0] = ID;
    this->Tx[1] = 0x64;
    this->Tx[2] = Speed >> 8;
    this->Tx[3] = Speed & 0x00ff;
    this->Tx[4] = 0;
    this->Tx[5] = 0;
    this->Tx[6] = Acce;
    this->Tx[7] = Brake_P;
    this->Tx[8] = 0;
    this->Tx[9] = CRC8_Table(Tx.data(), 9);
    Send_Motor();

    Receive_Motor();
    Receiver->ID = this->Rx[0];
    Receiver->BMode = this->Rx[1];
    Receiver->ECurru = (this->Rx[2] << 8) + this->Rx[3];
    Receiver->BSpeed = (this->Rx[4] << 8) + this->Rx[5];
    Receiver->Position = (this->Rx[6] << 8) + this->Rx[7];
    Receiver->ErrCode = this->Rx[8];
}

void MotorHandler::Get_Motor(uint8_t ID, Receiver *Receiver)
{
    Tx[0] = ID;
    Tx[1] = 0x74;
    Tx[2] = 0;
    Tx[3] = 0;
    Tx[4] = 0;
    Tx[5] = 0;
    Tx[6] = 0;
    Tx[7] = 0;
    Tx[8] = 0;
    Tx[9] = CRC8_Table(Tx.data(), 9);
    Send_Motor();

    Receive_Motor();
    Receiver->ID = this->Rx[0];
    Receiver->BMode = this->Rx[1];
    Receiver->ECurru = (this->Rx[2] << 8) + this->Rx[3];
    Receiver->BSpeed = (this->Rx[4] << 8) + this->Rx[5];
    Receiver->Temp = this->Rx[6];
    Receiver->Position = this->Rx[7];
    Receiver->ErrCode = this->Rx[8];
}

void MotorHandler::Set_MotorMode(uint8_t Mode, uint8_t ID)
{
    Tx[0] = ID;
    Tx[1] = 0xA0;
    Tx[2] = 0;
    Tx[3] = 0;
    Tx[4] = 0;
    Tx[5] = 0;
    Tx[6] = 0;
    Tx[7] = 0;
    Tx[8] = 0;
    Tx[9] = Mode;
    Send_Motor();
}

void MotorHandler::Set_MotorID(uint8_t ID)
{
    Tx[0] = 0xAA;
    Tx[1] = 0x55;
    Tx[2] = 0x53;
    Tx[3] = ID;
    Tx[4] = 0;
    Tx[5] = 0;
    Tx[6] = 0;
    Tx[7] = 0;
    Tx[8] = 0;
    Tx[9] = 0;
    Send_Motor();
}

void MotorHandler::Check_Motor(Receiver *Receiver)
{
    Tx[0] = 0xc8;
    Tx[1] = 0x64;
    Tx[2] = 0;
    Tx[3] = 0;
    Tx[4] = 0;
    Tx[5] = 0;
    Tx[6] = 0;
    Tx[7] = 0;
    Tx[8] = 0;
    Tx[9] = CRC8_Table(Tx.data(), 9);

    Send_Motor();
    Receive_Motor();

    Receiver->ID = this->Rx[0];
    Receiver->BMode = this->Rx[1];
    Receiver->ECurru = (this->Rx[2] << 8) + this->Rx[3];
    Receiver->BSpeed = (this->Rx[4] << 8) + this->Rx[5];
    Receiver->Position = (this->Rx[6] << 8) + this->Rx[7];
    Receiver->ErrCode = this->Rx[8];
}

void MotorHandler::Send_Motor()
{
    // 送信前：方向制御ピン（GP11）をHIGHにして送信モードに設定
    digitalWrite(11, HIGH);
    Serial1.write(Tx.data(), 10);
    Serial1.flush();  // 送信完了待ち
    delay(10);        // 送信完了のための遅延（必要に応じて調整）
    // 送信後：方向制御ピンをLOWにして受信モードに戻す
    digitalWrite(11, LOW);
}

void MotorHandler::Receive_Motor()
{
    if (Serial1.available())
    {
        Serial1.readBytes(Rx.data(), 10);
    }
}

unsigned char MotorHandler::CRC8_Table(uint8_t *p, int counter)
{
    unsigned char crc8 = 0;
    // Model:CRC-8/MAXIM (polynomial x8 + x5 + x4 + 1)
    for (; counter > 0; counter--)
    {
        crc8 = CRC8Table[crc8 ^ *p];
        p++;
    }
    return crc8;
}
