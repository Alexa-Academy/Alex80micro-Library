#include "ALEX80u.h"

namespace {

#if defined(ARDUINO_AVR_UNO)
constexpr uint8_t AVR_DATA_PORTD_MASK = 0xf8U;
constexpr uint8_t AVR_DATA_PORTC_MASK = 0x38U;
#elif defined(ARDUINO_MINIMA)
constexpr uint16_t R4_DATA_PORT1_MASK = 0x00dfU;
constexpr uint16_t R4_DATA_PORT0_MASK = 0x0004U;
#elif defined(ARDUINO_UNOWIFIR4)
constexpr uint16_t R4_DATA_PORT1_MASK = 0x18e3U;
constexpr uint16_t R4_DATA_PORT0_MASK = 0x0004U;
#endif

#if defined(ARDUINO_MINIMA) || defined(ARDUINO_UNOWIFIR4)
inline void writeR4Port(
    R_PORT0_Type *port,
    uint16_t mask,
    uint16_t value) {
  const uint16_t bits = value & mask;
  port->PCNTR3 =
      (static_cast<uint32_t>(mask & static_cast<uint16_t>(~bits)) << 16) |
      bits;
}
#endif

}  // namespace


// PRIVATE -----------------------------------------

void ALEX80u::mcp1_pinMode(int pin, byte mode) {
  this->mcp1.pinMode(pin, mode);
}

void ALEX80u::mcp2_pinMode(int pin, byte mode) {
  this->mcp2.pinMode(pin, mode);
}

void ALEX80u::mcp2_digitalWrite(int pin, byte value) {
  this->mcp2.digitalWrite(pin, value);
}


// PUBLIC -----------------------------------------

ALEX80u::ALEX80u(unsigned long ram_speed, unsigned long mcp_speed) {
  this->rs = ram_speed;
  this->ms = mcp_speed;
}

void ALEX80u::begin_UNO() {
  // Imposto il DataBus in Input
  pinMode(3, INPUT);   // D0
  pinMode(4, INPUT);   // D1
  pinMode(5, INPUT);   // D2
  pinMode(6, INPUT);   // D3
  pinMode(7, INPUT);   // D4
  pinMode(A3, INPUT);  // D5
  pinMode(A4, INPUT);  // D6
  pinMode(A5, INPUT);  // D7
  // Inizializzo le Uscite
  pinMode(2, OUTPUT);   // CLK
  pinMode(A0, OUTPUT);  // INT
  pinMode(A1, OUTPUT);  // NMI
  pinMode(A2, OUTPUT);  // WAIT
  // Imposto lo stato delle uscite
  set_CLK(LOW);
  set_INT(HIGH);
  set_NMI(HIGH);
  set_WAIT(HIGH);
}

void ALEX80u::begin_RAM() {
  this->spiBus.begin(SPI);
  delay(1);
  this->sram.begin(8, this->spiBus, this->rs);
}

void ALEX80u::begin_MCP() {
  this->spiBus.begin(SPI);
  delay(1);
  this->mcp1.begin(10, this->spiBus, this->ms);
  this->mcp2.begin(9, this->spiBus, this->ms);
  delay(1);
  // Inizializzo i pin di MCP1
  this->mcp1_pinMode(0, INPUT);  // A0
  this->mcp1_pinMode(1, INPUT);  // A1
  this->mcp1_pinMode(2, INPUT);  // A2
  this->mcp1_pinMode(3, INPUT);  // A3
  this->mcp1_pinMode(4, INPUT);  // A4
  this->mcp1_pinMode(5, INPUT);  // A5
  this->mcp1_pinMode(6, INPUT);  // A6
  this->mcp1_pinMode(7, OUTPUT);
  this->mcp1_pinMode(8, INPUT);   // A7
  this->mcp1_pinMode(9, INPUT);   // A8
  this->mcp1_pinMode(10, INPUT);  // A9
  this->mcp1_pinMode(11, INPUT);  // A10
  this->mcp1_pinMode(12, INPUT);  // A11
  this->mcp1_pinMode(13, INPUT);  // A12
  this->mcp1_pinMode(14, INPUT);  // A13
  this->mcp1_pinMode(15, OUTPUT);
  // Inizializzo i pin di MCP2
  this->mcp2_pinMode(0, OUTPUT);  // RST
  this->mcp2_pinMode(1, INPUT);   // HALT
  this->mcp2_pinMode(2, INPUT);   // RFSH
  this->mcp2_pinMode(3, INPUT);   // M1
  this->mcp2_pinMode(4, INPUT);   // IORQ
  this->mcp2_pinMode(5, INPUT);   // MREQ
  this->mcp2_pinMode(6, INPUT);   // WR
  this->mcp2_pinMode(7, OUTPUT);
  this->mcp2_pinMode(8, INPUT);    // A14
  this->mcp2_pinMode(9, INPUT);    // A15
  this->mcp2_pinMode(10, OUTPUT);  // BUSRQ
  this->mcp2_pinMode(11, INPUT);   // BUSACK
  this->mcp2_pinMode(12, INPUT);   // RD
  this->mcp2_pinMode(13, OUTPUT);
  this->mcp2_pinMode(14, OUTPUT);
  this->mcp2_pinMode(15, OUTPUT);
  // Imposto alte le uscite
  this->mcp2_digitalWrite(0, HIGH);   // RST
  this->mcp2_digitalWrite(10, HIGH);  // BUSRQ
}

