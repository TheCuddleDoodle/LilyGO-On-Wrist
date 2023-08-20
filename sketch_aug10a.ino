#include "cat.h"

#include "TFT_eSPI.h"

#include <WiFi.h>

#include "time.h"

#define BUTTON_PIN 16

TFT_eSPI lcd = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite( & lcd);
TFT_eSprite spr = TFT_eSprite( & lcd); //sprite for seconds
TFT_eSprite spr2 = TFT_eSprite( & lcd); //sprite for calendar
TFT_eSprite cal = TFT_eSprite( & lcd);

const char * ssid = "VAIBHAV";
const char * password = "12345678";

const char * ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 16200; //time zone * 3600 , my time zone is  +1 GTM
const int daylightOffset_sec = 3600;

#define gray 0x6B6D
#define blue 0x0AAD
#define orange 0xC260
#define purple 0x604D
#define green 0x1AE9

char timeHour[3];
char timeMin[3];
char timeSec[3];
char day[3];
char month[6];
char year[5];
char timeWeekDay[3];
String dayInWeek;
String IP;

int left = 0;
int right = 14;

RTC_DATA_ATTR int bootCount = 0;

RTC_DATA_ATTR time_t sleepEnterTime;
RTC_DATA_ATTR time_t initBootTime;
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

void setup(void) {
  pinMode(left, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);
  lcd.init();
  lcd.setRotation(1);
  sprite.createSprite(320, 170);
  sprite.setSwapBytes(true);
  sprite.setTextColor(TFT_WHITE, 0xEAA9);
  sprite.setTextDatum(4);
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);
  cal.createSprite(218, 26);
  cal.fillSprite(TFT_GREEN);
  cal.setTextColor(TFT_WHITE, TFT_GREEN);

  spr.createSprite(80, 40);
  spr.fillSprite(TFT_GREEN);
  spr2.createSprite(80, 64);
  spr2.fillSprite(TFT_GREEN);
  spr2.setTextDatum(4);
  spr.setTextColor(TFT_WHITE, TFT_GREEN);
  spr2.setTextColor(TFT_WHITE, TFT_GREEN);
  spr.setFreeFont( & Orbitron_Light_32);
  spr2.setFreeFont( & Orbitron_Light_24);

  Serial.begin(9600);
  if(bootCount ==0){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  IP = WiFi.localIP().toString();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  time( & initBootTime);
  }
 
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  print_wakeup_reason();

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_16, 0); //1 = High, 0 = Low

}

int x = 0;
long startF = 0;
long endF = 0;
double fps = 0;

void printLocalTime() {
  struct tm timeinfo;

  if (!getLocalTime( & timeinfo)) {

    return;
  }
   char buff[100];
  strftime (buff, 100, "from wifi %Y-%m-%d %H:%M:%S.000",  & timeinfo);
  Serial.println(buff);
  
  strftime(timeHour, 3, "%H", & timeinfo);
  strftime(timeMin, 3, "%M", & timeinfo);
  strftime(timeSec, 3, "%S", & timeinfo);

  strftime(timeWeekDay, 10, "%A", & timeinfo);
  dayInWeek = String(timeWeekDay);

  strftime(day, 3, "%d", & timeinfo);
  strftime(month, 6, "%B", & timeinfo);
  strftime(year, 5, "%Y", & timeinfo);
  struct tm stored_Time = *localtime(&initBootTime);
  char storedtime[100];
  strftime (storedtime, 100, "from storedtime %Y-%m-%d %H:%M:%S.000",  & stored_Time);
  Serial.println(storedtime);

}

void printLocalTime_from_boot() {
  time_t now;
  time( & now);
  struct tm timeinfo = * localtime( & now);
  if (!getLocalTime( & timeinfo)) {

    return;
  }
  char buff[100];
  strftime (buff, 100, "from local boot time %Y-%m-%d %H:%M:%S.000",  & timeinfo);
  Serial.println(buff);
  timeinfo.tm_hour += 5;
  timeinfo.tm_min += 30;
  if (bootCount != 0) {
    // The device has woken up from a deep sleep
    // Calculate elapsed time
    // adjust minutes and hours if seconds are over 60
    while (timeinfo.tm_sec >= 60) {
      timeinfo.tm_sec -= 60;
      timeinfo.tm_min++;
    }
    while (timeinfo.tm_min >= 60) {
      timeinfo.tm_min -= 60;
      timeinfo.tm_hour++;
    }
  }

  strftime(timeHour, 3, "%H", & timeinfo);
  strftime(timeMin, 3, "%M", & timeinfo);
  strftime(timeSec, 3, "%S", & timeinfo);

  strftime(timeWeekDay, 10, "%A", & timeinfo);
  dayInWeek = String(timeWeekDay);

  strftime(day, 3, "%d", & timeinfo);
  strftime(month, 6, "%B", & timeinfo);
  strftime(year, 5, "%Y", & timeinfo);

}



