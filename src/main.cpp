/*
 * main.cpp: simply forwards MIN to UART
 */

#include "Arduino.h"
#include "min.h"
#include <string.h>
#define S6C Serial1
#define ODRIVE Serial2
#define LED_PIN  13

uint8_t led_state;

struct min_context min_ctx;
uint8_t cur_min_id; 

char buf[64];
size_t buf_len;

const char kStartQuery[] = "spinmebaby";
const char kStopQuery[] = "stop";
const char kSpeedQuery[] = "speed";
const char kStartCommand[] = "w axis0.requested_state 5\n";
const char kStopCommand[]  = "w axis0.requested_state 1\n";

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
  } else if (strncmp(buf, kSpeedQuery, strlen(kSpeedQuery)) == 0) {
    Serial.println("Received speed!");
    int speed = 0;
    if (sscanf(buf, "speed %d", &speed) != EOF) {
      sprintf(buf, "v 0 %d\n", speed);
    } else {
      sprintf(buf, "v 0 0\n");
    }
  }
  size_t buf_len = strlen(buf);

  ODRIVE.write(buf, buf_len);
  //ODRIVE.write('\n');
  Serial.println("Message sent:");

  Serial.write(min_payload, buf_len);
  cur_min_id = min_id + 1;
}

void setup()
{
  S6C.begin(9600);
  ODRIVE.begin(115200);
  Serial.begin(9600);
  // while (!Serial);
  pinMode(LED_PIN, OUTPUT);
  min_init_context(&min_ctx, 0);
}

void loop()
{

  if(S6C.available() > 0) {
    buf_len = S6C.readBytes(buf, 64U);
    digitalWrite(LED_PIN, HIGH); 
    delay(100);
    digitalWrite(LED_PIN, LOW); 

  } else {
    buf_len = 0;
  }
  
  min_poll(&min_ctx, (uint8_t *)buf, (uint8_t)buf_len);

  if (ODRIVE.available() > 0) {
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
