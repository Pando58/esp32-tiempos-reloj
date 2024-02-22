#include "WiFi.h"
#include "time.h"
#include "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "RTClib.h"
#include "math.h"

#define out1 2
// #define out2 0
// #define out3 4

#define OLED_W 128
#define OLED_H 64
#define OLED_RESET -1
#define OLED_SCR_ADDR 0x3C

String ssid = "Wifi4us";
String pswd = "TLWA830REE";

const char* ntp_server = "pool.ntp.org";
const long gmt_offset_sec = -6 * 3600;
const int daylight_offset_sec = 0;

Adafruit_SSD1306 display(OLED_W, OLED_H, &Wire, OLED_RESET);

int wifi_counter = 14;

RTC_DS1307 rtc;

struct tm tm_update_time;
struct tm tm_now;

String ip;

void setup() {
  pinMode(out1, OUTPUT);

  digitalWrite(out1, LOW);

  tm_update_time.tm_hour = 5;
  tm_update_time.tm_min = 0;
  tm_update_time.tm_sec = 0;

  Serial.begin(115200);

  if (!rtc.begin()) {
    Serial.println("Error inicializando el reloj");
    Serial.flush();
    while (1) delay(10);
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_SCR_ADDR)) { // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  Serial.println("");
  Serial.print("Conectando a red " + ssid);
  display.clearDisplay();
  display.setCursor(0, 30);
  display.println(F("Conectando a WiFi..."));
  display.display();

  WiFi.begin(ssid, pswd);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);

    if (WiFi.status() == 1) {
      Serial.println("");
      Serial.println("\nEl SSID no existe");
      Serial.println("Continuando sin WiFi");
      Serial.println("");
      break;
    }

    wifi_counter--;
    if (wifi_counter <= 0) {
      Serial.println("");
      Serial.println("\nError conectando a la red");
      Serial.println("Continuando sin WiFi");
      Serial.println("");
      break;
    }
  }

  ip = WiFi.localIP().toString();

  if (WiFi.status() == 3) {
    Serial.println("");
    Serial.println("\nConectado");
    Serial.println("IP: " + ip + "\n");
  }
  display.clearDisplay();
  oledPrintNetworkInfo();

  updateSystemTime();
}

void loop() {
  // setTmNow();

  // checkTimeUpdate();

  // if (tm_now.tm_sec == 0 && !manualEnabled) {
  //   int wday = tm_now.tm_wday == 0 ? 6 : tm_now.tm_wday - 1;
  //
  //   for (int i = 0; i < n_tiempos; i++) {
  //     if (
  //       tabla_tiempos[wday][i][0] == tm_now.tm_hour &&
  //       tabla_tiempos[wday][i][1] == tm_now.tm_min
  //     ) {
  //       Serial.println("Evento: " + String(tabla_tiempos[wday][i][0]) + ":" + String(tabla_tiempos[wday][i][1]));
  //
  //       outState[0] = tabla_tiempos[wday][i][2] == 1;
  //       outState[1] = tabla_tiempos[wday][i][3] == 1;
  //       outState[2] = tabla_tiempos[wday][i][4] == 1;
  //
  //       digitalWrite(out1, tabla_tiempos[wday][i][2] == 1 ? HIGH : LOW);
  //       digitalWrite(out2, tabla_tiempos[wday][i][3] == 1 ? HIGH : LOW);
  //       digitalWrite(out3, tabla_tiempos[wday][i][4] == 1 ? HIGH : LOW);
  //
  //       break;
  //     }
  //   }
  // }

  // Serial.println(&tm_now);


  DateTime rtcnow = rtc.now();
  //
  Serial.print(rtcnow.year(), DEC);
  Serial.print('/');
  Serial.print(rtcnow.month(), DEC);
  Serial.print('/');
  Serial.print(rtcnow.day(), DEC);
  // Serial.print(" (");
  // Serial.print(rtcnow.dayOfTheWeek());
  // Serial.print(")");
  Serial.print(" ");
  Serial.print(rtcnow.hour(), DEC);
  Serial.print(':');
  Serial.print(rtcnow.minute(), DEC);
  Serial.print(':');
  Serial.print(rtcnow.second(), DEC);
  Serial.println();


  int h = rtcnow.minute();
  int m = rtcnow.second();
  int s = 0;

  if ((h >= 6 && h < 10) || (h >= 15 && h < 19)) {
    m = m % 30;

    // ON - 10 min
    if (m < 10) {
      Serial.print("ON  - ");
      printTimeMin(m, s);
      Serial.print(" / ");
      printTimeMin(10, 0);
    }
    // OFF - 20 min
    else {
      Serial.print("OFF - ");
      printTimeMin(m - 10, s);
      Serial.print(" / ");
      printTimeMin(20, 0);
    }
  } else if ((h >= 10 && h < 15)) {
    m = m % 20;

    // ON - 10 min
    if (m < 10) {
      Serial.print("ON  - ");
      printTimeMin(m, s);
      Serial.print(" / ");
      printTimeMin(10, 0);
    }
    // OFF - 10 min
    else {
      Serial.print("OFF - ");
      printTimeMin(m - 10, 0);
      Serial.print(" / ");
      printTimeMin(10, 0);
    }
  } else {
    m = (m + (((h - 19 + 24) % 24) * 60)) % 100;

    // ON - 10 min
    if (m < 10) {
      Serial.print("ON  - ");
      printTimeHour(floor(m / 60), m % 60, s);
      Serial.print(" / ");
      printTimeHour(0, 10, 0);
    }
    // OFF - 90 min
    else {
      Serial.print("OFF - ");
      printTimeHour(floor((m - 10) / 60), (m - 10) % 60, s);
      Serial.print(" / ");
      printTimeHour(1, 30, 0);
    }
  }

  Serial.println();


  delay(1000 - (millis() % 1000));
}

