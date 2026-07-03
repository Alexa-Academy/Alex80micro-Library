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
    // Disattiva tutti i dispositivi dello shield prima di abilitare il bus.
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
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
      // La libreria del core calcola correttamente i divisori per R3 e R4.
      // Dopo endTransaction la configurazione hardware rimane attiva.
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
#if defined(ARDUINO_MINIMA)
    R_PORT1->PORR = 1U << 12;
#else
    R_PORT1->PORR = 1U << 3;
#endif
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
#if defined(ARDUINO_MINIMA)
    R_PORT1->POSR = 1U << 12;
#else
    R_PORT1->POSR = 1U << 3;
#endif
    return;
  }
#endif
  digitalWrite(chipSelectPin, HIGH);
}

void FastSpiBus::invalidate() {
  configuredClockHz_ = 0;
}