void ALEX80u::set_CLK(byte mode) {
#if defined(ARDUINO_AVR_UNO)
  if (mode == LOW) {
    PORTD &= static_cast<uint8_t>(~_BV(2));
  } else {
    PORTD |= _BV(2);
  }
#elif defined(ARDUINO_MINIMA)
  if (mode == LOW) {
    R_PORT1->PORR = 1U << 5;
  } else {
    R_PORT1->POSR = 1U << 5;
  }
#elif defined(ARDUINO_UNOWIFIR4)
  if (mode == LOW) {
    R_PORT1->PORR = 1U << 4;
  } else {
    R_PORT1->POSR = 1U << 4;
  }
#else
  digitalWrite(2, mode);
#endif
}

void ALEX80u::set_INT(byte mode) {
#if defined(ARDUINO_AVR_UNO)
  if (mode == LOW) {
    PORTC &= static_cast<uint8_t>(~_BV(0));
  } else {
    PORTC |= _BV(0);
  }
#elif defined(ARDUINO_MINIMA) || defined(ARDUINO_UNOWIFIR4)
  if (mode == LOW) {
    R_PORT0->PORR = 1U << 14;
  } else {
    R_PORT0->POSR = 1U << 14;
  }
#else
  digitalWrite(A0, mode);
#endif
}

void ALEX80u::set_NMI(byte mode) {
#if defined(ARDUINO_AVR_UNO)
  if (mode == LOW) {
    PORTC &= static_cast<uint8_t>(~_BV(1));
  } else {
    PORTC |= _BV(1);
  }
#elif defined(ARDUINO_MINIMA) || defined(ARDUINO_UNOWIFIR4)
  if (mode == LOW) {
    R_PORT0->PORR = 1U << 0;
  } else {
    R_PORT0->POSR = 1U << 0;
  }
#else
  digitalWrite(A1, mode);
#endif
}

void ALEX80u::set_WAIT(byte mode) {
#if defined(ARDUINO_AVR_UNO)
  if (mode == LOW) {
    PORTC &= static_cast<uint8_t>(~_BV(2));
  } else {
    PORTC |= _BV(2);
  }
#elif defined(ARDUINO_MINIMA) || defined(ARDUINO_UNOWIFIR4)
  if (mode == LOW) {
    R_PORT0->PORR = 1U << 1;
  } else {
    R_PORT0->POSR = 1U << 1;
  }
#else
  digitalWrite(A2, mode);
#endif
}

void ALEX80u::set_RST(byte mode) {
  this->mcp2_digitalWrite(0, mode);
}

void ALEX80u::set_BUSRQ(byte mode) {
  this->mcp2_digitalWrite(10, mode);
}

uint16_t ALEX80u::read_ADDR() {
  uint16_t mcp1Value;
  uint16_t mcp2Value;
  FastMcp23s17::readGPIO16Pair(
      this->mcp1,
      this->mcp2,
      mcp1Value,
      mcp2Value);
  return
      (mcp1Value & 0x007fU) |
      ((mcp1Value >> 1) & 0x3f80U) |
      ((mcp2Value << 6) & 0xc000U);
}

uint8_t ALEX80u::read_CMD() {
  const uint16_t value = this->mcp2.readGPIO16();
  return
      static_cast<uint8_t>((value >> 1) & 0x3fU) |
      static_cast<uint8_t>((value >> 5) & 0xc0U);
}

void ALEX80u::pinMode_DATA(byte mode) {
#if defined(ARDUINO_AVR_UNO)
  if (mode == OUTPUT) {
    DDRD |= AVR_DATA_PORTD_MASK;
    DDRC |= AVR_DATA_PORTC_MASK;
  } else if (mode == INPUT) {
    DDRD &= static_cast<uint8_t>(~AVR_DATA_PORTD_MASK);
    DDRC &= static_cast<uint8_t>(~AVR_DATA_PORTC_MASK);
  }
#elif defined(ARDUINO_MINIMA) || defined(ARDUINO_UNOWIFIR4)
  if (mode == OUTPUT) {
    R_PORT1->PDR |= R4_DATA_PORT1_MASK;
    R_PORT0->PDR |= R4_DATA_PORT0_MASK;
  } else if (mode == INPUT) {
    R_PORT1->PDR &= static_cast<uint16_t>(~R4_DATA_PORT1_MASK);
    R_PORT0->PDR &= static_cast<uint16_t>(~R4_DATA_PORT0_MASK);
  }
#else
  if (mode == OUTPUT) {
    pinMode(3, OUTPUT);   // D0
    pinMode(4, OUTPUT);   // D1
    pinMode(5, OUTPUT);   // D2
    pinMode(6, OUTPUT);   // D3
    pinMode(7, OUTPUT);   // D4
    pinMode(A3, OUTPUT);  // D5
    pinMode(A4, OUTPUT);  // D6
    pinMode(A5, OUTPUT);  // D7
  } else if (mode == INPUT) {
    pinMode(3, INPUT);   // D0
    pinMode(4, INPUT);   // D1
    pinMode(5, INPUT);   // D2
    pinMode(6, INPUT);   // D3
    pinMode(7, INPUT);   // D4
    pinMode(A3, INPUT);  // D5
    pinMode(A4, INPUT);  // D6
    pinMode(A5, INPUT);  // D7
  }
#endif
}

