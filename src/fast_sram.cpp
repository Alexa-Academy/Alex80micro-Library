#include "fast_sram.h"

bool FastSram::begin(
    uint8_t chipSelectPin,
    FastSpiBus &bus,
    uint32_t clockHz) {
  chipSelectPin_ = chipSelectPin;
  clockHz_ = clockHz;
  bus_ = &bus;
  bus_->begin();
  bus_->deselect(chipSelectPin_);

  bus_->beginTransfer(clockHz_);
  bus_->select(chipSelectPin_);
  bus_->transfer(COMMAND_WRITE_MODE);
  bus_->transfer(MODE_SEQUENTIAL);
  bus_->deselect(chipSelectPin_);
  bus_->endTransfer();

  for (uint8_t line = 0; line < CACHE_LINE_COUNT; ++line) {
    cacheValid_[line] = false;
  }
  return true;
}

uint8_t FastSram::read8(uint16_t address) {
  const uint16_t lineBase =
      address & static_cast<uint16_t>(~(CACHE_LINE_SIZE - 1U));
  const uint8_t line =
      static_cast<uint8_t>((address / CACHE_LINE_SIZE) &
                           (CACHE_LINE_COUNT - 1U));

  if (!cacheValid_[line] || cacheTags_[line] != lineBase) {
    readBlock(
        lineBase,
        &cache_[static_cast<uint16_t>(line) * CACHE_LINE_SIZE],
        CACHE_LINE_SIZE);
    cacheTags_[line] = lineBase;
    cacheValid_[line] = true;
  }

  return cache_[static_cast<uint16_t>(line) * CACHE_LINE_SIZE +
                (address - lineBase)];
}

void FastSram::readBlock(
    uint16_t address,
    uint8_t *destination,
    uint8_t length) {
  bus_->beginTransfer(clockHz_);
  bus_->select(chipSelectPin_);
  bus_->transfer(COMMAND_READ);
  bus_->transfer(0x00);
  bus_->transfer(static_cast<uint8_t>(address >> 8));
  bus_->transfer(static_cast<uint8_t>(address));
  for (uint8_t i = 0; i < length; ++i) {
    destination[i] = bus_->transfer(0x00);
  }
  bus_->deselect(chipSelectPin_);
  bus_->endTransfer();
}

void FastSram::write8(uint16_t address, uint8_t value) {
  bus_->beginTransfer(clockHz_);
  bus_->select(chipSelectPin_);
  bus_->transfer(COMMAND_WRITE);
  bus_->transfer(0x00);
  bus_->transfer(static_cast<uint8_t>(address >> 8));
  bus_->transfer(static_cast<uint8_t>(address));
  bus_->transfer(value);
  bus_->deselect(chipSelectPin_);
  bus_->endTransfer();

  const uint16_t lineBase =
      address & static_cast<uint16_t>(~(CACHE_LINE_SIZE - 1U));
  const uint8_t line =
      static_cast<uint8_t>((address / CACHE_LINE_SIZE) &
                           (CACHE_LINE_COUNT - 1U));
  if (cacheValid_[line] && cacheTags_[line] == lineBase) {
    cache_[static_cast<uint16_t>(line) * CACHE_LINE_SIZE +
           (address - lineBase)] = value;
  }
}
