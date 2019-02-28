/*
 * main.cpp: simply forwards MIN to UART
 */

#include "Arduino.h"
#include "min.h"
#include "ODriveArduino.h"
#include <string.h>
#define S6C Serial1
#define ODRIVE Serial2
#define LED_PIN  13

bool battery_warned = false;
struct min_context min_ctx;
uint8_t cur_min_id; 

char buf[64];
size_t buf_len;

unsigned long battery_time;
const float kMinVoltage = 13.0;
const int kNumWarnings = 10;
const unsigned int kODriveTimeout = 1000;
const char kStartQuery[] = "spinmebaby";
const char kStopQuery[] = "stop";
const char kSpeedQuery[] = "speed";
const char kBatteryQuery[]  = "battery";
const char kStartCommand[] = "w axis0.controller.vel_ramp_enable 1\n"
                             "w axis0.requested_state 5\n";
const char kStopCommand[]  = "w axis0.requested_state 1\n";
const char kSpeedCommand[] = "w axis0.controller.vel_ramp_target %d\n";
const char kBatteryCommand[]  = "r vbus_voltage\n";
const char kBatteryError[]  = "BATTERY LOW\n";

void min_tx_start(uint8_t port) {}
void min_tx_finished(uint8_t port) {}

uint16_t min_tx_space(uint8_t port)
{
  return ODRIVE.availableForWrite();
}

void min_tx_byte(uint8_t port, uint8_t byte)
{
  S6C.write(&byte, 1U);  
}

uint32_t min_time_ms(void)
{
  return millis();
}

void min_application_handler(uint8_t min_id, uint8_t *min_payload, 
    uint8_t len_payload, uint8_t port)
{
  min_payload++;
  len_payload--; 

  char* buf = (char*) min_payload;

  if (strncmp(buf, kStartQuery, strlen(kStartQuery)) == 0) {
    Serial.println("Received start!");
    strncpy(buf, kStartCommand, strlen(kStartCommand) + 1); 
  } else if (strncmp(buf, kStopQuery, strlen(kStopQuery)) == 0) {
    Serial.println("Received stop!");
    strncpy(buf, kStopCommand, strlen(kStopCommand) + 1);
  } else if (strncmp(buf, kBatteryQuery, strlen(kBatteryQuery)) == 0) {
    Serial.println("Received battery!");
    strncpy(buf, kBatteryCommand, strlen(kStopCommand) + 1);
  } else if (strncmp(buf, kSpeedQuery, strlen(kSpeedQuery)) == 0) {
    Serial.println("Received speed!");
    int speed = 0;
    if (sscanf(buf, "speed %d", &speed) != EOF) {
      sprintf(buf, kSpeedCommand, speed);
    } else {
      sprintf(buf, kSpeedCommand, 0); ;
    }
  }
  buf_len = strlen(buf);
  ODRIVE.write(buf, buf_len);
  Serial.println("Message sent:");
  Serial.write(min_payload, buf_len);
  cur_min_id = min_id + 1;
}

void setup()
{
  S6C.begin(9600);
  ODRIVE.begin(115200);
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  min_init_context(&min_ctx, 0);
  battery_time = millis();
}

void loop()
{
  Serial.println("Looping!");
  if (millis() - battery_time > 5000 && !battery_warned) {
    Serial.println("Battery check!");
    ODRIVE.write(kBatteryCommand, strlen(kBatteryCommand));

    unsigned long start = millis();
    while(ODRIVE.available() > 0 && millis() - start < kODriveTimeout) { 
      Serial.println("Waiting!");
      delay(10); // TODO: add timeout
    }
    ODRIVE.readBytes(buf, 64U);
    float voltage;
    sscanf(buf, "%f\n", &voltage);

    if (voltage < kMinVoltage) {
      Serial.println("Bad voltage!");
      strncpy(buf, kStopCommand, strlen(kStopCommand) + 1);
      buf_len = strlen(buf);
      ODRIVE.write(buf, buf_len);

      buf_len = strlen(kBatteryError);
      strncpy(buf+2, kBatteryError, buf_len);
      buf[0] = 0; // MESSAGE_SEND
      buf[1] = buf_len;

      for (int i = 0; i < kNumWarnings; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        delay(500);
        min_send_frame(&min_ctx, cur_min_id++, (uint8_t *)buf, (uint8_t)(buf_len + 2));
      }
      battery_warned = true;
    }
    battery_time = millis();
  }

  if (S6C.available() > 0) {
    Serial.println("S6C has a message!");
    buf_len = S6C.readBytes(buf, 64U);
    digitalWrite(LED_PIN, HIGH); 
    delay(100);
    digitalWrite(LED_PIN, LOW); 
  } else {
    buf_len = 0;
  }
  
  Serial.println("Polling!");
  min_poll(&min_ctx, (uint8_t *)buf, (uint8_t)buf_len);
  Serial.println("Done polling!");

  if (ODRIVE.available() > 0) {
    Serial.println("Reading!");
    buf_len = ODRIVE.readBytes(buf+2, 62U);
    Serial.println("Message received: ");
    for (uint8_t i = 0; i < buf_len; i++) {
      Serial.write(buf[i+2]);
    }
    buf[0] = 0; // MESSAGE_SEND
    buf[1] = buf_len;
    min_send_frame(&min_ctx, cur_min_id++, (uint8_t *)buf, (uint8_t)(buf_len + 2));
  }

  delay(10);
}
