#include "fast_mcp23s17.h"

namespace {
constexpr uint8_t OPCODE_WRITE = 0x40;
constexpr uint8_t OPCODE_READ = 0x41;
constexpr uint8_t REG_IODIRA = 0x00;
constexpr uint8_t REG_GPPUA = 0x0c;
constexpr uint8_t REG_GPIOA = 0x12;
constexpr uint8_t REG_OLATA = 0x14;
}  // namespace

void FastMcp23s17::begin(uint8_t chipSelectPin, FastSpiBus &bus, uint32_t clockHz) {
    chipSelectPin_ = chipSelectPin;
    clockHz_ = clockHz;
    bus_ = &bus;
    bus_->begin();
    bus_->deselect(chipSelectPin_);

    // The MCP23S17 reset values already match direction_, pullup_ and output_.
}

void FastMcp23s17::pinMode(uint8_t pin, uint8_t mode) {
  if (pin >= 16) {
    return;
  }
  const uint8_t port = pin >> 3;
  const uint8_t mask = 1U << (pin & 0x07U);
  const uint8_t previousDirection = direction_[port];
  const uint8_t previousPullup = pullup_[port];

  if (mode == OUTPUT) {
    direction_[port] &= static_cast<uint8_t>(~mask);
    pullup_[port] &= static_cast<uint8_t>(~mask);
  } else {
    direction_[port] |= mask;
    if (mode == INPUT_PULLUP) {
      pullup_[port] |= mask;
    } else {
      pullup_[port] &= static_cast<uint8_t>(~mask);
    }
  }

  if (direction_[port] != previousDirection) {
    writeRegister(REG_IODIRA + port, direction_[port]);
  }
  if (pullup_[port] != previousPullup) {
    writeRegister(REG_GPPUA + port, pullup_[port]);
  }
}

void FastMcp23s17::digitalWrite(uint8_t pin, uint8_t value) {
  if (pin >= 16) {
    return;
  }
  const uint8_t port = pin >> 3;
  const uint8_t mask = 1U << (pin & 0x07U);
  if (value == LOW) {
    output_[port] &= static_cast<uint8_t>(~mask);
  } else {
    output_[port] |= mask;
  }
  writeRegister(REG_OLATA + port, output_[port]);
}

uint8_t FastMcp23s17::digitalRead(uint8_t pin) {
  if (pin >= 16) {
    return LOW;
  }
  return (readGPIO(pin >> 3) >> (pin & 0x07U)) & 0x01U;
}

uint8_t FastMcp23s17::readGPIO(uint8_t port) {
  return port < 2 ? readRegister(REG_GPIOA + port) : 0;
}

uint16_t FastMcp23s17::readGPIO16() {
  return readRegisters(REG_GPIOA);
}

void FastMcp23s17::readGPIO16Pair(FastMcp23s17 &first, FastMcp23s17 &second, uint16_t &firstValue, uint16_t &secondValue) {
  FastSpiBus *bus = first.bus_;
  bus->beginTransfer(first.clockHz_);
  bus->select(first.chipSelectPin_);
  firstValue = first.readRegistersSelected(REG_GPIOA);
  bus->deselect(first.chipSelectPin_);

if (second.bus_ == bus && second.clockHz_ == first.clockHz_) {
    bus->select(second.chipSelectPin_);
    secondValue = second.readRegistersSelected(REG_GPIOA);
    bus->deselect(second.chipSelectPin_);
    bus->endTransfer();
  } else {
    bus->endTransfer();
    secondValue = second.readGPIO16();
  }
}

void FastMcp23s17::writeGPIO(uint8_t value, uint8_t port) {
  if (port >= 2) {
    return;
  }
  output_[port] = value;
  writeRegister(REG_OLATA + port, value);
}

void FastMcp23s17::writeGPIO16(uint16_t value) {
  output_[0] = static_cast<uint8_t>(value);
  output_[1] = static_cast<uint8_t>(value >> 8);
  writeRegisters(REG_OLATA, output_[0], output_[1]);
}

void FastMcp23s17::writeGPIOBits(
    uint8_t port,
    uint8_t mask,
    uint8_t value) {
  if (port >= 2) {
    return;
  }
  output_[port] =
      (output_[port] & static_cast<uint8_t>(~mask)) | (value & mask);
  writeRegister(REG_OLATA + port, output_[port]);
}

void FastMcp23s17::writeRegister(uint8_t address, uint8_t value) {
  bus_->beginTransfer(clockHz_);
  bus_->select(chipSelectPin_);
  bus_->transfer(OPCODE_WRITE);
  bus_->transfer(address);
  bus_->transfer(value);
  bus_->deselect(chipSelectPin_);
  bus_->endTransfer();
}

uint8_t FastMcp23s17::readRegister(uint8_t address) {
  bus_->beginTransfer(clockHz_);
  bus_->select(chipSelectPin_);
  bus_->transfer(OPCODE_READ);
  bus_->transfer(address);
  const uint8_t value = bus_->transfer(0);
  bus_->deselect(chipSelectPin_);
  bus_->endTransfer();
  return value;
}

void FastMcp23s17::writeRegisters(
    uint8_t address,
    uint8_t first,
    uint8_t second) {
  bus_->beginTransfer(clockHz_);
  bus_->select(chipSelectPin_);
  bus_->transfer(OPCODE_WRITE);
  bus_->transfer(address);
  bus_->transfer(first);
  bus_->transfer(second);
  bus_->deselect(chipSelectPin_);
  bus_->endTransfer();
}

uint16_t FastMcp23s17::readRegisters(uint8_t address) {
  bus_->beginTransfer(clockHz_);
  bus_->select(chipSelectPin_);
  const uint16_t value = readRegistersSelected(address);
  bus_->deselect(chipSelectPin_);
  bus_->endTransfer();
  return value;
}

uint16_t FastMcp23s17::readRegistersSelected(uint8_t address) {
  bus_->transfer(OPCODE_READ);
  bus_->transfer(address);
  const uint8_t first = bus_->transfer(0);
  const uint8_t second = bus_->transfer(0);
  return static_cast<uint16_t>(first) |
      (static_cast<uint16_t>(second) << 8);
}
