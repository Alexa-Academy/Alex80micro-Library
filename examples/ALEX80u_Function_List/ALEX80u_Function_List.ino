#include "ALEX80u.h"

// UNO R3
#define MCP_SPEED 8000000
#define RAM_SPEED 8000000
#define SER_SPEED 9600

// UNO R4
//#define RAM_SPEED 20000000
//#define MCP_SPEED 10000000
//#define SER_SPEED 115200

ALEX80u a80u(RAM_SPEED, MCP_SPEED);

uint16_t Addr = 0x0000;
uint8_t Data = 0x00;
bool HALT, RFSH, M1, IORQ, MREQ, WR, BUSAK, RD;

void setup() {

  Serial.begin(SER_SPEED);
  while (!Serial)
    ;

  a80u.begin_UNO();  // Configures Arduino Uno inputs and outputs
  a80u.begin_RAM();  // Initializes the SRAM
  a80u.begin_MCP();  // Configures inputs and outputs of both MCPs

  a80u.set_RST(LOW);  // HIGH | LOW - Sets the RST pin state
  a80u.set_CLK(LOW);  // HIGH | LOW - Sets the CLK pin state

  a80u.set_INT(HIGH);    // HIGH | LOW - Sets the INT pin state
  a80u.set_NMI(HIGH);    // HIGH | LOW - Sets the NMI pin state
  a80u.set_BUSRQ(HIGH);  // HIGH | LOW - Sets the BUSRQ pin state
  a80u.set_WAIT(HIGH);   // HIGH | LOW - Sets the WAIT pin state

  uint8_t Cmd = a80u.read_CMD();  // b0: HALT | b1: RFSH | b2: M1 | b3: IORQ | b4: MREQ | b5: WR | b6: BUSAK | b7: RD
  HALT = bitRead(Cmd, 0);
  RFSH = bitRead(Cmd, 1);
  M1 = bitRead(Cmd, 2);
  IORQ = bitRead(Cmd, 3);
  MREQ = bitRead(Cmd, 4);
  WR = bitRead(Cmd, 5);
  BUSAK = bitRead(Cmd, 6);
  RD = bitRead(Cmd, 7);

  Addr = a80u.read_ADDR();  // Reads the address bus

  Data = a80u.read_RAM(Addr);  // Reads SRAM
  a80u.write_RAM(Addr, Data);  // Writes SRAM

  a80u.pinMode_DATA(OUTPUT);  // INPUT | OUTPUT - Sets the data-bus direction
  a80u.write_DATA(Data);      // Writes the data bus

  a80u.pinMode_DATA(INPUT);  // INPUT | OUTPUT - Sets the data-bus direction
  Data = a80u.read_DATA();   // Reads the data bus
}

void loop() {
  // put your main code here, to run repeatedly:
}
