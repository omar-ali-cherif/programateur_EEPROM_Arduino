#include <SPI.h>
#include <SPIMemory.h>

#define FLASH_CS 10
SPIFlash flash(FLASH_CS);

const uint32_t FLASH_SIZE = 4194304UL;  // 4 MB
const uint16_t CHUNK      = 256;

uint8_t buf[CHUNK];

void setup() {
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  Serial.begin(115200);
  delay(200);

  if (!flash.begin()) {
    Serial.println("Init failed!");
    while (1);
  }

  // Handshake: wait for Python to send a byte before starting
  Serial.println("READY");
  while (!Serial.available()) { /* wait */ }
  Serial.read();   // consume the go signal

  for (uint32_t addr = 0; addr < FLASH_SIZE; addr += CHUNK) {
    flash.readByteArray(addr, buf, CHUNK);
    Serial.write(buf, CHUNK);   // raw bytes out
  }
  
}

void loop() {}
