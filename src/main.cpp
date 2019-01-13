/*
 * main.cpp: simply forwards MIN to UART
 */
#include "Arduino.h"
#include "min.h"

struct min_context min_ctx;
uint32_t last_sent;

uint16_t min_tx_space(uint8_t port)
{
  return ODrive.availableForWrite();
}

void min_tx_byte(uint8_t port, uint8_t byte)
{
  ODrive.write(&byte, 1U);  
}

uint32_t min_time_ms(void)
{
  return millis();
}

void min_application_handler(uint8_t min_id, uint8_t *min_payload, uint8_t len_payload, uint8_t port)
{
  Serial.print("MIN frame with ID ");
  Serial.print(min_id);
  Serial.print(" received at ");
  Serial.println(millis());
  min_id++;
  // The frame echoed back doesn't go through the transport protocol: it's send back directly
  min_send_frame(&min_ctx, min_id, min_payload, len_payload);
}

void setup()
{
  min_init_context(&min_ctx, 0);
  last_sent = millis();
}

void loop()
{
  char buf[32];
  size_t buf_len;

  if(SerialUSB.available() > 0) {
    buf_len = SerialUSB.readBytes(buf, 32U);
  }
  else {
    buf_len = 0;
  }

  min_poll(&min_ctx, (uint8_t *)buf, (uint8_t)buf_len);

  if(!min_queue_frame(&min_ctx, 0x33U, (uint8_t *)&now, 4U)) {
    // The queue has overflowed for some reason
    Serial.print("Can't queue at time ");
    Serial.println(millis());
  }
  last_sent = now;
}
