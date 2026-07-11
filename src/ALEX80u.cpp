#include "ALEX80u.h"


// PRIVATE -----------------------------------------

void ALEX80u::mcp1_pinMode(int pin, byte mode) {
    mcp1.pinMode(pin, mode);
}

void ALEX80u::mcp2_pinMode(int pin, byte mode) {
    mcp2.pinMode(pin, mode);
}

void ALEX80u::mcp2_digitalWrite(int pin, byte value) {
    mcp2.digitalWrite(pin, value);
}


// PUBLIC -----------------------------------------

ALEX80u::ALEX80u(unsigned long ram_speed, unsigned long mcp_speed) : rs(ram_speed), ms(mcp_speed) {}

void ALEX80u::begin_UNO() {
    // Set the data bus as input.
    pinMode(3, INPUT);   // D0
    pinMode(4, INPUT);   // D1
    pinMode(5, INPUT);   // D2
    pinMode(6, INPUT);   // D3
    pinMode(7, INPUT);   // D4
    pinMode(A3, INPUT);  // D5
    pinMode(A4, INPUT);  // D6
    pinMode(A5, INPUT);  // D7
    // Initialize the outputs.
    pinMode(2, OUTPUT);   // CLK
    pinMode(A0, OUTPUT);  // INT
    pinMode(A1, OUTPUT);  // NMI
    pinMode(A2, OUTPUT);  // WAIT
    // Set the output states.
    digitalWrite(2, LOW);    // CLK
    digitalWrite(A0, HIGH);  // INT
    digitalWrite(A1, HIGH);  // NMI
    digitalWrite(A2, HIGH);  // WAIT
}

void ALEX80u::begin_RAM() {
  spiBus.begin(SPI);
  delay(1);
  sram.begin(8, spiBus, rs);
}

void ALEX80u::begin_MCP() {
    spiBus.begin(SPI);
    mcp1.begin(10, spiBus, ms);
    mcp2.begin(9, spiBus, ms);

    mcp2_digitalWrite(0, HIGH);   // RST
    mcp2_digitalWrite(10, HIGH);  // BUSRQ
  
    // Initialize MCP1 pins.
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
    // Initialize MCP2 pins.
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
    uint16_t mcp1Value;
    uint16_t mcp2Value;
    FastMcp23s17::readGPIO16Pair(mcp1, mcp2, mcp1Value, mcp2Value);
    return (mcp1Value & 0x007fU) | ((mcp1Value >> 1) & 0x3f80U) | ((mcp2Value << 6) & 0xc000U);
}

uint8_t ALEX80u::read_CMD() {
    const uint16_t value = mcp2.readGPIO16();
    return static_cast<uint8_t>((value >> 1) & 0x3fU) | static_cast<uint8_t>((value >> 5) & 0xc0U);
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
    return sram.read8(ind);
}

void ALEX80u::write_RAM(uint16_t ind, uint8_t val) {
    sram.write8(ind, val);
}
