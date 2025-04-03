#include <Arduino.h>
#include "DDT_Motor.h"

// ピン番号と RS485 DIR ピンをセットするのみ
MotorHandler::MotorHandler(int serial_port_RX, int serial_port_TX)
  : _rxPin(serial_port_RX)
  , _txPin(serial_port_TX)
{
  // RS485 方向制御ピンを初期化（送受信デフォルトは受信モード）
  pinMode(RS485_DIR_PIN, OUTPUT);
  digitalWrite(RS485_DIR_PIN, LOW);

  // ここではまだ Serial2.begin() は呼ばない！
}

// UART (Serial2) の初期化は begin() で行う
void MotorHandler::begin(uint32_t baudRate)
{
  Serial2.setRX(_rxPin); 
  Serial2.setTX(_txPin); 
  Serial2.begin(baudRate, SERIAL_8N1);
}



bool MotorHandler::Control_Motor(uint16_t Speed, uint8_t ID, uint8_t Mode, uint8_t Acce, uint8_t Brake_P, Receiver *Receiver)
{
    this->Tx[0] = ID;
    this->Tx[1] = 0x64;
    this->Tx[2] = Speed >> 8;
    this->Tx[3] = Speed & 0x00FF;
    this->Tx[4] = 0;
    this->Tx[5] = 0;
    this->Tx[6] = Acce;
    this->Tx[7] = Brake_P;
    this->Tx[8] = 0;
    this->Tx[9] = CRC8_Table(Tx.data(), 9);
    
    Send_Motor();

    // 受信の成功・失敗をチェック
    if (Receive_Motor(ID, Mode)) {
        Receiver->ID = this->Rx[0];
        Receiver->BMode = this->Rx[1];
        Receiver->ECurru = (int16_t)((this->Rx[2] << 8) | this->Rx[3]);
        Receiver->BSpeed = (int16_t)((this->Rx[4] << 8) | this->Rx[5]);
        Receiver->Position = (this->Rx[6] << 8) | this->Rx[7];
        Receiver->ErrCode = this->Rx[8];
        return true;  // 受信成功
    } else {
        return false; // 受信失敗
    }
}


bool MotorHandler::Get_Motor(uint8_t ID, uint8_t Mode, Receiver *Receiver)
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

    if (Receive_Motor(ID, Mode)) {
        Receiver->ID = Rx[0];
        Receiver->BMode = Rx[1];
        Receiver->ECurru = (int16_t)((Rx[2] << 8) | Rx[3]);
        Receiver->BSpeed = (int16_t)((Rx[4] << 8) | Rx[5]);
        Receiver->Temp = Rx[6];
        Receiver->Position = (Rx[7] << 8) | Rx[8];
        Receiver->ErrCode = Rx[9];
        return true;
    } else {
        return false;
    }
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

bool MotorHandler::Check_Motor(Receiver *Receiver)
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

    if (Receive_Motor_Check(Receiver)) {
        return true;  // 受信成功
    } else {
        return false; // 受信失敗
    }
}



void MotorHandler::Send_Motor(){
    // 送信前に受信バッファをクリア（ゴミデータ排除）
    while (Serial2.available() > 0) {
        Serial2.read();
    }

    // RS485送信モードに設定
    digitalWrite(11, HIGH);
    Serial2.write(Tx.data(), 10);
    Serial2.flush();  // 送信完了待ち
    digitalWrite(11, LOW);
}


bool MotorHandler::Receive_Motor(uint8_t expectedID, uint8_t expectedMode){
    unsigned long startTime = millis();  // タイムアウト用の開始時間

    // 【同期処理】expectedIDが来るまでバイトを読み捨てながら待つ
    while (true) {
        if (Serial2.available() > 0) {
            uint8_t byteIn = Serial2.read();
            if (byteIn == expectedID) {
                Rx[0] = byteIn;
                //Serial.print("[DEBUG] Synced on ID byte: ");
                //Serial.println(byteIn, HEX);
                break; // 同期完了
            } else {
                Serial.print("[DEBUG] Skipped byte (not ID): ");
                Serial.println(byteIn, HEX);
            }
        }
        if (millis() - startTime > 15) {
            Serial.println("[DEBUG] Timeout while waiting for ID sync");
            return false;
        }
    }

    // Modeバイト取得
    startTime = millis();
    while (Serial2.available() < 1) {
        if (millis() - startTime > 5) {
            Serial.println("[DEBUG] Timeout waiting for mode byte");
            return false;
        }
    }
    uint8_t modeByte = Serial2.read();
    //Serial.print("[DEBUG] Mode byte: ");
    //Serial.println(modeByte, HEX);
    if (modeByte != expectedMode) {
        Set_MotorMode(expectedMode, expectedID);
        Set_MotorMode(expectedMode, expectedID);
        Set_MotorMode(expectedMode, expectedID);
        Serial.print("[DEBUG] Unexpected mode byte: ");
        Serial.println(modeByte, HEX);
        return false;
    }
    Rx[1] = modeByte;

    // 残り8バイト取得
    startTime = millis();
    while (Serial2.available() < 8) {
        if (millis() - startTime > 10) {
            Serial.println("[DEBUG] Timeout waiting for remaining bytes");
            return false;
        }
    }
    Serial2.readBytes(Rx.data() + 2, 8);

    // ダンプ
    //Serial.print("[DEBUG] Frame: ");
    //for (int i = 0; i < 10; i++) {
    //    Serial.print(Rx[i], HEX);
    //    Serial.print(" ");
    //}
    //Serial.println();

    // CRC
    uint8_t crcCalc = CRC8_Table(Rx.data(), 9);
    if (crcCalc == Rx[9]) {
        //Serial.println("[DEBUG] CRC check passed");
        return true;
    } else {
        Serial.print("[DEBUG] CRC mismatch! Got ");
        Serial.print(Rx[9], HEX);
        Serial.print(" Calc ");
        Serial.println(crcCalc, HEX);
        return false;
    }
}




bool MotorHandler::Receive_Motor_Check(Receiver *Receiver)
{
    unsigned long startTime = millis();

    // 受信バッファが 10 バイト以上になるまで待つ（最大 5ms）
    while (Serial2.available() < 10) {
        if (millis() - startTime > 5) {
            Serial.println("Timeout waiting for Check_Motor data!");
            return false;
        }
    }

    // 10 バイトすべて受信
    Serial2.readBytes(Rx.data(), 10);

    // CRC8 チェック
    uint8_t calculatedCRC = CRC8_Table(Rx.data(), 9);
    if (calculatedCRC != Rx[9]) {
        Serial.print("CRC Mismatch in Check_Motor! Expected: ");
        Serial.print(Rx[9], HEX);
        Serial.print(", Calculated: ");
        Serial.println(calculatedCRC, HEX);
        return false;
    }

    // ID を取得
    Receiver->ID = Rx[0];

    // 受信データを保存
    Receiver->BMode = Rx[1];
    Receiver->ECurru = (int16_t)((Rx[2] << 8) | Rx[3]);
    Receiver->BSpeed = (int16_t)((Rx[4] << 8) | Rx[5]);
    Receiver->Position = (Rx[6] << 8) | Rx[7];
    Receiver->ErrCode = Rx[8];

    Serial.print("Valid Check_Motor data received! ID: ");
    Serial.println(Receiver->ID);

    return true;
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
