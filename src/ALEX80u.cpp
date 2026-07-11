#include "ALEX80u.h"


// PRIVATE -----------------------------------------

void ALEX80u::mcp1_pinMode(int pin, byte mode) {
    uint8_t reg = (pin < 8) ? 0x00 : 0x01;
    uint8_t bit = pin & 0x07;
    uint8_t iodir = this->mcpRead(10, reg);
    if (mode == OUTPUT) iodir &= ~(1 << bit);
    else if (mode == INPUT) iodir |= (1 << bit);

    mcpWrite(10, reg, iodir);
}

void ALEX80u::mcp2_pinMode(int pin, byte mode) {
    uint8_t reg = (pin < 8) ? 0x00 : 0x01;
    uint8_t bit = pin & 0x07;
    uint8_t iodir = this->mcpRead(9, reg);
    if (mode == OUTPUT) iodir &= ~(1 << bit);
    else if (mode == INPUT) iodir |= (1 << bit);

    mcpWrite(9, reg, iodir);
}

void ALEX80u::mcp2_digitalWrite(int pin, byte value) {
    uint8_t reg = (pin < 8) ? 0x14 : 0x15;  // OLATA / OLATB
    uint8_t bit = pin & 0x07;
    uint8_t olat = mcpRead(9, reg);
    if (value == HIGH) olat |= (1 << bit);
    else if (value == LOW) olat &= ~(1 << bit);

    mcpWrite(9, reg, olat);
}

void ALEX80u::mcpWrite(uint8_t cs, uint8_t reg, uint8_t data) {
  SPI.beginTransaction(SPISettings(this->ms, MSBFIRST, SPI_MODE0));
  digitalWrite(cs, LOW);
  SPI.transfer(0x40);
  SPI.transfer(reg);
  SPI.transfer(data);
  digitalWrite(cs, HIGH);
  SPI.endTransaction();
}

uint8_t ALEX80u::mcpRead(uint8_t cs, uint8_t reg) {
  byte value;
  SPI.beginTransaction(SPISettings(this->ms, MSBFIRST, SPI_MODE0));
  digitalWrite(cs, LOW);
  SPI.transfer(0x41);
  SPI.transfer(reg);
  value = SPI.transfer(0x00);
  digitalWrite(cs, HIGH);
  SPI.endTransaction();
  return value;
}


// PUBLIC -----------------------------------------

ALEX80u::ALEX80u(unsigned long ram_speed, unsigned long mcp_speed) : rs(ram_speed), ms(mcp_speed) {}

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
  digitalWrite(2, LOW);    // CLK
  digitalWrite(A0, HIGH);  // INT
  digitalWrite(A1, HIGH);  // NMI
  digitalWrite(A2, HIGH);  // WAIT
}

void ALEX80u::begin_RAM() {
  SPI.begin();
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  delay(1);
  SPI.beginTransaction(SPISettings(this->rs, MSBFIRST, SPI_MODE0));
  digitalWrite(8, LOW);
  SPI.transfer(0x01);  // WRITE MODE REGISTER
  SPI.transfer(0x00);  // MODE BYTE
  digitalWrite(8, HIGH);
  SPI.endTransaction();
}

void ALEX80u::begin_MCP() {
  // Imposto i CS
  SPI.begin();

  // Better to set the latch high before setting pin as outputs
  digitalWrite(9, HIGH);
  digitalWrite(10, HIGH);
  pinMode(9, OUTPUT);   // CS MCP2
  pinMode(10, OUTPUT);  // CS MCP1

  mcp2_digitalWrite(0, HIGH);   // RST
  mcp2_digitalWrite(10, HIGH);  // BUSRQ

  // Inizializzo i pin di MCP1
  mcp1_pinMode(0, INPUT);  // A0
  mcp1_pinMode(1, INPUT);  // A1
  mcp1_pinMode(2, INPUT);  // A2
  mcp1_pinMode(3, INPUT);  // A3
  mcp1_pinMode(4, INPUT);  // A4
  mcp1_pinMode(5, INPUT);  // A5
  mcp1_pinMode(6, INPUT);  // A6
  mcp1_pinMode(8, INPUT);   // A7
  mcp1_pinMode(9, INPUT);   // A8
  mcp1_pinMode(10, INPUT);  // A9
  mcp1_pinMode(11, INPUT);  // A10
  mcp1_pinMode(12, INPUT);  // A11
  mcp1_pinMode(13, INPUT);  // A12
  mcp1_pinMode(14, INPUT);  // A13

  // Inizializzo i pin di MCP2
  mcp2_pinMode(0, OUTPUT);  // RST
  mcp2_pinMode(1, INPUT);   // HALT
  mcp2_pinMode(2, INPUT);   // RFSH
  mcp2_pinMode(3, INPUT);   // M1
  mcp2_pinMode(4, INPUT);   // IORQ
  mcp2_pinMode(5, INPUT);   // MREQ
  mcp2_pinMode(6, INPUT);   // WR
  mcp2_pinMode(8, INPUT);    // A14
  mcp2_pinMode(9, INPUT);    // A15
  mcp2_pinMode(10, OUTPUT);  // BUSRQ
  mcp2_pinMode(11, INPUT);   // BUSACK
  mcp2_pinMode(12, INPUT);   // RD
}