long t = 0;
long f = 0;
int xt = 230;
int yt = 8;

// uint32_t volt = (analogRead(4) * 2 * 3.3 * 1000) / 4096;

void offsetrtc(){
  time_t now;
  time( & now);
  struct tm timeinfo = * localtime( & now);


    // Your existing code to get timeinfo

    // Add offset of 5 hours and 30 minutes
    timeinfo.tm_hour += 5;
    timeinfo.tm_min += 30;

    // Adjust minutes and hours if they overflow
    if (timeinfo.tm_min >= 60) {
      timeinfo.tm_min -= 60;
      timeinfo.tm_hour++;
    }

    if (timeinfo.tm_hour >= 24) {
      timeinfo.tm_hour -= 24;
    }

    // Convert the timeinfo to a formatted string
    char buff[100];
    strftime(buff, 100, "after 2nd boot set time %Y-%m-%d %H:%M:%S.000", & timeinfo);
    Serial.println(buff);

    // Extract individual time components
    char timeHour[3];
    char timeMin[3];
    char timeSec[3];
    char day[3];
    char month[6];
    char year[5];

    strftime(timeHour, 3, "%H", & timeinfo);
    strftime(timeMin, 3, "%M", & timeinfo);
    strftime(timeSec, 3, "%S", & timeinfo);
    strftime(day, 3, "%d", & timeinfo);
    strftime(month, 6, "%B", & timeinfo);
    strftime(year, 5, "%Y", & timeinfo);
}


void loop() {
  //  if(digitalRead(left)==0)
  //  xt--;
  //  if(digitalRead(right)==0)
  //  xt++;

  startF = millis();

  spr.fillSprite(TFT_GREEN);
  spr2.fillSprite(TFT_GREEN);
  spr2.setFreeFont( & Orbitron_Light_24);
  sprite.pushImage(0, 0, aniWidth, aniHeigth, logo2[x]);
  sprite.setTextColor(purple, TFT_WHITE);
  sprite.fillRoundRect(xt, yt, 80, 26, 3, TFT_WHITE);
  sprite.fillRoundRect(xt, yt + 70, 80, 16, 3, TFT_WHITE);

  sprite.drawString(String(month) + "/" + String(day), xt + 40, yt + 70 + 8, 2);

  sprite.drawString(String(timeHour) + ":" + String(timeMin), xt + 40, yt + 13, 4);
  spr.drawString(String(timeSec), 4, 6);
  spr2.drawRoundRect(0, 0, 80, 34, 3, TFT_WHITE);

  spr2.drawString("FPS", 62, 14, 2);
  spr2.drawString(String((int) fps), 26, 14);
  spr2.drawString("CONNECTED", 40, 44, 2);
  spr2.setTextFont(0);
  spr2.drawString(IP, 40, 60);

  cal.drawRoundRect(0, 0, 217, 26, 3, TFT_WHITE);
  cal.drawString("Never Gonna Give You Up", 8, 4, 2);

  //cal.drawString(dayInWeek,20,30);

  cal.pushToSprite( & sprite, xt - 224, yt, TFT_GREEN);
  spr.pushToSprite( & sprite, xt + 4, yt + 22, TFT_GREEN);
  spr2.pushToSprite( & sprite, xt, yt + 70 + 16 + 6, TFT_GREEN);
  sprite.pushSprite(0, 0);
  //delay(20);

  if (t + 1000 < millis()) {
    if (bootCount != 1) {
      printLocalTime_from_boot();
    } else {
      printLocalTime();
    }
    t = millis();
  }

  if (f + 10000 < millis()) {
    if (bootCount == 1 ) {
      printLocalTime();
      Serial.println("dsfdsfefe");
      f = millis();
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      delay(200);
      esp_deep_sleep_start();
    }
    else{
     Serial.println("dsfdsfefe");
    f = millis();
    esp_deep_sleep_start();
   
  }
  }
  x++;
  if (x == framesNumber)
    x = 0;

  endF = millis();
  fps = 1000 / (endF - startF);
}