uint8_t ALEX80u::read_DATA() {
#if defined(ARDUINO_AVR_UNO)
  return
      static_cast<uint8_t>((PIND >> 3) & 0x1fU) |
      static_cast<uint8_t>((PINC & 0x38U) << 2);
#elif defined(ARDUINO_MINIMA)
  const uint16_t port1 = R_PORT1->PIDR;
  const uint16_t port0 = R_PORT0->PIDR;
  return
      static_cast<uint8_t>((port1 >> 4) & 0x01U) |
      static_cast<uint8_t>((port1 >> 2) & 0x02U) |
      static_cast<uint8_t>(port1 & 0x04U) |
      static_cast<uint8_t>((port1 >> 3) & 0x18U) |
      static_cast<uint8_t>((port0 & 0x04U) << 3) |
      static_cast<uint8_t>((port1 & 0x02U) << 5) |
      static_cast<uint8_t>((port1 & 0x01U) << 7);
#elif defined(ARDUINO_UNOWIFIR4)
  const uint16_t port1 = R_PORT1->PIDR;
  const uint16_t port0 = R_PORT0->PIDR;
  return
      static_cast<uint8_t>((port1 >> 5) & 0x07U) |
      static_cast<uint8_t>((port1 >> 8) & 0x18U) |
      static_cast<uint8_t>((port0 & 0x04U) << 3) |
      static_cast<uint8_t>((port1 & 0x02U) << 5) |
      static_cast<uint8_t>((port1 & 0x01U) << 7);
#else
  uint8_t ret = 0;
  bitWrite(ret, 0, digitalRead(3));
  bitWrite(ret, 1, digitalRead(4));
  bitWrite(ret, 2, digitalRead(5));
  bitWrite(ret, 3, digitalRead(6));
  bitWrite(ret, 4, digitalRead(7));
  bitWrite(ret, 5, digitalRead(A3));
  bitWrite(ret, 6, digitalRead(A4));
  bitWrite(ret, 7, digitalRead(A5));
  return ret;
#endif
}

void ALEX80u::write_DATA(uint8_t val) {
#if defined(ARDUINO_AVR_UNO)
  const uint8_t savedSreg = SREG;
  noInterrupts();
  PORTD =
      (PORTD & static_cast<uint8_t>(~AVR_DATA_PORTD_MASK)) |
      static_cast<uint8_t>((val & 0x1fU) << 3);
  PORTC =
      (PORTC & static_cast<uint8_t>(~AVR_DATA_PORTC_MASK)) |
      static_cast<uint8_t>((val & 0xe0U) >> 2);
  SREG = savedSreg;
#elif defined(ARDUINO_MINIMA)
  const uint16_t port1 =
      static_cast<uint16_t>((val & 0x01U) << 4) |
      static_cast<uint16_t>((val & 0x02U) << 2) |
      static_cast<uint16_t>(val & 0x04U) |
      static_cast<uint16_t>((val & 0x18U) << 3) |
      static_cast<uint16_t>((val & 0x40U) >> 5) |
      static_cast<uint16_t>((val & 0x80U) >> 7);
  const uint16_t port0 = static_cast<uint16_t>((val & 0x20U) >> 3);
  writeR4Port(R_PORT1, R4_DATA_PORT1_MASK, port1);
  writeR4Port(R_PORT0, R4_DATA_PORT0_MASK, port0);
#elif defined(ARDUINO_UNOWIFIR4)
  const uint16_t port1 =
      static_cast<uint16_t>((val & 0x07U) << 5) |
      static_cast<uint16_t>((val & 0x18U) << 8) |
      static_cast<uint16_t>((val & 0x40U) >> 5) |
      static_cast<uint16_t>((val & 0x80U) >> 7);
  const uint16_t port0 = static_cast<uint16_t>((val & 0x20U) >> 3);
  writeR4Port(R_PORT1, R4_DATA_PORT1_MASK, port1);
  writeR4Port(R_PORT0, R4_DATA_PORT0_MASK, port0);
#else
  digitalWrite(3, bitRead(val, 0));
  digitalWrite(4, bitRead(val, 1));
  digitalWrite(5, bitRead(val, 2));
  digitalWrite(6, bitRead(val, 3));
  digitalWrite(7, bitRead(val, 4));
  digitalWrite(A3, bitRead(val, 5));
  digitalWrite(A4, bitRead(val, 6));
  digitalWrite(A5, bitRead(val, 7));
#endif
}

uint8_t ALEX80u::read_RAM(uint16_t ind) {
  return this->sram.read8(ind);
}

void ALEX80u::write_RAM(uint16_t ind, uint8_t val) {
  this->sram.write8(ind, val);
}