void ALEX80u::set_CLK(byte mode) {
  digitalWrite(2, mode);
}

void ALEX80u::set_INT(byte mode) {
  digitalWrite(A0, mode);
}

void ALEX80u::set_NMI(byte mode) {
  digitalWrite(A1, mode);
}

void ALEX80u::set_WAIT(byte mode) {
  digitalWrite(A2, mode);
}

void ALEX80u::set_RST(byte mode) {
  mcp2_digitalWrite(0, mode);
}

void ALEX80u::set_BUSRQ(byte mode) {
  mcp2_digitalWrite(10, mode);
}

uint16_t ALEX80u::read_ADDR() {
  uint16_t ret;
  uint8_t addr_a = mcpRead(10, 0x12);  // MCP1 port A
  uint8_t addr_b = mcpRead(10, 0x13);  // MCP1 port B
  uint8_t addr_c = mcpRead(9, 0x13);   // MCP2 port B
  bitWrite(ret, 0, bitRead(addr_a, 0));
  bitWrite(ret, 1, bitRead(addr_a, 1));
  bitWrite(ret, 2, bitRead(addr_a, 2));
  bitWrite(ret, 3, bitRead(addr_a, 3));
  bitWrite(ret, 4, bitRead(addr_a, 4));
  bitWrite(ret, 5, bitRead(addr_a, 5));
  bitWrite(ret, 6, bitRead(addr_a, 6));
  bitWrite(ret, 7, bitRead(addr_b, 0));
  bitWrite(ret, 8, bitRead(addr_b, 1));
  bitWrite(ret, 9, bitRead(addr_b, 2));
  bitWrite(ret, 10, bitRead(addr_b, 3));
  bitWrite(ret, 11, bitRead(addr_b, 4));
  bitWrite(ret, 12, bitRead(addr_b, 5));
  bitWrite(ret, 13, bitRead(addr_b, 6));
  bitWrite(ret, 14, bitRead(addr_c, 0));
  bitWrite(ret, 15, bitRead(addr_c, 1));
  return ret;
}

uint8_t ALEX80u::read_CMD() {
  uint8_t ret;
  uint8_t cmd_a = mcpRead(9, 0x12);  // MCP2 port A
  uint8_t cmd_b = mcpRead(9, 0x13);  // MCP2 port B
  bitWrite(ret, 0, bitRead(cmd_a, 1));     // HALT
  bitWrite(ret, 1, bitRead(cmd_a, 2));     // RFSH
  bitWrite(ret, 2, bitRead(cmd_a, 3));     // M1
  bitWrite(ret, 3, bitRead(cmd_a, 4));     // IORQ
  bitWrite(ret, 4, bitRead(cmd_a, 5));     // MREQ
  bitWrite(ret, 5, bitRead(cmd_a, 6));     // WR
  bitWrite(ret, 6, bitRead(cmd_b, 3));     // BUSACK
  bitWrite(ret, 7, bitRead(cmd_b, 4));     // RD
  return ret;
}

void ALEX80u::pinMode_DATA(byte mode) {
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
}

uint8_t ALEX80u::read_DATA() {
  uint8_t ret;
  bitWrite(ret, 0, digitalRead(3));
  bitWrite(ret, 1, digitalRead(4));
  bitWrite(ret, 2, digitalRead(5));
  bitWrite(ret, 3, digitalRead(6));
  bitWrite(ret, 4, digitalRead(7));
  bitWrite(ret, 5, digitalRead(A3));
  bitWrite(ret, 6, digitalRead(A4));
  bitWrite(ret, 7, digitalRead(A5));
  return ret;
}

void ALEX80u::write_DATA(uint8_t val) {
  digitalWrite(3, bitRead(val, 0));
  digitalWrite(4, bitRead(val, 1));
  digitalWrite(5, bitRead(val, 2));
  digitalWrite(6, bitRead(val, 3));
  digitalWrite(7, bitRead(val, 4));
  digitalWrite(A3, bitRead(val, 5));
  digitalWrite(A4, bitRead(val, 6));
  digitalWrite(A5, bitRead(val, 7));
}

uint8_t ALEX80u::read_RAM(uint16_t ind) {
  SPI.beginTransaction(SPISettings(this->rs, MSBFIRST, SPI_MODE0));
  digitalWrite(8, LOW);  // CS RAM
  SPI.transfer(0x03);    // READ
  SPI.transfer(0x00);
  SPI.transfer16(ind);
  uint8_t ret = SPI.transfer(0x00);
  digitalWrite(8, HIGH);
  SPI.endTransaction();
  return ret;
}

void ALEX80u::write_RAM(uint16_t ind, uint8_t val) {
  SPI.beginTransaction(SPISettings(this->rs, MSBFIRST, SPI_MODE0));
  digitalWrite(8, LOW);  // CS RAM
  SPI.transfer(0x02);    // WRITE
  SPI.transfer(0x00);
  SPI.transfer16(ind);
  SPI.transfer(val);
  digitalWrite(8, HIGH);
  SPI.endTransaction();
}