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
  bus_->transfer(0x00);  // Byte mode.
  bus_->deselect(chipSelectPin_);
  bus_->endTransfer();
  return true;
}

uint8_t FastSram::read8(uint16_t address) {
  bus_->beginTransfer(clockHz_);
  bus_->select(chipSelectPin_);
  bus_->transfer(COMMAND_READ);
  bus_->transfer(0x00);
  bus_->transfer(static_cast<uint8_t>(address >> 8));
  bus_->transfer(static_cast<uint8_t>(address));
  const uint8_t value = bus_->transfer(0x00);
  bus_->deselect(chipSelectPin_);
  bus_->endTransfer();
  return value;
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
}
