//
//  @file KrsServoCheckId.ino
//  @brief KRS ID command check program
//  @author Kondo Kagaku Co.,Ltd.
//  @date 2020/02/20
//
//  IDを1に変更し、再度読み込んで書き込んだIDかどうかチェックします
//  ICSの通信にはSoftwareSerialを使います。
//  表示機がないので、実際に動いていないように見えます。
//	あくまでも記述方法を参考にしてください。
//

#include <IcsSoftSerialClass.h> 

const byte S_RX_PIN = 8;
const byte S_TX_PIN = 9;

const byte EN_PIN = 2;
const long BAUDRATE = 115200;
const int TIMEOUT = 100;

IcsSoftSerialClass krs(S_RX_PIN,S_TX_PIN,EN_PIN,BAUDRATE,TIMEOUT);  //インスタンス＋ENピン(2番ピン)およびUARTの設定、softSerial版

void setup() {
  // put your setup code here, to run once:
  krs.begin();  //サーボモータの通信初期設定
  Serial.begin(9600);
}

void loop() {

  const byte SET_ID = 1;

  int reId;

  //IDの設定
  Serial.print("Set ID =>");
  Serial.println(SET_ID,DEC);
  krs.setID(SET_ID);

  //IDの取得

  reId = krs.getID();
  Serial.print("Get ID =>");
  Serial.println(reId,DEC);
    
  if(reId != SET_ID)
  { 
    //失敗した時の処理
    Serial.println("ID Verify False");
    delay(1000);
    
  }
  else
  {
    Serial.println("ID Verify OK");
  }  
  

  for(;;);//ここで終わり
  
}
