//////////////////////////////////////////////////
//    ESP32C3_NTP_RTC_TH_003.ino
//       last edit: 2023.01.31
//                  ver.1.00
//       by:        jewelry.kingfisher
//////////////////////////////////////////////////

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <RTClib.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <HTTPClient.h>
#include <ST7032_asukiaaa.h>
#include <Wire.h>
#include "AM232X.h"

WiFiMulti WiFiMulti;
struct tm timeInfo;

ST7032_asukiaaa lcd;
AM232X AM2322;

int count = 0;
time_t timeNow = 0;       // time now 
time_t prevDisplay = 0;   // when the digital clock was displayed


float tt;
float hh;
int   AM2322status;

/////  READ AM2322 DATA
void readAM2322( ) {
  AM2322status = AM2322.read();
  switch (AM2322status) {
  case AM232X_OK:
    tt = AM2322.getTemperature();
    hh = AM2322.getHumidity();
    break;
  }
}


void setup() {
  char titlechr[] =  "NTPclock,Tmp,Hum";
  char startChar[] = "setup,[ver.1.00]";

  Serial.begin(115200);
  Serial.println();

  Wire.begin();        // init. I2C
  AM2322.begin();
  AM2322.wakeUp();
  delay(2000);
  Serial.begin(115200);

  lcd.begin(16, 2);     //LCD表示初期設定(16桁, 2行)
  lcd.setContrast(45);  //コントラスト(0〜63)

  lcd.setCursor(0, 0);  //LCD display
  lcd.print( titlechr );
  lcd.setCursor(0, 1);  //LCD display
  lcd.print( startChar );
  // Serial.println("===== setup started =====");
  // delay(1000);


  AM2322status = AM2322.read();   // dummy read
  delay(2000);
  AM2322status = AM2322.read();   // dummy read
  delay(2000);
  AM2322status = AM2322.read();
  // AM2322 "Sensor not found"
  if ( AM2322status != AM232X_OK ) {
    Serial.println("Sensor not found");
    lcd.setCursor(0, 0);  //LCD display
    lcd.print( "Sensor not found" );
    while (1);
  }


  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("SSID", "PASSWORD");    // setup here!!

  // wait for WiFi connection
  Serial.print("Waiting for WiFi to connect...");
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
  }
  Serial.println(" connected");
  Serial.println(WiFi.localIP());

///// config NTP to JST local time /////
  configTime(9 * 3600L, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp"); 

}


void LCDdisplay() {
  char timeChar[30];
  char dispA[16];
//  char dispChar[] =  "<<Hello!>>[XIAO ESP32C3]+[AM2322 Sensor]+[AQM1602Y]";
//  int numDispChar = strlen(dispChar);    // dispChar[] の文字数
  int i;
  char buff0;

  if ( getLocalTime(&timeInfo, 1000) == true ) {
    Serial.println("just get accurate NTP server time...");
    sprintf(timeChar, "%04d/%02d/%02d %02d:%02d:%02d",
        timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
        timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
    Serial.println( timeChar );
  }

///// for test //////////////////////////////////////////
  // Serial.print( "connecter IP: " );
  // Serial.println(WiFi.localIP());
////////////////////////////////////////////////////////

  // #1 line display NTP yyyy/mm/dd, and tt:mm:ss
  for ( i= 0; i < 16 ; i++) {
    dispA[i] = timeChar[i+3];    // "yyyy" display process below
  }
  // display "yyyy" turn around each 1 char, so y-->2-->0-->2-->--3-->y... 
  if ( count == 0 ) {
    buff0 = 'y'; } else {
    buff0 = timeChar[count-1];
  }
  dispA[0] = buff0;
  count++;
  if ( count > 4 ) { count = 0; }
  lcd.setCursor(0, 0);  //LCD display)
  lcd.print( dispA );

    
  // #2 line display Temperature, Humidity
  readAM2322();
  lcd.setCursor(0, 1);
  lcd.print("T=     ,  ");
  lcd.setCursor(3, 1);
  lcd.print(String(tt,1));
  lcd.setCursor(9, 1);
  lcd.print("H=    %");
  lcd.setCursor(11, 1);
  lcd.print(String(hh,1));


/////for test ///////////////////////////////////////6666///
  // Serial.println();
  // Serial.print("readAM2322,  T= ");
  // Serial.print( String(tt,1) );
  // Serial.print("   ,H= ");
  // Serial.println(String(hh,1));
///////////////////////////////////////////////////////

}


void loop() {
  getLocalTime(&timeInfo, 1000);
  timeNow = mktime(&timeInfo);
  if ( timeNow != prevDisplay) { //update the display only if time has changed
    prevDisplay = timeNow;
    LCDdisplay();
  }
}  
