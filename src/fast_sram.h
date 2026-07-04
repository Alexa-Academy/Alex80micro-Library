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
#if defined(ARDUINO_MINIMA) || defined(ARDUINO_UNOWIFIR4)
  static constexpr uint8_t CACHE_LINE_SIZE = 32;
  static constexpr uint8_t CACHE_LINE_COUNT = 128;
#else
  static constexpr uint8_t CACHE_LINE_SIZE = 16;
  static constexpr uint8_t CACHE_LINE_COUNT = 4;
#endif
  static constexpr uint16_t CACHE_SIZE =
      CACHE_LINE_SIZE * CACHE_LINE_COUNT;

  static constexpr uint8_t COMMAND_READ = 0x03;
  static constexpr uint8_t COMMAND_WRITE = 0x02;
  static constexpr uint8_t COMMAND_WRITE_MODE = 0x01;
  static constexpr uint8_t MODE_SEQUENTIAL = 0x40;

  uint8_t chipSelectPin_ = 8;
  uint32_t clockHz_ = 8000000UL;
  FastSpiBus *bus_ = nullptr;
  uint8_t cache_[CACHE_SIZE];
  uint16_t cacheTags_[CACHE_LINE_COUNT];
  bool cacheValid_[CACHE_LINE_COUNT];

  void readBlock(uint16_t address, uint8_t *destination, uint8_t length);
};

#endif
