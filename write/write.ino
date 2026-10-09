#include <SPI.h>
#include <SPIMemory.h>

#define FLASH_CS   10
#define PAGE_SIZE  64


SPIFlash flash(FLASH_CS);

const uint32_t FLASH_SIZE = 4194304UL;

uint8_t pageBuf[PAGE_SIZE];
uint8_t verifyBuf[PAGE_SIZE];

void setup() {
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  Serial.begin(115200);       // ATmega8 @ 8 MHz or 16 MHz — 115200 is safe
  if (!flash.begin(FLASH_SIZE)) {
    Serial.println(F("INIT_FAIL"));
    delay(2000);
    setup();
    while (1);
  }
  Serial.println(F("BOOT"));
  
}

void loop() {
  if (!Serial.available()) return;

  char c = Serial.read();

  if (c == 'E') {
    Serial.println(F("ERASING"));
    if (!flash.eraseChip()) {
      Serial.println(F("EFAIL"));
      return;
    }
    Serial.println(F("R"));
  }
  else if (c == 'P') {
    uint8_t hdr[2];
    if (!readExact(hdr, 2)) { Serial.println(F("XADDR")); return; }
    uint16_t idx = (uint16_t)hdr[0] | ((uint16_t)hdr[1] << 8);
    uint32_t addr = (uint32_t)idx * PAGE_SIZE;

    if (!readExact(pageBuf, PAGE_SIZE)) { Serial.println(F("XDATA")); return; }

    if (!flash.writeByteArray(addr, pageBuf, PAGE_SIZE)) {
      Serial.println(F("XWRITE"));
      return;
    }
    flash.readByteArray(addr, verifyBuf, PAGE_SIZE);
    for (uint8_t i = 0; i < PAGE_SIZE; i++) {
      if (verifyBuf[i] != pageBuf[i]) { Serial.println(F("XVERIFY")); return; }
    }
    Serial.println(F("K"));
  }
  else if (c == 'D') {
    Serial.println(F("F"));
  }
  else {
    Serial.println(F("XCMD"));
  }
}

bool readExact(uint8_t *buf, uint8_t n) {
  uint8_t got = 0;
  uint32_t last = millis();
  while (got < n) {
    if (Serial.available()) {
      buf[got++] = (uint8_t)Serial.read();
      last = millis();
    } else if (millis() - last > 2000UL) {
      return false;
    }
  }
  return true;
}
