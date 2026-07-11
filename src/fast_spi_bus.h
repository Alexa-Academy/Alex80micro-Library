#ifndef ALEX80_FAST_SPI_BUS_H
#define ALEX80_FAST_SPI_BUS_H

#include <Arduino.h>
#include <SPI.h>

class FastSpiBus {
public:
  void begin(SPIClass &spi = SPI);
  void beginTransfer(uint32_t clockHz);
  void endTransfer();

  uint8_t transfer(uint8_t value);
  void select(uint8_t chipSelectPin);
  void deselect(uint8_t chipSelectPin);
  void invalidate();

private:
  SPIClass *spi_ = nullptr;
  uint32_t configuredClockHz_ = 0;
  bool initialized_ = false;
  bool transactionOpen_ = false;
};

#endif
