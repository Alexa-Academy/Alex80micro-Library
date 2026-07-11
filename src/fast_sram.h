#ifndef ALEX80_FAST_SRAM_H
#define ALEX80_FAST_SRAM_H

#include "fast_spi_bus.h"

class FastSram {
public:
  bool begin(
      uint8_t chipSelectPin,
      FastSpiBus &bus,
      uint32_t clockHz);

  uint8_t read8(uint16_t address);
  void write8(uint16_t address, uint8_t value);

private:
  static constexpr uint8_t COMMAND_READ = 0x03;
  static constexpr uint8_t COMMAND_WRITE = 0x02;
  static constexpr uint8_t COMMAND_WRITE_MODE = 0x01;

  uint8_t chipSelectPin_ = 8;
  uint32_t clockHz_ = 8000000UL;
  FastSpiBus *bus_ = nullptr;
};

#endif
