#ifndef ALEX80_FAST_MCP23S17_H
#define ALEX80_FAST_MCP23S17_H

#include "fast_spi_bus.h"

class FastMcp23s17 {
public:
  void begin(
      uint8_t chipSelectPin,
      FastSpiBus &bus,
      uint32_t clockHz);

  void pinMode(uint8_t pin, uint8_t mode);
  void digitalWrite(uint8_t pin, uint8_t value);
  uint8_t digitalRead(uint8_t pin);

  uint8_t readGPIO(uint8_t port);
  uint16_t readGPIO16();
  static void readGPIO16Pair(
      FastMcp23s17 &first,
      FastMcp23s17 &second,
      uint16_t &firstValue,
      uint16_t &secondValue);

  void writeGPIO(uint8_t value, uint8_t port);
  void writeGPIO16(uint16_t value);
  void writeGPIOBits(uint8_t port, uint8_t mask, uint8_t value);

  void writeRegister(uint8_t address, uint8_t value);
  uint8_t readRegister(uint8_t address);

private:
  void writeRegisters(uint8_t address, uint8_t first, uint8_t second);
  uint16_t readRegisters(uint8_t address);
  uint16_t readRegistersSelected(uint8_t address);

  uint8_t chipSelectPin_ = 10;
  uint32_t clockHz_ = 8000000UL;
  FastSpiBus *bus_ = nullptr;
  uint8_t direction_[2] = {0xff, 0xff};
  uint8_t pullup_[2] = {0x00, 0x00};
  uint8_t output_[2] = {0x00, 0x00};
};

#endif
