#ifndef ALEX80U_H
#define ALEX80U_H

#include <Arduino.h>
#include <SPI.h>
#include "fast_mcp23s17.h"
#include "fast_spi_bus.h"
#include "fast_sram.h"

class ALEX80u {

private:
  unsigned long rs;  // Ram Spi Speed
  unsigned long ms;  // Mcp Spi Speed
  FastSpiBus spiBus;
  FastMcp23s17 mcp1;
  FastMcp23s17 mcp2;
  FastSram sram;

  void mcp1_pinMode(int pin, byte mode);
  void mcp2_pinMode(int pin, byte mode);
  void mcp2_digitalWrite(int pin, byte value);


public:
  ALEX80u(unsigned long ram_speed, unsigned long mcp_speed);

  void begin_UNO();  // Imposta Input ed Output di Arduino UNO
  void begin_RAM();  // Inizializza la Ram
  void begin_MCP();  // Imposta Input ed Output dei due Mcp

  void set_CLK(byte mode);    // HIGH | LOW - Imposta lo stato del pin CLK
  void set_INT(byte mode);    // HIGH | LOW - Imposta lo stato del pin INT
  void set_NMI(byte mode);    // HIGH | LOW - Imposta lo stato del pin NMI
  void set_WAIT(byte mode);   // HIGH | LOW - Imposta lo stato del pin WAIT
  void set_RST(byte mode);    // HIGH | LOW - Imposta lo stato del pin RST
  void set_BUSRQ(byte mode);  // HIGH | LOW - Imposta lo stato del pin BUSRQ

  uint16_t read_ADDR();  // Lettura AddressBus
  uint8_t read_CMD();    // b0: HALT | b1: RFSH | b2: M1 | b3: IORQ | b4: MREQ | b5: WR | b6: BUSAK | b7: RD

  void pinMode_DATA(byte mode);  // INPUT | OUTPUT - Direzione DataBus
  uint8_t read_DATA();           // Lettura DataBus
  void write_DATA(uint8_t val);  // Scrittura DataBus

  uint8_t read_RAM(uint16_t ind);             // Lettura Ram
  void write_RAM(uint16_t ind, uint8_t val);  // Scrittura Ram
};

#endif
