/*
 * main.cpp: simply forwards MIN to UART
 */
#include "Arduino.h"
#include "min.h"
#define S6C     Serial1
#define ODRIVE  Serial2

struct min_context min_ctx;
uint8_t cur_min_id; 

void min_tx_start(uint8_t port) {}
void min_tx_finished(uint8_t port) {}

uint16_t min_tx_space(uint8_t port)
{
  return ODRIVE.availableForWrite();
}

void min_tx_byte(uint8_t port, uint8_t byte)
{
  ODRIVE.write(&byte, 1U);  
}

uint32_t min_time_ms(void)
{
  return millis();
}

void min_application_handler(uint8_t min_id, uint8_t *min_payload, 
    uint8_t len_payload, uint8_t port)
{
  ODRIVE.write(min_payload, len_payload);
  cur_min_id = min_id + 1;
}

void setup()
{
  min_init_context(&min_ctx, 0);
}

void loop()
{
  char buf[32];
  size_t buf_len;

  if(S6C.available() > 0) {
    buf_len = S6C.readBytes(buf, 32U);
  }
  else {
    buf_len = 0;
  }

  min_poll(&min_ctx, (uint8_t *)buf, (uint8_t)buf_len);

  if (ODRIVE.available() > 0) {
    buf_len = ODRIVE.readBytes(buf, 32U);
    min_send_frame(&min_ctx, cur_min_id++, (uint8_t *)buf, (uint8_t)buf_len);
  }
}
