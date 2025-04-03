#include "IcsHardSerialClass.h"
#include "hardware/uart.h"

/**
*	@brief コンストラクタ
**/
IcsHardSerialClass::IcsHardSerialClass() : icsUART(nullptr), enPin(0), baudRate(115200), timeOut(100) {}

/**
*	@brief コンストラクタ
*	@param[in] *uart ハードウェアUART (uart0 または uart1)
* 	@param[in] enpin 送受信切替えピンの番号
**/
IcsHardSerialClass::IcsHardSerialClass(uart_inst_t* uart, byte enpin) 
  : icsUART(uart), enPin(enpin), baudRate(115200), timeOut(100) {}

/**
* @brief コンストラクタ
* @param[in] *uart ハードウェアUART (uart0 または uart1)
* @param[in] enpin 送受信切替えピンの番号
* @param[in] baudrate 通信速度
* @param[in] timeout 受信タイムアウト(ms)
**/
IcsHardSerialClass::IcsHardSerialClass(uart_inst_t* uart, byte enpin, long baudrate, int timeout)
  : icsUART(uart), enPin(enpin), baudRate(baudrate), timeOut(timeout) {}

/**
* @brief デストラクタ
**/
IcsHardSerialClass::~IcsHardSerialClass() {
    if (icsUART) {
        uart_deinit(icsUART);
    }
}

/**
* @brief 通信の初期設定
* @retval true 成功
* @retval false 失敗
**/
bool IcsHardSerialClass::begin() {
    if (icsUART == nullptr) {
        return false;
    }
    uart_init(icsUART, baudRate);
    gpio_set_function(enPin, GPIO_FUNC_SIO);
    gpio_set_dir(enPin, GPIO_OUT);
    enLow(); // 受信モード
    return true;
}

bool IcsHardSerialClass::begin(long baudrate, int timeout) {
    baudRate = baudrate;
    timeOut = timeout;
    return begin();
}

bool IcsHardSerialClass::begin(uart_inst_t* uart, int enpin, long baudrate, int timeout) {
    icsUART = uart;
    enPin = enpin;
    baudRate = baudrate;
    timeOut = timeout;
    return begin();
}

/**
* @brief ICS通信の送受信
**/
bool IcsHardSerialClass::synchronize(byte* txBuf, byte txLen, byte* rxBuf, byte rxLen) {
    int rxSize = 0;
    if (icsUART == nullptr) {
        return false;
    }

    uart_tx_wait_blocking(icsUART);
    enHigh(); // 送信モード
    sleep_us(200);
    uart_write_blocking(icsUART, txBuf, txLen);
    uart_tx_wait_blocking(icsUART);

    enLow(); // 受信モード
    sleep_us(200);

    uint64_t startTime = millis();
    while (rxSize < rxLen) {
        if (uart_is_readable(icsUART)) {
            rxBuf[rxSize++] = uart_getc(icsUART);
        }
        if (millis() - startTime > timeOut) {
            return false;
        }
    }
    return (rxSize == rxLen);
}

/**
* @brief サーボのIDを設定する（プロトコルに合わせて実装してください）
**/
bool IcsHardSerialClass::setID(byte id) {
  // 例: コマンドフォーマット {0xAA, 0x55, 0x01, id}
  byte cmd[] = {0xAA, 0x55, 0x01, id};
  byte response[2];

  // デバッグ: 送信するコマンドをシリアル出力
  Serial.print("setID: Sending command: ");
  for (byte i = 0; i < sizeof(cmd); i++) {
      Serial.print(cmd[i], HEX);
      Serial.print(" ");
  }
  Serial.println();

    // デバッグ: 受信した応答をシリアル出力
    Serial.print("setID: Received response: ");
    for (byte i = 0; i < sizeof(response); i++) {
        Serial.print(response[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

  if (!synchronize(cmd, sizeof(cmd), response, 2)) {
      Serial.println("setID: synchronize() failed");
      return false;
  }

  // 仮のチェック: 受信応答の2バイト目が設定したIDと一致すれば成功
  return (response[1] == id);
}


/**
* @brief サーボのIDを取得する（プロトコルに合わせて実装してください）
**/
int IcsHardSerialClass::getID() {
    // 例: コマンドフォーマット {0xAA, 0x55, 0x02} でID要求
    byte cmd[] = {0xAA, 0x55, 0x02};
    byte response[1];
    if (!synchronize(cmd, sizeof(cmd), response, 1)) {
        return -1;
    }
    return response[0];
}
