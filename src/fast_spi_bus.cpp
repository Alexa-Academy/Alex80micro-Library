#include "fast_spi_bus.h"

namespace {
inline bool usesDirectSpi(SPIClass *spi) {
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_RENESAS_UNO)
  return spi == &SPI;
#else
  (void) spi;
  return false;
#endif
}
}  // namespace

void FastSpiBus::begin(SPIClass &spi) {
  spi_ = &spi;
  if (!initialized_) {
    // Preload HIGH before making CS pins outputs to avoid a LOW pulse.
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    // Deselect all shield devices before enabling the bus.
    deselect(8);
    deselect(9);
    deselect(10);
    spi_->begin();
    initialized_ = true;
  }
  invalidate();
}

void FastSpiBus::beginTransfer(uint32_t clockHz) {
  if (!initialized_ || spi_ == nullptr) {
    begin(SPI);
  }

  if (usesDirectSpi(spi_)) {
    if (configuredClockHz_ != clockHz) {
      // The core library calculates the correct divisors for R3 and R4.
      // The hardware configuration remains active after endTransaction().
      spi_->beginTransaction(SPISettings(clockHz, MSBFIRST, SPI_MODE0));
      spi_->endTransaction();
      configuredClockHz_ = clockHz;
    }
    transactionOpen_ = false;
  } else {
    spi_->beginTransaction(SPISettings(clockHz, MSBFIRST, SPI_MODE0));
    transactionOpen_ = true;
  }
}

void FastSpiBus::endTransfer() {
  if (transactionOpen_) {
    spi_->endTransaction();
    transactionOpen_ = false;
  }
}

uint8_t FastSpiBus::transfer(uint8_t value) {
#if defined(ARDUINO_ARCH_AVR)
  if (usesDirectSpi(spi_)) {
    SPDR = value;
    while ((SPSR & _BV(SPIF)) == 0) {
    }
    return SPDR;
  }
#elif defined(ARDUINO_ARCH_RENESAS_UNO)
  if (usesDirectSpi(spi_)) {
    R_SPI0->SPDR_BY = value;
    while (R_SPI0->SPSR_b.SPRF == 0) {
      __NOP();
    }
    return R_SPI0->SPDR_BY;
  }
#endif
  return spi_->transfer(value);
}

void FastSpiBus::select(uint8_t chipSelectPin) {
#if defined(ARDUINO_ARCH_AVR)
  if (chipSelectPin >= 8 && chipSelectPin <= 10) {
    PORTB &= static_cast<uint8_t>(~_BV(chipSelectPin - 8));
    return;
  }
#elif defined(ARDUINO_ARCH_RENESAS_UNO)
  if (chipSelectPin == 8) {
    R_PORT3->PORR = 1U << 4;
    return;
  }
  if (chipSelectPin == 9) {
    R_PORT3->PORR = 1U << 3;
    return;
  }
  if (chipSelectPin == 10) {
    R_PORT1->PORR = 1U << 3;
    return;
  }
#endif
  digitalWrite(chipSelectPin, LOW);
}

void FastSpiBus::deselect(uint8_t chipSelectPin) {
#if defined(ARDUINO_ARCH_AVR)
  if (chipSelectPin >= 8 && chipSelectPin <= 10) {
    PORTB |= _BV(chipSelectPin - 8);
    return;
  }
#elif defined(ARDUINO_ARCH_RENESAS_UNO)
  if (chipSelectPin == 8) {
    R_PORT3->POSR = 1U << 4;
    return;
  }
  if (chipSelectPin == 9) {
    R_PORT3->POSR = 1U << 3;
    return;
  }
  if (chipSelectPin == 10) {
    R_PORT1->POSR = 1U << 3;
    return;
  }
#endif
  digitalWrite(chipSelectPin, HIGH);
}

void FastSpiBus::invalidate() {
  configuredClockHz_ = 0;
}
