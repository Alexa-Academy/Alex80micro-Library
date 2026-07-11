#ifndef ALEX80U_H
#define ALEX80U_H

#include <Arduino.h>
#include <SPI.h>
#include "fast_mcp23s17.h"
#include "fast_spi_bus.h"
#include "fast_sram.h"

class ALEX80u {

private:
  unsigned long rs;  // SRAM SPI speed
  unsigned long ms;  // MCP SPI speed
  FastSpiBus spiBus;
  FastMcp23s17 mcp1;
  FastMcp23s17 mcp2;
  FastSram sram;

  void mcp1_pinMode(int pin, byte mode);
  void mcp2_pinMode(int pin, byte mode);
  void mcp2_digitalWrite(int pin, byte value);


public:
  ALEX80u(unsigned long ram_speed, unsigned long mcp_speed);

  void begin_UNO();  // Configures Arduino Uno inputs and outputs
  void begin_RAM();  // Initializes the SRAM
  void begin_MCP();  // Configures inputs and outputs of both MCPs

  void set_CLK(byte mode);    // HIGH | LOW - Sets the CLK pin state
  void set_INT(byte mode);    // HIGH | LOW - Sets the INT pin state
  void set_NMI(byte mode);    // HIGH | LOW - Sets the NMI pin state
  void set_WAIT(byte mode);   // HIGH | LOW - Sets the WAIT pin state
  void set_RST(byte mode);    // HIGH | LOW - Sets the RST pin state
  void set_BUSRQ(byte mode);  // HIGH | LOW - Sets the BUSRQ pin state

  uint16_t read_ADDR();  // Reads the address bus
  uint8_t read_CMD();    // b0: HALT | b1: RFSH | b2: M1 | b3: IORQ | b4: MREQ | b5: WR | b6: BUSAK | b7: RD

  void pinMode_DATA(byte mode);  // INPUT | OUTPUT - Sets the data-bus direction
  uint8_t read_DATA();           // Reads the data bus
  void write_DATA(uint8_t val);  // Writes the data bus

  uint8_t read_RAM(uint16_t ind);             // Reads SRAM
  void write_RAM(uint16_t ind, uint8_t val);  // Writes SRAM
};

#endif
