/*************************************************************
 IoT Plant Watering System with Arduino UNO R4 WiFi
 
 Sistem penyiraman tanaman otomatis berbasis IoT menggunakan Arduino UNO R4 WiFi dan platform cloud Blynk
 
 Fitur:
 - Pembacaan kelembaban tanah via sensor FC-28
 - Klasifikasi kondisi tanah: Dry/Moderate/Wet
 - Kontrol pompa air otomatis via relay module
 - Monitoring dan kontrol manual via aplikasi Blynk
 - Penyimpanan status pompa di EEPROM
 - Tampilan kondisi tanah pada LED matrix Arduino
 
 Hardware:
 - Arduino UNO R4 WiFi
 - FC-28 Soil Moisture Sensor (pin A0)
 - 5V Relay Module (pin 8)
 - 5V Water Pump
 
 Blynk Virtual Pins:
 - V0: Soil Moisture (float, 0.0-100.0%)
 - V1: Water Pump Toggle (int, 0=OFF/1=ON)
 *************************************************************/

#define BLYNK_TEMPLATE_ID "TMPL66TIDdQ4W"
#define BLYNK_TEMPLATE_NAME "Remote Watering System"
#define BLYNK_AUTH_TOKEN "F_CxA1mCZuq_jmN-dm-z9VaGX49TVr-w"

#include <SPI.h>
#include <WiFiS3.h>
#include <BlynkSimpleWifi.h>
#include "Arduino_LED_Matrix.h"
#include <EEPROM.h>

#define moisture_sensor A0
#define relay 8 

BlynkTimer timer;
ArduinoLEDMatrix matrix;

/* buat connect ke hotspotnya Erin*/
char ssid[] = "Infinix NOTE 50 Pro";
char pass[] = "satusampaidelapan";

int eeprom_addr = 0;
int sensorValue = 0;
int pump_status = 0;
int prev_pump_status = 0;
float moist_percent = 0.00;

const uint32_t HAPPY_LED[] = {
  0x3fc48a95,
  0x58019fd9,
  0x5889871
};

const uint32_t NORMAL_LED[] = {
  0x3fc40298,
  0xd98d8019,
  0x5889871
};

const uint32_t SAD_LED[] = {
  0x3fc48a9d,
  0xd8898018,
  0x71889905
};

BLYNK_WRITE(V1) {
  pump_status = param.asInt();
  EEPROM.write(eeprom_addr, pump_status);
  prev_pump_status = EEPROM.read(eeprom_addr);
  Serial.println(prev_pump_status);
  Serial.println(pump_status);
}

void sendSensor() {
  Blynk.virtualWrite(V0, moist_percent);
}

void init_renesas_MCU_IO() {
  pinMode(relay, OUTPUT);
  pinMode(moisture_sensor, INPUT);
  analogReadResolution(12);
  matrix.begin();
}

void track_soil_moisture() {
  sensorValue = analogRead(moisture_sensor);
  moist_percent = 100 - ((float)sensorValue / 4096.0) * 100;

  if (moist_percent >= 0 && moist_percent < 33.33) {
    Serial.println("DRY");
    matrix.loadFrame(SAD_LED);
  }
  else if (moist_percent >= 33.33 && moist_percent < 66.66) {
    Serial.println("MODERATE");
    matrix.loadFrame(NORMAL_LED);
  }
  else if (moist_percent >= 66.66 && moist_percent <= 100) {
    Serial.println("WET");
    matrix.loadFrame(HAPPY_LED);
  }
}

void setup() {
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  init_renesas_MCU_IO();
  timer.setInterval(1000L, sendSensor);
  prev_pump_status = EEPROM.read(eeprom_addr);
  pump_status = prev_pump_status;
}

void loop() {
  Blynk.run();
  timer.run();
  track_soil_moisture();

  if (pump_status == 0) {
    Serial.println("Water pump is off");
    digitalWrite(relay, LOW);
  }
  else if (pump_status == 1) {
    Serial.println("Water pump is on");
    digitalWrite(relay, HIGH);
  }

  delay(500);
}