void updateSystemTime() {
  if (WiFi.status() != 3) {
    return;
  }

  display.fillRect(0, 56, 128, 10, SSD1306_BLACK);
  display.setCursor(0, 56);
  display.println(F("Actualizando hora..."));
  display.display();

  Serial.println("Actualizando hora...");

  configTime(gmt_offset_sec, daylight_offset_sec, ntp_server);

  time_t now;

  while (true) {
    time(&now);

    if (now > (2023 - 1970) * 365 * 24 * 60 * 60) {
        break;
    }
  }

  Serial.println("Hora actualizada\n");

  Serial.println("Actualizando RTC...");
  setTmNow();
  adjustRTC();
  Serial.println("RTC Actualizado");
}

void checkTimeUpdate() {
  if (
    tm_update_time.tm_hour == tm_now.tm_hour &&
    tm_update_time.tm_min == tm_now.tm_min &&
    tm_update_time.tm_sec == tm_now.tm_sec
  ) {
    updateSystemTime();
  }
}

void oledPrintNetworkInfo() {
  display.setCursor(0, 0);
  display.println(F("Conectado a red"));
  display.setCursor(0, 12);
  display.println(F(("SSID: " + ssid).c_str()));
  // display.setCursor(0, 38);
  // display.println(F(("IP: " + ip).c_str()));
  display.display();
}

void oledPrintTime() {
  String h = (tm_now.tm_hour < 10 ? "0" : "") + String(tm_now.tm_hour);
  String m = (tm_now.tm_min < 10 ? "0" : "") + String(tm_now.tm_min);
  String s = (tm_now.tm_sec < 10 ? "0" : "") + String(tm_now.tm_sec);

  display.fillRect(0, 56, 128, 10, SSD1306_BLACK);
  display.setCursor(41, 56);
  display.println(F((h + ":" + m + ":" + s).c_str()));
  display.display();
}

void setTmNow() {
  time_t now = time(NULL);
  tm_now = *localtime(&now);
}

void adjustRTC() {
  // Serial.println(&tm_now);
  rtc.adjust(DateTime(1900 + tm_now.tm_year, tm_now.tm_mon, tm_now.tm_mday, tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec));
}

void printTimeHour(int h, int m, int s) {
  if (h < 10) { Serial.print("0"); };
  Serial.print(h);
  Serial.print(":");
  if (m < 10) { Serial.print("0"); };
  Serial.print(m);
  Serial.print(":");
  if (s < 10) { Serial.print("0"); };
  Serial.print(s);
}

void printTimeMin(int m, int s) {
  if (m < 10) { Serial.print("0"); };
  Serial.print(m);
  Serial.print(":");
  if (s < 10) { Serial.print("0"); };
  Serial.print(s);
}
